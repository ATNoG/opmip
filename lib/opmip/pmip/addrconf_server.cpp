//=============================================================================
// Brief   : Address Configuration Server
// Authors : Bruno Santos <bsantos@av.it.pt>
// Authors : Filipe Manco <filipe.manco@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#include <opmip/exception.hpp>
#include <opmip/pmip/addrconf_server.hpp>
#include <opmip/pmip/icmp_sender.hpp>
#include <opmip/ip/icmp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
static void ra_send_handler(const boost::system::error_code& ec)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		std::cerr << "ICMPv6 router advertisement send error: " << ec.message() << std::endl;
}

static void router_advertisement(const boost::system::error_code& ec,
                                 icmp_ra_sender_ptr& ras,
                                 addrconf_server::socket_ptr& sock,
                                 net::link::ethernet::endpoint& ep,
                                 addrconf_server::timer_ptr& timer)
{
	if (ec) {
		BOOST_ASSERT(ec == boost::system::errc::make_error_condition(boost::system::errc::operation_canceled));
		return;
	}

	ras->async_send(*sock, ep, boost::bind(ra_send_handler, _1));
	timer->expires_from_now(boost::posix_time::seconds(3)); //FIXME: set a proper timer
	timer->async_wait(boost::bind(router_advertisement, _1, ras, sock, ep, timer));
}

///////////////////////////////////////////////////////////////////////////////
addrconf_server::addrconf_server(boost::asio::io_service& ios)
	: _io_service(ios)
{
}

void addrconf_server::add(uint device_id, const router_advertisement_info& ai)
{
	client_map::iterator i = _clients.find(ai.dst_link_address);
	if(i != _clients.end())
		i->second.get()->cancel();

	socket_ptr sock(boost::make_shared<net::link::ethernet::socket>(boost::ref(_io_service),
	                                                                net::link::ethernet(net::link::ethernet::ipv6)));
	timer_ptr timer(boost::make_shared<boost::asio::deadline_timer>(boost::ref(_io_service)));
	icmp_ra_sender_ptr ras(new icmp_ra_sender(ai));
	net::link::ethernet::endpoint ep(net::link::ethernet::ipv6,
	                                 device_id,
	                                 net::link::ethernet::endpoint::outgoing,
	                                 ai.dst_link_address);

	router_advertisement(boost::system::error_code(), ras, sock, ep, timer);
	_clients[ai.dst_link_address] = timer;
}

void addrconf_server::del(const link_address& addr)
{
	client_map::iterator i = _clients.find(addr);

	if (i == _clients.end())
		return;

	i->second->cancel();
	_clients.erase(i);
}

void addrconf_server::clear()
{
	for (client_map::iterator i = _clients.begin(), e = _clients.end(); i != e; ++i)
		i->second->cancel();

	_clients.clear();
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
