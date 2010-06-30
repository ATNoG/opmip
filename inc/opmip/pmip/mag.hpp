//=============================================================================
// Brief   : Mobile Access Gateway Service
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

#ifndef OPMIP_PMIP_MAG__HPP_
#define OPMIP_PMIP_MAG__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/mproto.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ll/mac_address.hpp>
#include <opmip/pmip/bulist.hpp>
#include <opmip/pmip/node_db.hpp>
#include <opmip/sys/ip6_tunnel.hpp>
#include <opmip/sys/route_table.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/icmp.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class mag {
	typedef boost::asio::io_service::strand strand;

public:
	typedef ip::address_v6  ip_address;
	typedef ll::mac_address mac_address;

public:
	mag(boost::asio::io_service& ios, node_db& ndb, size_t concurrency);

	void start(const char* id, const ip_address& mn_access_link);
	void stop();

	void mobile_node_attach(const mac_address& mn_mac);
	void mobile_node_detach(const mac_address& mn_mac);

private:

	void icmp_ra_timer_handler(const boost::system::error_code& ec, const std::string& mn_id);
	void icmp_ra_send_handler(const boost::system::error_code& ec);

private:
	void istart(const char* id, const ip_address& mn_access_link);
	void istop();

	void imobile_node_attach(const mac_address& mn_mac);
	void imobile_node_detach(const mac_address& mn_mac);

	void add_route_entries(bulist_entry* be);
	void del_route_entries(bulist_entry* be);

private:
	strand   _service;
	bulist   _bulist;
	node_db& _node_db;
	ip::mproto::socket            _mp_sock;
	boost::asio::ip::icmp::socket _icmp_sock;

	uint             _tunnel_dev;
	uint             _access_dev;
	sys::route_table _route_table;
	std::string      _identifier;
	size_t           _concurrency;
	sys::ip6_tunnel  _tunnel;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_MAG__HPP_ */
