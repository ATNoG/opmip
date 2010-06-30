//=============================================================================
// Brief   : Binding Update List
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

#ifndef OPMIP_PMIP_BULIST__HPP_
#define OPMIP_PMIP_BULIST__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ip/prefix.hpp>
#include <opmip/ll/technology.hpp>
#include <opmip/ll/mac_address.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/intrusive/rbtree.hpp>
#include <string>
#include <vector>
#include <ctime>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class bulist_entry {

	friend class bulist;

public:
	typedef ip::address_v6          net_address;
	typedef ip::prefix_v6           net_prefix;
	typedef std::vector<net_prefix> net_prefix_list;
	typedef ll::mac_address         link_address;
	typedef uint                    nic_id;

	enum bind_status_t {
		k_bind_unknown,
		k_bind_requested,
		k_bind_ack,
		k_bind_detach,
		k_bind_error,
	};

public:
	bulist_entry(boost::asio::io_service& ios, const std::string& mn_id,
	                                           const link_address& mn_link_address,
	                                           const net_prefix_list& mn_prefix_list,
	                                           const net_address& lma_address)
		: _mn_id(mn_id), _mn_link_addr(mn_link_address),
		  _mn_prefix_list(mn_prefix_list), _lma_addr(lma_address),
		  initial_lifetime(0), remaining_lifetime(0), sequence_number(std::time(nullptr)),
		  timestamp(std::time(nullptr)), bind_status(k_bind_unknown), retry_count(0),
		  timer(ios)
	{ }

	const std::string&     mn_id() const              { return _mn_id; }
	const link_address&    mn_link_address() const    { return _mn_link_addr; }
	const net_prefix_list& mn_prefix_list() const     { return _mn_prefix_list; }
	const net_address&     lma_address() const        { return _lma_addr; }

private:
	boost::intrusive::set_member_hook<> _mn_id_hook;
	boost::intrusive::set_member_hook<> _mn_link_addr_hook;

	std::string     _mn_id;               ///MN Identifier
	link_address    _mn_link_addr;        ///MN Link Address for the MN access point
	net_prefix_list _mn_prefix_list;      ///MN List of Network Prefixes
	nic_id          _mn_access_dev;       ///MAG Network Interface Identifier for the MN access point
	net_address     _lma_addr;            ///LMA Address

public:
	uint64        initial_lifetime;    ///Initial Lifetime
	uint64        remaining_lifetime;  ///Remaining Lifetime
	uint16        sequence_number;     ///Last Sequence Number
	uint64        timestamp;           ///Timestamp to limit the send rate
	bind_status_t bind_status;
	uint          retry_count;

	boost::asio::deadline_timer timer;
};

///////////////////////////////////////////////////////////////////////////////
class bulist {
	struct compare_mn_id {
		bool operator()(const bulist_entry& rhs, const bulist_entry& lhs) const
		{
			return rhs._mn_id < lhs._mn_id;
		}

		bool operator()(const bulist_entry& rhs, const std::string& key) const
		{
			return rhs._mn_id < key;
		}

		bool operator()(const std::string& key, const bulist_entry& lhs) const
		{
			return key < lhs._mn_id;
		}
	};

	struct compare_mn_link_address {
		bool operator()(const bulist_entry& rhs, const bulist_entry& lhs) const
		{
			return rhs._mn_link_addr < lhs._mn_link_addr;
		}

		bool operator()(const bulist_entry& rhs, const bulist_entry::link_address& key) const
		{
			return rhs._mn_link_addr < key;
		}

		bool operator()(const bulist_entry::link_address& key, const bulist_entry& lhs) const
		{
			return key < lhs._mn_link_addr;
		}
	};

	typedef boost::intrusive::compare<compare_mn_id>           compare_mn_id_option;
	typedef boost::intrusive::compare<compare_mn_link_address> compare_mn_link_addr_option;

	typedef boost::intrusive::member_hook<bulist_entry,
	                                      boost::intrusive::set_member_hook<>,
	                                      &bulist_entry::_mn_id_hook> mn_id_hook_option;
	typedef boost::intrusive::member_hook<bulist_entry,
	                                      boost::intrusive::set_member_hook<>,
	                                      &bulist_entry::_mn_link_addr_hook> mn_link_addr_hook_option;

	typedef boost::intrusive::rbtree<bulist_entry,
	                                 mn_id_hook_option,
	                                 compare_mn_id_option> mn_id_tree;
	typedef boost::intrusive::rbtree<bulist_entry,
	                                 mn_link_addr_hook_option,
	                                 compare_mn_link_addr_option> mn_link_addr_tree;

public:
	typedef bulist_entry                  entry_type;
	typedef bulist_entry::net_address     net_address;
	typedef bulist_entry::net_prefix      net_prefix;
	typedef bulist_entry::net_prefix_list net_prefix_list;
	typedef bulist_entry::link_address    link_address;
	typedef bulist_entry::nic_id          nic_id;

public:
	bulist();
	~bulist();

	bool insert(bulist_entry* entry);
	bool remove(bulist_entry* entry);

	bulist_entry* find(const std::string& mn_id);
	bulist_entry* find(const link_address& mn_link_address);

	void clear();

private:
	mn_id_tree        _mn_id_tree;
	mn_link_addr_tree _mn_link_addr_tree;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_BULIST__HPP_ */
