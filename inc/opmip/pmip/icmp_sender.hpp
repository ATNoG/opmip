//=============================================================================
// Brief   : ICMPv6 Sender
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

#ifndef OPMIP_PMIP_ICMP_SENDER__HPP_
#define OPMIP_PMIP_ICMP_SENDER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/pmip/types.hpp>
#include <opmip/net/ip/ipv6_packet.hpp>
#include <opmip/net/ip/icmp6_ra_packet.hpp>
#include <opmip/net/link/ethernet.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class icmp_ra_sender : public boost::enable_shared_from_this<icmp_ra_sender> {
	template<class Handler>
	struct asio_handler;

public:
	typedef std::vector<ip::prefix_v6> prefix_list;

public:
	icmp_ra_sender(const router_advertisement_info& rainfo);

	template<class Handler>
	void async_send(boost::asio::ip::icmp::socket& sock, Handler handler)
	{
		sock.async_send_to(_ra_pkt.cbuffer(),
			               _endpoint,
			               asio_handler<Handler>(this, handler));
	}

	template<class Handler>
	void async_send(net::link::ethernet::socket& sock, net::link::ethernet::endpoint& ep, Handler handler)
	{
		sock.async_send_to(_ipv6_pkt.cbuffer(),
			               ep,
			               asio_handler<Handler>(this, handler));
	}

public:
	boost::asio::ip::icmp::endpoint _endpoint;
	net::ip::ipv6_packet            _ipv6_pkt;
	net::ip::icmp6_ra_packet        _ra_pkt;
};

typedef boost::shared_ptr<icmp_ra_sender> icmp_ra_sender_ptr;

template<class Handler>
struct icmp_ra_sender::asio_handler {
	asio_handler(icmp_ra_sender* ras, Handler handler)
		: _ras(ras->shared_from_this()), _handler(handler)
	{ }

	void operator()(const boost::system::error_code& ec, size_t /*wbytes*/)
	{
		_handler(ec, _ras);
	}

	icmp_ra_sender_ptr _ras;
	Handler            _handler;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_ICMP_SENDER__HPP_ */
