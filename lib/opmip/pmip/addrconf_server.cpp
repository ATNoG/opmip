//=============================================================================
// Brief   : Address Configuration Server
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

#include <opmip/exception.hpp>
#include <opmip/pmip/addrconf_server.hpp>
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

///////////////////////////////////////////////////////////////////////////////
addrconf_server::addrconf_server(boost::asio::io_service& ios)
	: _io_service(ios), _link_sock(ios, net::link::ethernet(net::link::ethernet::ipv6))
{
}

addrconf_server::~addrconf_server()
{
	clear();
	dev_clear();
}

bool addrconf_server::dev_add(uint id, const link_address& link_addr, const ip6_address& link_local)
{
	device d(id, link_addr, link_local);

	if (!_devices.insert(d).second)
		return false;

	return true;
}

bool addrconf_server::dev_rem(uint id)
{
	devices::iterator i = _devices.find(id);

	if (i == _devices.end())
		return false;

	if (i->udp_sock)
		i->udp_sock->cancel();
	_devices.erase(i);
	return true;
}

void addrconf_server::dev_clear()
{
	for (devices::iterator i = _devices.begin(), e = _devices.end(); i != e; ++i)
		if (i->udp_sock)
			i->udp_sock->cancel();

	_devices.clear();
}

bool addrconf_server::add(const router_advertisement_info& ai)
{
	client c(ai.dst_link_address, ai.link_address);

	c.ra_timer = boost::make_shared<boost::asio::deadline_timer>(boost::ref(_io_service));
	c.prefixes = ai.prefix_list;

	if (!_clients.insert(c).second)
		return false;

	icmp_ra_sender_ptr ras(new icmp_ra_sender(ai));
	net::link::ethernet::endpoint ep(net::link::ethernet::ipv6,
	                                 ai.device_id,
	                                 net::link::ethernet::endpoint::outgoing,
	                                 ai.dst_link_address);

	router_advertisement(boost::system::error_code(), ras, ep, c.ra_timer);
	return true;
}

bool addrconf_server::del(const link_address& addr)
{
	clients::iterator i = _clients.find(addr);

	if (i == _clients.end())
		return false;

	if (i->ra_timer)
		i->ra_timer->cancel();
	_clients.erase(i);
	return true;
}

void addrconf_server::clear()
{
	for (clients::iterator i = _clients.begin(), e = _clients.end(); i != e; ++i)
		if (i->ra_timer)
			i->ra_timer->cancel();

	_clients.clear();
}

void addrconf_server::router_advertisement(const boost::system::error_code& ec,
                                           icmp_ra_sender_ptr& ras,
                                           net::link::ethernet::endpoint& ep,
                                           timer_ptr& timer)
{
	if (ec) {
		BOOST_ASSERT(ec == boost::system::errc::make_error_condition(boost::system::errc::operation_canceled));
		return;
	}

	ras->async_send(_link_sock, ep, boost::bind(ra_send_handler, _1));
	timer->expires_from_now(boost::posix_time::seconds(3)); //FIXME: set a proper timer
	timer->async_wait(boost::bind(&addrconf_server::router_advertisement, this, _1, ras, ep, timer));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
