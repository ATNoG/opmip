//=============================================================================
// Brief   : Mobility Protocol Sender
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

#ifndef OPMIP_PMIP_MP_SENDER__HPP_
#define OPMIP_PMIP_MP_SENDER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/mproto.hpp>
#include <opmip/pmip/types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class pbu_sender : public boost::enable_shared_from_this<pbu_sender> {
	template<class Handler>
	struct asio_handler;

public:
	pbu_sender(const proxy_binding_info& pbinfo);

	template<class Handler>
	void async_send(ip::mproto::socket& sock, Handler handler)
	{
		sock.async_send_to(boost::asio::buffer(_buffer, _length),
			               _endpoint,
			               asio_handler<Handler>(this, handler));
	}

public:
	ip::mproto::endpoint _endpoint;
	uint                 _length;
	uchar                _buffer[1460];
};

typedef boost::shared_ptr<pbu_sender> pbu_sender_ptr;

template<class Handler>
struct pbu_sender::asio_handler {
	asio_handler(pbu_sender* pbus, Handler handler)
		: _pbus(pbus), _handler(handler)
	{ }

	void operator()(const boost::system::error_code& ec, size_t wbytes)
	{
		BOOST_ASSERT((ec || (!ec && wbytes == _pbus->_length)));
		_handler(ec, _pbus);
	}

	pbu_sender_ptr _pbus;
	Handler        _handler;
};

///////////////////////////////////////////////////////////////////////////////
class pba_sender : public boost::enable_shared_from_this<pba_sender> {
	template<class Handler>
	struct asio_handler;

public:
	pba_sender(const proxy_binding_info& pbinfo);

	template<class Handler>
	void async_send(ip::mproto::socket& sock, Handler handler)
	{
		sock.async_send_to(boost::asio::buffer(_buffer, _length),
			               _endpoint,
			               asio_handler<Handler>(this, handler));
	}

private:
	ip::mproto::endpoint _endpoint;
	uint                 _length;
	uchar                _buffer[1460];
};

typedef boost::shared_ptr<pba_sender> pba_sender_ptr;

template<class Handler>
struct pba_sender::asio_handler {
	asio_handler(pba_sender* pbas, Handler handler)
		: _pbas(pbas), _handler(handler)
	{ }

	void operator()(const boost::system::error_code& ec, size_t wbytes)
	{
		BOOST_ASSERT((ec || (!ec && wbytes == _pbas->_length)));
		_handler(ec, _pbas);
	}

	pba_sender_ptr _pbas;
	Handler        _handler;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_MP_SENDER__HPP_ */
