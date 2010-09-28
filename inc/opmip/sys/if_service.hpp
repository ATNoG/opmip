//==============================================================================
// Brief   : Network Interface Service
// Authors : Bruno Santos <bsantos@av.it.pt>
// -----------------------------------------------------------------------------
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
//==============================================================================

#ifndef OPMIP_SYS_IF_SERVICE__HPP_
#define OPMIP_SYS_IF_SERVICE__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/impl/if_service.hpp>
#include <opmip/sys/error.hpp>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

////////////////////////////////////////////////////////////////////////////////
class if_service : boost::noncopyable {
public:
	typedef impl::if_service::address_mac        address_mac;
	typedef impl::if_service::event_type         event_type;
	typedef impl::if_service::wireless_event     wireless_event;
	typedef impl::if_service::wireless_frequency wireless_frequency;
	typedef impl::if_service::event              event;
	typedef impl::if_service::event_handler      event_handler;

public:
	if_service(boost::asio::io_service& ios)
		: implementation(ios)
	{ }

	template<class EventHandler>
	void start(EventHandler handler)
	{
		boost::system::error_code ec;

		implementation.set_event_handler(handler);
		implementation.start(ec);
		throw_on_error(ec);
	}

	void stop()
	{
		boost::system::error_code ec;

		implementation.stop(ec);
		throw_on_error(ec);
	}

private:
	impl::if_service implementation;
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_IF_SERVICE__HPP_ */
