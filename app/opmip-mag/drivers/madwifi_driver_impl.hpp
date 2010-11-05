//==============================================================================
// Brief   : MadWifi Driver Implementation
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

#ifndef OPMIP_APP_DRIVER_MADWIFI_DRIVER_IMPL__HPP_
#define OPMIP_APP_DRIVER_MADWIFI_DRIVER_IMPL__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ll/mac_address.hpp>
#include <opmip/sys/netlink.hpp>
#include <boost/system/error_code.hpp>
#include <boost/function.hpp>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
class madwifi_driver_impl : boost::noncopyable {
public:
	typedef ll::mac_address address_mac;

	enum event_type {
		e_unknown,
		e_new,
		e_del,
		e_get,
		e_set,
	};

	enum wireless_event {
		wevent_none,
		wevent_attach,
		wevent_detach,
	};

	struct event {
		event()
			: which(e_unknown), if_index(0), if_type(0), if_flags(0),
			  if_change(0), if_state(0), if_mtu(0)
		{
			if_wireless.which = wevent_none;
		}

		event_type  which;
		uint        if_index;
		uint        if_type;
		uint        if_flags;
		uint        if_change;
		uint        if_state;
		std::string if_name;
		address_mac if_address;
		uint        if_mtu;

		struct {
			wireless_event which;
			address_mac    address;
		} if_wireless;
	};

	typedef boost::function<void(const boost::system::error_code&,
	                             const event&)> event_handler;

public:
	madwifi_driver_impl(boost::asio::io_service& ios);
	~madwifi_driver_impl();

	void start(boost::system::error_code& ec);
	void stop(boost::system::error_code& ec);

	template<class EventHandler>
	void set_event_handler(EventHandler handler)
	{
		_event_handler = handler;
	}

private:
	void receive_handler(boost::system::error_code ec, size_t rbytes);

private:
	sys::netlink<0>::socket _rtnl;
	event_handler           _event_handler;
	uchar                   _buffer[4096];
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_DRIVER_MADWIFI_DRIVER_IMPL__HPP_ */
