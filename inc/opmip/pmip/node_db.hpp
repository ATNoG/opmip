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
#include <opmip/rbtree.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ip/prefix.hpp>
#include <opmip/ll/mac_address.hpp>
#include <vector>
#include <map>

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

protected:
	node(const std::string& id)
		: _id(id)
	{ }

public:
	const std::string& id() const { return _id; }

protected:
	std::string _id;
};

///////////////////////////////////////////////////////////////////////////////
class router_node : public node {
	friend class node_db;

	struct compare {
		bool operator()(const router_node& rhs, const router_node& lhs) const
		{
			return rhs._address < lhs._address;
		}

		bool operator()(const router_node& rhs, const ip_address& key) const
		{
			return rhs._address < key;
		}

		bool operator()(const ip_address& key, const router_node& lhs) const
		{
			return key < lhs._address;
		}
	};

protected:
	router_node(const std::string& id, const ip_address& addr, uint device_id)
		: node(id), _address(addr), _device_id(device_id)
	{ }

public:
	const ip_address& address() const   { return _address; }
	uint              device_id() const { return _device_id; }

protected:
	rbtree_hook _hook;
	rbtree_hook _key_hook;
	ip_address  _address;
	uint        _device_id;

	//hmac<sha1>::key _key;
};

///////////////////////////////////////////////////////////////////////////////
class mobile_node : public node {
	friend class node_db;

public:
	typedef ip::prefix_v6             ip_prefix;
	typedef ll::mac_address           link_address;
	typedef std::vector<ip_prefix>    ip_prefix_list;
	typedef std::vector<link_address> link_address_list;

public:
	mobile_node(const std::string& id, const ip_prefix_list& prefs,
	            const link_address_list& link_addrs, const std::string& lma_id,
	            const ip_address& home_addr)
		: node(id), _prefixes(prefs), _link_addrs(link_addrs), _lma_id(lma_id),
		  _home_addr(home_addr)
	{ }

	const ip_prefix_list&    prefix_list() const    { return _prefixes; }
	const link_address_list& link_addresses() const { return _link_addrs; }
	const ip_address&        home_address() const   { return _home_addr; }
	const std::string&       lma_id() const         { return _lma_id; }

private:
	rbtree_hook       _hook;
	ip_prefix_list    _prefixes;
	link_address_list _link_addrs;
	std::string       _lma_id;
	ip_address        _home_addr;
};

///////////////////////////////////////////////////////////////////////////////
class node_db {
	typedef rbtree<router_node, &router_node::_hook, node::compare> router_node_tree;
	typedef rbtree<mobile_node, &mobile_node::_hook, node::compare> mobile_node_tree;

	typedef rbtree<router_node, &router_node::_key_hook, router_node::compare>
		router_node_key_tree;

	typedef std::map<mobile_node::link_address, mobile_node*> mobile_node_key_tree;

public:
	typedef std::string                    key;
	typedef router_node::ip_address        router_key;
	typedef mobile_node::link_address      mn_key;
	typedef router_node::ip_address        ip_address;
	typedef mobile_node::ip_prefix         ip_prefix;
	typedef mobile_node::ip_prefix_list    ip_prefix_list;
	typedef mobile_node::link_address      link_address;
	typedef mobile_node::link_address_list link_address_list;
	typedef router_node_tree::iterator     router_node_iterator;
	typedef mobile_node_tree::iterator     mobile_node_iterator;

public:
	node_db();
	~node_db();

	std::pair<size_t, size_t> load(std::istream& input);

	const router_node* find_router(const key& key) const;
	const mobile_node* find_mobile_node(const key& key) const;

	const router_node* find_router(const router_key& key) const;
	const mobile_node* find_mobile_node(const mn_key& key) const;

	router_node_iterator router_node_begin() { return _router_nodes_by_id.begin(); }
	router_node_iterator router_node_end()   { return _router_nodes_by_id.end(); }

	mobile_node_iterator mobile_node_begin() { return _mobile_nodes_by_id.begin(); }
	mobile_node_iterator mobile_node_end()   { return _mobile_nodes_by_id.end(); }

protected:
	bool insert_router(const std::string& id, const ip_address& addr, uint device_id);
	bool insert_mobile_node(const std::string& id, const ip_prefix_list& prefs,
	                        const link_address_list& link_addrs, const std::string& lma_id,
	                        const ip_address& home_addr);

private:
	router_node_tree     _router_nodes_by_id;
	mobile_node_tree     _mobile_nodes_by_id;
	router_node_key_tree _router_nodes_by_key;
	mobile_node_key_tree _mobile_nodes_by_key;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_NODE_DB__HPP_ */
