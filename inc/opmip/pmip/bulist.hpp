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

#ifndef OPMIP_BULIST__HPP_
#define OPMIP_BULIST__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/bcache.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
class bulist {
	class value;

public:
};

class bulist::value {
	friend class bulist;

public:
	typedef ip6_address           net_address;
	typedef ip6_prefix            net_prefix;
	typedef std::list<net_prefix> net_prefix_list;
	typedef std::string           net_access_id;
	typedef std::vector<uint8>    link_id;
	typedef mac_address           link_address;
	typedef uint                  tunnel_id;
	typedef uint                  nic_id;

	value()
	{ }

private:
//	list_node _bulist_entry_hook;

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
} /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_BULIST__HPP_ */
