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
#include <opmip/refcount.hpp>
#include <opmip/pmip/types.hpp>
#include <boost/asio/ip/icmp.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class icmp_ra_sender : public refcount_base {
	template<class Handler>
	struct asio_handler;

public:
	typedef std::vector<ip::prefix_v6> prefix_list;

public:
	icmp_ra_sender(const router_advertisement_info& rainfo);

	template<class Handler>
	void async_send(boost::asio::ip::icmp::socket& sock, Handler handler)
	{
		sock.async_send_to(boost::asio::buffer(_buffer, _length),
			               _endpoint,
			               asio_handler<Handler>(this, handler));
	}

public:
	boost::asio::ip::icmp::endpoint _endpoint;
	uint                            _length;
	uchar                           _buffer[1460];
};

typedef refcount_ptr<icmp_ra_sender> icmp_ra_sender_ptr;

template<class Handler>
struct icmp_ra_sender::asio_handler {
	asio_handler(icmp_ra_sender* ras, Handler handler)
		: _ras(ras), _handler(handler)
	{ }

	void operator()(const boost::system::error_code& ec, size_t wbytes)
	{
		BOOST_ASSERT((ec || (!ec && wbytes == _ras->_length)));
		_handler(ec, _ras);
	}

	icmp_ra_sender_ptr _ras;
	Handler            _handler;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_ICMP_SENDER__HPP_ */
