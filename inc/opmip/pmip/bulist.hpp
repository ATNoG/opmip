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
#include <boost/intrusive/rbtree.hpp>
#include <string>
#include <vector>
#include <list>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class bulist_entry {

	friend class bulist;

public:
	typedef ip::address_v6        net_address;
	typedef ip::prefix_v6         net_prefix;
	typedef std::list<net_prefix> net_prefix_list;
	typedef std::string           net_access_id;
	typedef std::vector<uint8>    link_id;
	typedef ll::mac_address       link_address;
	typedef uint                  tunnel_id;
	typedef uint                  nic_id;

public:
	bulist_entry()
	{ }

private:
	boost::intrusive::set_member_hook<> _hook;

protected:
	net_address     _mn_addr;             ///MN Address
	net_access_id   _mn_id;               ///MN Identifier
	link_id         _mn_link_id;          ///MN Link Identifier
	net_address     _home_addr;           ///Home Address
	net_address     _care_of_addr;        ///Care of Address
	net_prefix_list _mn_prefix_list;      ///MN List of Network Prefixes
	link_address    _mag_link_addr_to_mn; ///MAG Link Address for the MN access point
	nic_id          _mag_nic_id_to_mn;    ///MAG Network Interface Identifier for the MN access point
	net_address     _lma_addr;            ///LMA Address
	tunnel_id       _tunnel;              ///Tunnel Identifier
	uint64          _initial_lifetime;    ///Initial Lifetime
	uint64          _remaining_lifetime;  ///Remaining Lifetime
	uint16          _sequence_number;     ///Last Sequence Number
	uint64          _timestamp;           ///Timestamp to limit the send rate
	struct {
		bool      	pending;
		uchar     	count;
		uint64    	timeout;
	}             _retry_state;           ///Retransmission state
	bool          _is_send_enable;        ///Flag indicating if future binding updates should be sent
};

///////////////////////////////////////////////////////////////////////////////
class bulist {
	struct compare {
		bool operator()(const bulist_entry& rhs, const bulist_entry& lhs) const
		{
			return rhs._mn_id < lhs._mn_id;
		}

		bool operator()(const bulist_entry& rhs, const bulist_entry::net_access_id& key) const
		{
			return rhs._mn_id < key;
		}

		bool operator()(const bulist_entry::net_access_id& key, const bulist_entry& lhs) const
		{
			return key < lhs._mn_id;
		}
	};

	typedef boost::intrusive::compare<compare> compare_option;

	typedef boost::intrusive::member_hook<bulist_entry,
	                                      boost::intrusive::set_member_hook<>,
	                                      &bulist_entry::_hook> member_hook_option;

public:
	typedef boost::intrusive::rbtree<bulist_entry,
	                                 member_hook_option,
	                                 compare_option> type;

	typedef bulist_entry                  entry_type;
	typedef bulist_entry::net_address     net_address;
	typedef bulist_entry::net_prefix      net_prefix;
	typedef bulist_entry::net_prefix_list net_prefix_list;
	typedef bulist_entry::net_access_id   net_access_id;
	typedef bulist_entry::link_id         link_id;
	typedef bulist_entry::link_address    link_address;
	typedef bulist_entry::tunnel_id       tunnel_id;
	typedef bulist_entry::nic_id          nic_id;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_BULIST__HPP_ */
