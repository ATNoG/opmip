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

#include <opmip/logger.hpp>
#include <opmip/exception.hpp>
#include <opmip/pmip/addrconf_server.hpp>
#include <opmip/net/ip/dhcp_v6.hpp>
#include <opmip/ip/icmp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/shared_array.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <cmath>

namespace dhcp6 = opmip::net::ip::dhcp_v6;

static const uint k_lifetime_fixme = 3600; //1 hour (sec)

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
static logger log_("addrconf", std::cout);

///////////////////////////////////////////////////////////////////////////////
static void ra_send_handler(const boost::system::error_code& ec)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		log_(0, "ICMPv6 router advertisement send error: ", ec.message());
}

static void dhcp6_send_handler(const boost::system::error_code& ec, boost::shared_array<uint8>&)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		log_(0, "DHCPv6 send error: ", ec.message());
	log_(0, "DHCPv6 message sent");
}

///////////////////////////////////////////////////////////////////////////////
struct addrconf_server::dhcp_receive_data {
	uchar                          buffer[1024];
	boost::asio::ip::udp::endpoint source;
};

addrconf_server::addrconf_server(boost::asio::io_service& ios)
	: _link_sock(ios, net::link::ethernet(net::link::ethernet::ipv6)),
	  _udp_sock(ios, boost::asio::ip::udp::endpoint(ip6_address::any(), dhcp6::server_port))
{
}

addrconf_server::~addrconf_server()
{
	clear();
}

void addrconf_server::start()
{
	dhcp_receive_data_ptr rd(boost::make_shared<dhcp_receive_data>());
	ip6_address ma(dhcp6::all_servers_and_relay_agents);

	_udp_sock.set_option(boost::asio::ip::multicast::join_group(ma));
	_udp_sock.async_receive_from(boost::asio::buffer(rd->buffer), rd->source,
	                             boost::bind(&addrconf_server::dhcp6_receive_handler,
	                                         this, _1, _2, rd));
}

void addrconf_server::stop()
{
	_udp_sock.cancel();
}

bool addrconf_server::add(const router_advertisement_info& ai)
{
	client c(ai.dst_link_address, ai.link_address);

	c.ra_timer = boost::make_shared<boost::asio::deadline_timer>(boost::ref(_link_sock.get_io_service()));
	c.prefixes = ai.prefix_list;
	c.home_addr = ai.home_addr;

	if (!_clients.insert(c).second)
		return false;

	if (!ai.prefix_list.empty()) {
		icmp_ra_sender_ptr ras(new icmp_ra_sender(ai));

		net::link::ethernet::endpoint ep(net::link::ethernet::ipv6,
		                                 ai.device_id,
		                                 net::link::ethernet::endpoint::outgoing,
		                                 ai.dst_link_address);
		router_advertisement(boost::system::error_code(), ras, ep, c.ra_timer);
	}
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

void addrconf_server::dhcp6_receive_handler(const boost::system::error_code& ec,
                                            size_t blen,
	                                        dhcp_receive_data_ptr& data)
{
	if (ec) {
		BOOST_ASSERT(ec == boost::system::errc::make_error_condition(boost::system::errc::operation_canceled));
		log_(0, "dhcp6 receive error: ", ec.message());
		return;
	}

	dhcp_receive_data_ptr rd(boost::make_shared<dhcp_receive_data>());

	_udp_sock.async_receive_from(boost::asio::buffer(rd->buffer), rd->source,
	                             boost::bind(&addrconf_server::dhcp6_receive_handler,
	                                         this, _1, _2, rd));

	dhcp6_handle_message(dhcp6::buffer_type(data->buffer, data->buffer + blen), data->source);
}

void addrconf_server::dhcp6_handle_message(dhcp6::buffer_type buff, const boost::asio::ip::udp::endpoint& ep)
{
	dhcp6::buffer_type tmp;
	dhcp6::buffer_type cid;
	dhcp6::opcode op;
	dhcp6::option opt;
	link_address clink_addr;
	link_address poa_addr;
	ip6_address home_addr;
	uint tid;
	uint32 ia_id;
	uint32 t1;
	uint32 t2;

	log_(0, "dhcp6 message received");

	if (!parse_header(buff, op, tid))
		return;

	log_(0, "dhcp6 message: opcode = ", op, " tid = ", tid);

	while (dhcp6::buffer_size(buff)) {
		if (!parse_option(buff, opt, tmp))
			return;

		log_(0, "dhcp6 message option: option = ", opt);

		switch (opt) {
		case dhcp6::client_id:
			cid = tmp;
			if (!dhcp6::parse_option_duid(tmp, clink_addr))
				return;
			log_(0, "dhcp6 message option client_id: ", clink_addr);
			break;

		case dhcp6::ia_na:
			if (!dhcp6::parse_option_ia(tmp, ia_id, t1, t2))
				return;
			log_(0, "dhcp6 message option ia_na: id = ", ia_id, " t1 = ", t1, " t2 = ", t2);
			break;

		default:
			break;
		}
	}
	if (!clink_addr)
		return;

	log_(0, "dhcp6 message parsed with success");

	clients::iterator i = _clients.find(clink_addr);
	if (i != _clients.end()) {
		home_addr = i->home_addr;
		poa_addr = i->poa_link_addr;
	}

	log_(0, "dhcp6 client: <", home_addr, ", ", poa_addr, ">");

	bool hdonly = false;

	switch (op) {
	case dhcp6::solicitation:
		op = dhcp6::advertise;
		break;

	case dhcp6::info_request:
		hdonly = true;
	case dhcp6::request:
		op = dhcp6::reply;
		break;

	default:
		return;
	}

	dhcp6_reply_message(ep, op, tid, poa_addr, cid, ia_id, home_addr, hdonly);
}

void addrconf_server::dhcp6_reply_message(const boost::asio::ip::udp::endpoint& ep,
                                          dhcp6::opcode op,
                                          uint tid,
                                          const link_address& poa_addr,
                                          dhcp6::buffer_type& cid,
                                          uint ia_id,
                                          const ip6_address& home_addr,
                                          bool hdonly)
{
	boost::shared_array<uint8> buffer(new uint8[1024]);
	dhcp6::buffer_type buff(buffer.get(), buffer.get() + 1024);

	log_(0, "dhcp6 reply: opcode = ", op);

	if (dhcp6::gen_message(buff, op, tid, poa_addr, cid)) {
		if (home_addr.is_unspecified()) {
			if (!dhcp6::gen_option_status(buff, dhcp6::status_no_binding, "client record not available"))
				return;

		} else if (!hdonly) {
			dhcp6::buffer_type state;

			if (!dhcp6::gen_option_ia(buff, ia_id, k_lifetime_fixme * .5, k_lifetime_fixme * .8, &state))
				return;

			if (!dhcp6::gen_option_addr(buff, home_addr, k_lifetime_fixme, k_lifetime_fixme, state))
				return;
		}

		_udp_sock.async_send_to(boost::asio::buffer(buffer.get(), buff.first - buffer.get()),
		                        ep, boost::bind(dhcp6_send_handler, _1, buffer));
	}
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
