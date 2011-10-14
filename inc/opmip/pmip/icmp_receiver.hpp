//=============================================================================
// Brief   : ICMP Receiver
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

#ifndef OPMIP_PMIP_ICMP_RECEIVER__HPP_
#define OPMIP_PMIP_ICMP_RECEIVER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ll/mac_address.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/icmp.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class icmp_rs_receiver : public boost::enable_shared_from_this<icmp_rs_receiver> {
	template<class Handler>
	struct asio_handler;

public:
	icmp_rs_receiver()
	{ }

	template<class Handler>
	void async_receive(boost::asio::ip::icmp::socket& sock, Handler handler)
	{
		sock.async_receive_from(boost::asio::buffer(_buffer),
			                    _endpoint,
			                    asio_handler<Handler>(this, handler));
	}

private:
	bool parse(size_t rbytes, ll::mac_address& source_link_layer);

public:
	boost::asio::ip::icmp::endpoint _endpoint;
	uchar                           _buffer[1460];
};

typedef boost::shared_ptr<icmp_rs_receiver> icmp_rs_receiver_ptr;

template<class Handler>
struct icmp_rs_receiver::asio_handler {
	asio_handler(icmp_rs_receiver* rsr, Handler handler)
		: _rsr(rsr->shared_from_this()), _handler(handler)
	{ }

	void operator()(boost::system::error_code ec, size_t rbytes)
	{
		ll::mac_address sll;
		ip::address_v6  addr;

		if (!ec) {
			if (!_rsr->parse(rbytes, sll))
				ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);
			else
				addr = _rsr->_endpoint.address().to_v6();
		}

		_handler(ec, addr, sll, _rsr);
	}

	icmp_rs_receiver_ptr _rsr;
	Handler              _handler;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_ICMP_RECEIVER__HPP_ */
