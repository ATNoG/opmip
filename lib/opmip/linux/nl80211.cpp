//=============================================================================
// Brief   : Netlink 802.11 Interface
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

#include <opmip/linux/nl80211/nl80211.hpp>
#include <opmip/linux/genetlink/message.hpp>
#include <opmip/linux/netlink/message_iterator.hpp>
#include <opmip/linux/netlink/attribute_iterator.hpp>
#include <opmip/ll/mac_address.hpp>
#include <opmip/sys/error.hpp>
#include <boost/bind.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux_ {

///////////////////////////////////////////////////////////////////////////////
struct ieee80211_mgm_frame {
	uint16 control;  // Little endian
	uint16 duration;
	uint8  da[6];
	uint8  sa[6];
	uint8  bssid[6];
	//...
};

enum nl80211_cmds {
	nl80211_cmd_new_sta = 19,
	nl80211_cmd_authenticate = 37,
	nl80211_cmd_associate,
	nl80211_cmd_deauthenticate,
	nl80211_cmd_disassociate,
};

enum nl80211_attrs {
	nl80211_attr_wiphy = 1,
	nl80211_attr_if_index = 3,
	nl80211_attr_mac = 6,
	nl80211_attr_frame = 51,
	nl80211_attr_sta_info = 21,
	nl80211_attr_generation = 46,
};

///////////////////////////////////////////////////////////////////////////////
static void parse_attributes(genetlink::message& msg, nl80211::event& ev)
{
	std::cout << std::endl;

	for (netlink::attribute_iterator ai(msg); ai; ++ai) {
		switch (ai.type()) {
		case nl80211_attr_wiphy:
			ev.phy_id = *ai.get<uint32>();
			break;

		case nl80211_attr_if_index:
			ev.if_index = *ai.get<uint32>();
			break;

		case nl80211_attr_mac:
			ev.mac = ll::mac_address(*ai.get<ll::mac_address::bytes_type>());
			break;

		case nl80211_attr_frame: {
			ieee80211_mgm_frame* fm = ai.get<ieee80211_mgm_frame>();

			//uint version = (fm->control >> 14);

			ev.src_addr = ll::mac_address(fm->sa);
			ev.dst_addr = ll::mac_address(fm->da);
			ev.bssid = ll::mac_address(fm->bssid);
		} break;

		//case nl80211_attr_sta_info:
		//	ev.sta_info = *ai.get<uint32>();
		//	break;

		case nl80211_attr_generation:
			ev.generation = *ai.get<uint32>();
			break;

		default:
			std::cout << "     unknown attr = " << uint(ai.type()) << ", " << ai.length() << std::endl;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
nl80211::nl80211(boost::asio::io_service& ios)
	: _gnl(ios), _family(0), _version(0), _hdrsize(0), _mlne_mcast_id(0)
{
}

void nl80211::open_()
{
	_gnl.open(genetlink());
	_gnl.bind(genetlink::endpoint());

	boost::system::error_code ec;
	setup_family(ec);
	sys::throw_on_error(ec);

	_gnl.set_option(genetlink::multicast(genetlink::multicast::add, _mlne_mcast_id));
	_gnl.async_receive(boost::asio::buffer(_buffer), boost::bind(&nl80211::receive_handler, this, _1, _2));
}

void nl80211::close()
{
	_gnl.close();
}

void nl80211::setup_family(boost::system::error_code& ec)
{
	genetlink::message msg;

	msg.flags(netlink::message::request | netlink::message::ack);
	msg.command(genetlink::message::cmd_get_family);
	msg.version(1);
	msg.push_attr(genetlink::message::attr_family_name, "nl80211", 8);

	_gnl.send(msg.cbuffer());

	size_t rbytes = _gnl.receive(boost::asio::buffer(_buffer));
	for (netlink::message_iterator mit(_buffer, rbytes); mit; ++mit) {
		if (mit.type() == netlink::message::error) {
			ec = boost::system::error_code(mit.error(), boost::system::get_system_category());
			return;
		}

		if (mit.type() == netlink::message::min_type) {
			genetlink::message rmsg(mit.get());

			for (netlink::attribute_iterator ai(rmsg); ai; ++ai) {
				switch (ai.type()) {
				case genetlink::message::attr_family_id:
					_family = *ai.get<uint16>();
					break;

				case genetlink::message::attr_family_name:
					BOOST_ASSERT(!strcmp(ai.get<const char>(), "nl80211"));
					break;

				case genetlink::message::attr_version:
					_version = *ai.get<uint32>();
					break;

				case genetlink::message::attr_hdrsize:
					_hdrsize = *ai.get<uint32>();
					break;

				case genetlink::message::attr_mcast_groups:
					for (netlink::attribute_iterator ali(rmsg, ai); ali; ++ali) {
						const char* name = nullptr;
						uint32      id = 0;

						for (netlink::attribute_iterator aei(rmsg, ali); aei; ++aei) {
							switch (aei.type()) {
							case genetlink::message::attr_mcast_name:
								name = aei.get<const char>();
								break;

							case genetlink::message::attr_mcast_id:
								id = *aei.get<uint32>();
								break;
							}
						}

						if (name && !strcmp(name, "mlme")) {
							_mlne_mcast_id = id;
							break;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

/*void nl80211::setup_frame(boost::system::error_code& ec)
{
	genetlink::message msg;

	msg.type(_family);
	msg.flags(netlink::message::request | netlink::message::ack);
	msg.command(nl80211_cmd_register_action);
	msg.version(_version);

	uint32 if_idx = 3;
	uint8  frame[1] = { '\x06' };

	msg.push_attr(nl80211_attr_if_index, &if_idx, sizeof(id_idx));
	msg.push_attr(nl80211_attr_frame_match, frame, sizeof(frame));

	_gnl.send(msg.cbuffer());

	size_t rbytes = _gnl.receive(boost::asio::buffer(_buffer));
	for (netlink::message_iterator mit(_buffer, rbytes); mit; ++mit) {
		if (mit.type() == netlink::message::error) {
			ec = boost::system::error_code(mit.error(), boost::system::get_system_category());
			return;
		}

		if (mit.type() == _family) {
		}
	}
}*/

void nl80211::receive_handler(const boost::system::error_code& ec, size_t rbytes)
{
	if (ec) {
		_event_handler(ec, event());
		return;
	}

	for (netlink::message_iterator mit(_buffer, rbytes); mit; ++mit) {
		if (mit.type() == netlink::message::error) {
			int error = mit.error();
			if (error) {
				boost::system::error_code err(error, boost::system::get_system_category());
				_event_handler(err, event());
			}

		} else if (mit.type() == _family) {
			genetlink::message msg(mit.get());

			if (msg)
				message_handler(msg);
			else
				std::cerr << "warning: invalid nl80211 message"
				             "{\n"
				             "    method : opmip::linux::nl80211::event_handler()\n"
				             "    nltype : " << uint(msg.type()) <<
				             "}\n";

		} else {
			std::cerr << "warning: unexpected netlink message type\n"
			             "{\n"
			             "    method : opmip::linux::nl80211::event_handler()\n"
			             "    nltype : " << uint(mit.type()) <<
			             "}\n";
		}
	}

	_gnl.async_receive(boost::asio::buffer(_buffer), boost::bind(&nl80211::receive_handler, this, _1, _2));
}

void nl80211::message_handler(genetlink::message& msg)
{
	event ev;

	std::cout << "nl80211: cmd = " << uint(msg.command());
	switch (msg.command()) {
	case nl80211_cmd_new_sta:
		ev.which = event::new_sta;
		parse_attributes(msg, ev);
		break;

	case nl80211_cmd_authenticate:
	case nl80211_cmd_associate:
		ev.which = event::associate;
		parse_attributes(msg, ev);
		break;

	case nl80211_cmd_deauthenticate:
	case nl80211_cmd_disassociate:
		ev.which = event::disassociate;
		parse_attributes(msg, ev);
		break;

	default:
		std::cout << " (don't care)\n";
		return;
	}

	_event_handler(boost::system::error_code(), ev);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */ } /* namespace linux */

// EOF ////////////////////////////////////////////////////////////////////////
