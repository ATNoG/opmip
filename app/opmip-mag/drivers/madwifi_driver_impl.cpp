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

#include "madwifi_driver_impl.hpp"
#include <opmip/sys/netlink/message.hpp>
#include <opmip/sys/netlink/utils.hpp>
#include <opmip/sys/rtnetlink/link.hpp>
#include <opmip/sys/error.hpp>
#include <boost/bind.hpp>
#include <ctime>
#include <iostream>
#include "wireless.h"

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
enum rt_subscriptions {
	rt_link = 1
};

////////////////////////////////////////////////////////////////////////////////
madwifi_driver_impl::madwifi_driver_impl(boost::asio::io_service& ios)
	: _rtnl(ios)
{
}

madwifi_driver_impl::~madwifi_driver_impl()
{
}

void madwifi_driver_impl::start(boost::system::error_code& ec)
{
	_rtnl.open(sys::netlink<0>(), ec);
	if (ec)
		return;

	_rtnl.bind(sys::netlink<0>::endpoint(rt_link), ec);
	if (ec)
		return;

	_rtnl.async_receive(boost::asio::buffer(_buffer),
	                    boost::bind(&madwifi_driver_impl::receive_handler, this, _1, _2));
}

void madwifi_driver_impl::stop(boost::system::error_code& ec)
{
	_rtnl.close(ec);
}

void madwifi_driver_impl::receive_handler(boost::system::error_code ec, size_t rbytes)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_event_handler.clear();
		else
			_event_handler(ec, event());
		return;
	}

	sys::nl::message_iterator mit(_buffer, rbytes);
	sys::nl::message_iterator end;
	int                  errc = 0;

	for (; mit != end; ++mit) {
		if (mit->type == sys::nl::header::m_error) {
			sys::nl::message<sys::nl::error> err(mit);

			errc = -err->error;
			if (errc)
				ec = boost::system::error_code(errc, boost::system::get_system_category());

			_event_handler(ec, event());

		} else {
			sys::nl::message<sys::rtnl::link> lnk(mit);
			sys::nl::message<sys::rtnl::link>::attr_iterator ai = lnk.abegin();
			sys::nl::message<sys::rtnl::link>::attr_iterator aend;
			event inf;

			inf.which = static_cast<event_type>(mit->type - sys::rtnl::link::m_new + e_new);
			inf.if_index = lnk->index;
			inf.if_type = lnk->type;
			inf.if_flags = lnk->flags;
			inf.if_change = lnk->change;

			for (; ai != aend; ++ai) {
				switch (ai->type) {
				case sys::rtnl::link::attr_address:
					inf.if_address = address_mac(*ai.get<address_mac::bytes_type>());
				break;

				case sys::rtnl::link::attr_ifname:
					inf.if_name.assign(ai.get<const char>(), ai.length());
				break;

				case sys::rtnl::link::attr_mtu:
					inf.if_mtu = *ai.get<uint32>();
				break;

				case sys::rtnl::link::attr_wireless: {
					::iw_event* we = ai.get< ::iw_event>();

					switch (we->cmd) {
					case IWEVREGISTERED:
						inf.if_wireless.which = wevent_attach;
						inf.if_wireless.address = address_mac(*reinterpret_cast<opmip::ll::mac_address::bytes_type*>(we->u.ap_addr.sa_data));
						break;

					case IWEVEXPIRED:
						inf.if_wireless.which = wevent_detach;
						inf.if_wireless.address = address_mac(*reinterpret_cast<opmip::ll::mac_address::bytes_type*>(we->u.ap_addr.sa_data));
						break;

					default:
//						std::cout << "unknown wireless event: "
//						             "cmd = " << std::hex << uint(we->cmd) << std::dec << ", "
//						             "length = " << uint(we->len) << std::endl;
						break;
					}
				} break;

				case sys::rtnl::link::attr_operstate:
					inf.if_state = *ai.get<uint32>();
					break;

				default:
//					std::cout << "unknown attribute: type = "   << uint(ai->type)
//					          <<                  ", length = " << uint(ai->length) << std::endl;
					break;
				}
			}
			_event_handler(ec, inf);
		}
	}

	_rtnl.async_receive(boost::asio::buffer(_buffer),
	                    boost::bind(&madwifi_driver_impl::receive_handler, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
