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
#include <opmip/sys/netlink/error.hpp>
#include <opmip/sys/rtnetlink/link.hpp>
#include <boost/bind.hpp>
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

void madwifi_driver_impl::start(const if_list& ifl, boost::system::error_code& ec)
{
	_rtnl.open(sys::netlink<0>(), ec);
	if (ec)
		return;

	_rtnl.bind(sys::netlink<0>::endpoint(rt_link), ec);
	if (ec)
		return;

	for (if_list::const_iterator i = ifl.begin(), e = ifl.end(); i != e; ++i) {
		add_interface(*i, ec);
		if (ec)
			return;
	}

	_rtnl.async_receive(boost::asio::buffer(_buffer),
	                    boost::bind(&madwifi_driver_impl::receive_handler, this, _1, _2));
}

void madwifi_driver_impl::stop(boost::system::error_code& ec)
{
	_rtnl.close(ec);
}

void madwifi_driver_impl::add_interface(const std::string& name, boost::system::error_code& ec)
{
	sys::nl::message<sys::rtnl::link> msg;

	msg.mtype(sys::rtnl::link::m_get);
	msg.flags(sys::nl::header::request | sys::nl::header::ack);

	msg.push_attribute(sys::rtnl::link::attr_ifname, name.c_str(), name.length() + 1);

	_rtnl.send(msg.cbuffer(), 0, ec);
	if (ec)
		return;

	size_t rbytes = _rtnl.receive(boost::asio::buffer(_buffer), 0, ec);
	add_interface_h(ec, rbytes);
}

void madwifi_driver_impl::add_interface_h(boost::system::error_code& ec, size_t rbytes)
{
	if (ec)
		return;

	sys::nl::message_iterator mit(_buffer, rbytes);
	sys::nl::message_iterator end;
	int                       errc = 0;

	for (; mit != end; ++mit) {
		if (mit->type == sys::nl::header::m_error) {
			sys::nl::message<sys::nl::error> err(mit);

			errc = -err->error;
			if (errc) {
				ec = boost::system::error_code(errc, boost::system::get_system_category());
			}
			return;

		} else {
			sys::nl::message<sys::rtnl::link> lnk(mit);
			sys::nl::message<sys::rtnl::link>::attr_iterator ai = lnk.abegin();
			sys::nl::message<sys::rtnl::link>::attr_iterator aend;

			for (; ai != aend; ++ai) {
				switch (ai->type) {
				case sys::rtnl::link::attr_address:
					_address_map[lnk->index] = address_mac(*ai.get<address_mac::bytes_type>());
					break;
				}
			}
		}
	}

	rbytes = _rtnl.receive(boost::asio::buffer(_buffer), 0, ec);
	add_interface_h(ec, rbytes);
}

void madwifi_driver_impl::receive_handler(boost::system::error_code ec, size_t rbytes)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_event_handler.clear();

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
			if (errc) {
				ec = boost::system::error_code(errc, boost::system::get_system_category());
				_event_handler(ec, event());
			}

		} else {
			sys::nl::message<sys::rtnl::link> lnk(mit);
			address_map::iterator             idx = _address_map.find(lnk->index);

			if (idx == _address_map.end())
				continue;

			sys::nl::message<sys::rtnl::link>::attr_iterator ai = lnk.abegin();
			sys::nl::message<sys::rtnl::link>::attr_iterator ae;
			event ev;

			ev.if_index = idx->first;
			ev.if_address = idx->second;

			for (; ai != ae; ++ai) {
				if (ai->type == sys::rtnl::link::attr_wireless) {
					::iw_event* we = ai.get< ::iw_event>();

					switch (we->cmd) {
					case IWEVREGISTERED:
						ev.which = attach;
						ev.mn_address = address_mac(*reinterpret_cast<opmip::ll::mac_address::bytes_type*>(we->u.ap_addr.sa_data));
						_event_handler(ec, ev);
						break;

					case IWEVEXPIRED:
						ev.which = detach;
						ev.mn_address = address_mac(*reinterpret_cast<opmip::ll::mac_address::bytes_type*>(we->u.ap_addr.sa_data));
						_event_handler(ec, ev);
						break;
					}
				}
			}
		}
	}

	_rtnl.async_receive(boost::asio::buffer(_buffer),
	                    boost::bind(&madwifi_driver_impl::receive_handler, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
