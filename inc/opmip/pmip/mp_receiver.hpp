//=============================================================================
// Brief   : Mobility Protocol Receiver
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

#ifndef OPMIP_PMIP_MP_RECEIVER__HPP_
#define OPMIP_PMIP_MP_RECEIVER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/chrono.hpp>
#include <opmip/refcount.hpp>
#include <opmip/ip/mproto.hpp>
#include <opmip/pmip/types.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class pbu_receiver : public refcount_base {
	template<class Handler>
	struct asio_handler;

public:
	pbu_receiver()
	{ }

	template<class Handler>
	void async_receive(ip::mproto::socket& sock, Handler handler)
	{
		sock.async_receive_from(boost::asio::buffer(_buffer),
			                    _endpoint,
			                    asio_handler<Handler>(this, handler));
	}

private:
	bool parse(size_t rbytes, proxy_binding_info& pbinfo);

private:
	ip::mproto::endpoint _endpoint;
	uchar                _buffer[1460];
};

typedef refcount_ptr<pbu_receiver> pbu_receiver_ptr;

template<class Handler>
struct pbu_receiver::asio_handler {
	asio_handler(pbu_receiver* pbur, Handler handler)
		: _pbur(pbur), _handler(handler)
	{ }

	void operator()(boost::system::error_code ec, size_t rbytes)
	{
		proxy_binding_info pbinfo;
		chrono delay;

		delay.start();
		if (!ec) {
			if (!_pbur->parse(rbytes, pbinfo))
				ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);
		}

		_handler(ec, pbinfo, _pbur, delay);
	}

	pbu_receiver_ptr _pbur;
	Handler          _handler;
};

///////////////////////////////////////////////////////////////////////////////
class pba_receiver : public refcount_base {
	template<class Handler>
	struct asio_handler;

public:
	pba_receiver()
	{ }

	template<class Handler>
	void async_receive(ip::mproto::socket& sock, Handler handler)
	{
		sock.async_receive_from(boost::asio::buffer(_buffer),
			                    _endpoint,
			                    asio_handler<Handler>(this, handler));
	}

private:
	bool parse(size_t rbytes, proxy_binding_info& pbinfo);

private:
	ip::mproto::endpoint _endpoint;
	uchar                _buffer[1460];
};

typedef refcount_ptr<pba_receiver> pba_receiver_ptr;

template<class Handler>
struct pba_receiver::asio_handler {
	asio_handler(pba_receiver* pbar, Handler handler)
		: _pbar(pbar), _handler(handler)
	{ }

	void operator()(boost::system::error_code ec, size_t rbytes)
	{
		proxy_binding_info pbinfo;
		chrono delay;

		delay.start();
		if (!ec) {
			if (!_pbar->parse(rbytes, pbinfo))
				ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);
		}

		_handler(ec, pbinfo, _pbar, delay);
	}

	pba_receiver_ptr _pbar;
	Handler          _handler;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_MP_RECEIVER__HPP_ */
