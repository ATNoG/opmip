//=============================================================================
// Brief   : Node Database
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010 Universidade de Aveiro
// Copyrigth (C) 2010 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_PMIP_NODE_DB__HPP_
#define OPMIP_PMIP_NODE_DB__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ip/prefix.hpp>
#include <opmip/ll/mac_address.hpp>
#include <boost/intrusive/rbtree.hpp>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class node {
	friend class node_db;

	struct compare {
		bool operator()(const node& rhs, const node& lhs) const
		{
			return rhs._id < lhs._id;
		}

		bool operator()(const node& rhs, const std::string& key) const
		{
			return rhs._id < key;
		}

		bool operator()(const std::string& key, const node& lhs) const
		{
			return key < lhs._id;
		}
	};

public:
	typedef ip::address_v6 ip_address;

	enum etype {
		lma,
		mag,
		mobile
	};

protected:
	node(etype tp, const std::string& id)
		: _type(tp), _id(id)
	{ }

public:
	etype              type() const { return _type; }
	const std::string& id() const   { return _id; }

private:
	boost::intrusive::set_member_hook<> _hook;

protected:
	const etype _type;
	std::string _id;
};

///////////////////////////////////////////////////////////////////////////////
class route_node : public node {
	friend class node_db;

	struct compare {
		bool operator()(const route_node& rhs, const route_node& lhs) const
		{
			return rhs._address < lhs._address;
		}

		bool operator()(const route_node& rhs, const ip_address& key) const
		{
			return rhs._address < key;
		}

		bool operator()(const ip_address& key, const route_node& lhs) const
		{
			return key < lhs._address;
		}
	};

private:
	boost::intrusive::set_member_hook<> _hook;

protected:
	route_node(etype tp, const std::string& id, const ip_address& addr, uint device_id)
		: node(tp, id), _address(addr), _device_id(device_id)
	{ }

public:
	const ip_address& address() const   { return _address; }
	uint              device_id() const { return _device_id; }

protected:
	ip_address _address;
	uint       _device_id;

	//hmac<sha1>::key _key;
};

///////////////////////////////////////////////////////////////////////////////
class mag_node : public route_node {
public:
	mag_node(const std::string& id, const ip_address& addr, uint device_id)
		: route_node(node::mag, id, addr, device_id)
	{ }
};

///////////////////////////////////////////////////////////////////////////////
class lma_node : public route_node {
public:
	lma_node(const std::string& id, const ip_address& addr, uint device_id)
		: route_node(node::lma, id, addr, device_id)
	{ }
};

///////////////////////////////////////////////////////////////////////////////
class mobile_node : public node {
	friend class node_db;

public:
	typedef ip::prefix_v6          ip_prefix;
	typedef std::vector<ip_prefix> ip_prefix_list;
	typedef ll::mac_address        link_address;

private:
	struct compare {
		bool operator()(const mobile_node& rhs, const mobile_node& lhs) const
		{
			return rhs._link_addr < lhs._link_addr;
		}

		bool operator()(const mobile_node& rhs, const link_address& key) const
		{
			return rhs._link_addr < key;
		}

		bool operator()(const link_address& key, const mobile_node& lhs) const
		{
			return key < lhs._link_addr;
		}
	};

public:
	mobile_node(const std::string& id, const ip_prefix_list& prefs, const link_address& link_addr, const std::string& lma_id)
		: node(node::mobile, id), _prefixes(prefs), _link_addr(link_addr), _lma_id(lma_id)
	{ }

	const ip_prefix_list& prefix_list() const { return _prefixes; }
	const link_address&   mac_address() const { return _link_addr; }
	const std::string&    lma_id() const      { return _lma_id; }

private:
	boost::intrusive::set_member_hook<> _hook;

private:
	ip_prefix_list _prefixes;
	link_address   _link_addr;
	std::string    _lma_id;
};

///////////////////////////////////////////////////////////////////////////////
class node_db {
	typedef boost::intrusive::compare<node::compare> node_compare_option;

	typedef boost::intrusive::member_hook<node,
	                                      boost::intrusive::set_member_hook<>,
	                                      &node::_hook> node_member_hook_option;

	typedef boost::intrusive::rbtree<node,
	                                 node_member_hook_option,
	                                 node_compare_option> node_tree;


	typedef boost::intrusive::compare<route_node::compare> route_node_compare_option;

	typedef boost::intrusive::member_hook<route_node,
	                                      boost::intrusive::set_member_hook<>,
	                                      &route_node::_hook> route_node_member_hook_option;

	typedef boost::intrusive::rbtree<route_node,
	                                 route_node_member_hook_option,
	                                 route_node_compare_option> route_node_tree;


	typedef boost::intrusive::compare<mobile_node::compare> mobile_node_compare_option;

	typedef boost::intrusive::member_hook<mobile_node,
	                                      boost::intrusive::set_member_hook<>,
	                                      &mobile_node::_hook> mobile_node_member_hook_option;

	typedef boost::intrusive::rbtree<mobile_node,
	                                 mobile_node_member_hook_option,
	                                 mobile_node_compare_option> mobile_node_tree;

public:
	typedef std::string                 key;
	typedef lma_node::ip_address        lma_key;
	typedef mag_node::ip_address        mag_key;
	typedef mobile_node::link_address   mn_key;
	typedef route_node::ip_address      ip_address;
	typedef mobile_node::ip_prefix      ip_prefix;
	typedef mobile_node::ip_prefix_list ip_prefix_list;
	typedef mobile_node::link_address   link_address;

public:
	node_db();
	~node_db();

	size_t load(std::istream& input);

	const lma_node*    find_lma(const key& key) const;
	const mag_node*    find_mag(const key& key) const;
	const mobile_node* find_mobile_node(const key& key) const;

	const lma_node*    find_lma(const lma_key& key) const;
	const mag_node*    find_mag(const mag_key& key) const;
	const mobile_node* find_mobile_node(const mn_key& key) const;

protected:
	const node* find(const key& key) const;

	bool insert_lma(const std::string& id, const ip_address& addr, uint device_id);
	bool insert_mag(const std::string& id, const ip_address& addr, uint device_id);
	bool insert_mobile_node(const std::string& id, const ip_prefix_list& prefs, const link_address& link_addr, const std::string& lma_id);

private:
	node_tree        _nodes;
	route_node_tree  _lmas;
	route_node_tree  _mags;
	mobile_node_tree _mobile_nodes;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_NODE_DB__HPP_ */
