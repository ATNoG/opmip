//==============================================================================
// Brief   : Network Interface Service Implementation
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

#include <opmip/sys/if_service.hpp>
#include <opmip/sys/netlink/message.hpp>
#include <opmip/sys/netlink/utils.hpp>
#include <opmip/sys/rtnetlink/link.hpp>
#include <opmip/sys/error.hpp>
#include <boost/bind.hpp>
#include <ctime>
#include <iostream>
#include "wireless.h"

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace impl {

////////////////////////////////////////////////////////////////////////////////
enum rt_subscriptions {
	rt_link = 1
};

////////////////////////////////////////////////////////////////////////////////
if_service::if_service(boost::asio::io_service& ios)
	: _rtnl(ios)
{
}

if_service::~if_service()
{
}

void if_service::start(boost::system::error_code& ec)
{
	_rtnl.open(netlink<0>(), ec);
	if (ec)
		return;

	_rtnl.bind(netlink<0>::endpoint(rt_link), ec);
	if (ec)
		return;

	_rtnl.async_receive(boost::asio::buffer(_buffer),
	                    boost::bind(&if_service::receive_handler, this, _1, _2));
}

void if_service::stop(boost::system::error_code& ec)
{
	_rtnl.close(ec);
}

void if_service::receive_handler(boost::system::error_code ec, size_t rbytes)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_event_handler.clear();
		else
			_event_handler(ec, event());
		return;
	}

	nl::message_iterator mit(_buffer, rbytes);
	nl::message_iterator end;
	int                  errc = 0;

	for (; mit != end; ++mit) {
		if (mit->type == nl::header::m_error) {
			nl::message<nl::error> err(mit);

			errc = -err->error;
			if (errc)
				ec = boost::system::error_code(errc, boost::system::get_system_category());

			_event_handler(ec, event());

		} else {
			nl::message<rtnl::link> lnk(mit);
			nl::message<rtnl::link>::attr_iterator ai = lnk.abegin();
			nl::message<rtnl::link>::attr_iterator aend;
			event inf;

			inf.which = static_cast<event_type>(mit->type - rtnl::link::m_new + e_new);
			inf.if_index = lnk->index;
			inf.if_type = lnk->type;
			inf.if_flags = lnk->flags;
			inf.if_change = lnk->change;

			for (; ai != aend; ++ai) {
				switch (ai->type) {
				case rtnl::link::attr_address:
					inf.if_address = address_mac(*ai.get<address_mac::bytes_type>());
				break;

				case rtnl::link::attr_ifname:
					inf.if_name.assign(ai.get<const char>(), ai.length());
				break;

				case rtnl::link::attr_mtu:
					inf.if_mtu = *ai.get<uint32>();
				break;

				case rtnl::link::attr_wireless: {
					::iw_event* we = ai.get< ::iw_event>();

					switch (we->cmd) {
/*					case SIOCSIWFREQ:
						inf.if_wireless.which = wevent_frequency;
						inf.if_wireless.frequency.mantissa = we->u.freq.m;
						inf.if_wireless.frequency.exponent = we->u.freq.e;
						inf.if_wireless.frequency.index = we->u.freq.i;
						inf.if_wireless.frequency.fixed = (we->u.freq.flags & IW_FREQ_FIXED);
						break;
*/
					case SIOCGIWSCAN:
						inf.if_wireless.which = wevent_new_scan_results;
						break;

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

				case rtnl::link::attr_operstate:
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
	                    boost::bind(&if_service::receive_handler, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
} /* namespace impl */ } /* namespace sys */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
