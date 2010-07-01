//=============================================================================
// Brief   : RTNetlink
// Authors : Bruno Santos <bsantos@av.it.pt>
//
//
// Copyright (C) 2009 Universidade Aveiro - Instituto de Telecomunicacoes Polo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
#include "rtnetlink.hpp"
#include <boost/assert.hpp>

//
// Including linux headers is painfull
//
#ifndef __user
#define __user
#endif

#include <net/if.h>
#include <net/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "wireless.h"

///////////////////////////////////////////////////////////////////////////////
OPMIP_STATIC_ASSERT(rtnetlink::link          == RTMGRP_LINK, "");
OPMIP_STATIC_ASSERT(rtnetlink::notify        == RTMGRP_NOTIFY, "");
OPMIP_STATIC_ASSERT(rtnetlink::neigh         == RTMGRP_NEIGH, "");
OPMIP_STATIC_ASSERT(rtnetlink::tc            == RTMGRP_TC, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv4_ifaddr   == RTMGRP_IPV4_IFADDR, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv4_mroute   == RTMGRP_IPV4_MROUTE, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv4_route    == RTMGRP_IPV4_ROUTE, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv4_rule     == RTMGRP_IPV4_RULE, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv6_ifaddr   == RTMGRP_IPV6_IFADDR, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv6_mroute   == RTMGRP_IPV6_MROUTE, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv6_route    == RTMGRP_IPV6_ROUTE, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv6_ifinfo   == RTMGRP_IPV6_IFINFO, "");
OPMIP_STATIC_ASSERT(rtnetlink::DECnet_ifaddr == RTMGRP_DECnet_IFADDR, "");
OPMIP_STATIC_ASSERT(rtnetlink::DECnet_route  == RTMGRP_DECnet_ROUTE, "");
OPMIP_STATIC_ASSERT(rtnetlink::ipv6_prefix   == RTMGRP_IPV6_PREFIX, "");

OPMIP_STATIC_ASSERT(rtnetlink::if_link::ethernet   == ARPHRD_ETHER, "");
OPMIP_STATIC_ASSERT(rtnetlink::if_link::ieee802_11 == ARPHRD_IEEE80211, "");

OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::up             == IFF_UP, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::broadcast      == IFF_BROADCAST, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::debug          == IFF_DEBUG, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::loopback       == IFF_LOOPBACK, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::point_to_point == IFF_POINTOPOINT, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::no_trailers    == IFF_NOTRAILERS, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::running        == IFF_RUNNING, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::no_arp         == IFF_NOARP, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::promiscuous    == IFF_PROMISC, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::all_multicast  == IFF_ALLMULTI, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::master         == IFF_MASTER, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::slave          == IFF_SLAVE, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::multicast      == IFF_MULTICAST, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::portsel        == IFF_PORTSEL, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::automedia      == IFF_AUTOMEDIA, "");
OPMIP_STATIC_ASSERT((uint) rtnetlink::if_link::dynamic        == IFF_DYNAMIC, "");

///////////////////////////////////////////////////////////////////////////////
rtnetlink::if_link::if_link()
	: _type(0), _index(0), _flags(0), _mtu(opmip::nullptr), _link_type(opmip::nullptr),
	  _wevent(we_unknown)
{
}

rtnetlink::if_link::if_link(message& msg)
	: _type(0), _index(0), _flags(0), _mtu(opmip::nullptr), _link_type(opmip::nullptr),
	  _wevent(we_unknown)
{
	*this = msg;
}

rtnetlink::if_link& rtnetlink::if_link::operator=(message& msg)
{
	static_cast<data&>(*this) = msg;

	std::pair<const void*, opmip::size_t> pl = msg.payload();
	const ::ifinfomsg* ifi = reinterpret_cast<const ifinfomsg*>(pl.first);

	BOOST_ASSERT((msg.type() >= 16 || msg.type() <= 19 || pl.second < sizeof(::ifinfomsg)));

	_type = ifi->ifi_type;
	_index = ifi->ifi_index;
	_flags = ifi->ifi_flags;

	const ::rtattr* rta = IFLA_RTA(ifi);
	opmip::size_t attrlen = IFLA_PAYLOAD(reinterpret_cast<const ::nlmsghdr*>(_msg->header()));

	for (; RTA_OK(rta, attrlen); rta = RTA_NEXT(rta, attrlen)) {
		switch (rta->rta_type) {
		case IFLA_IFNAME:
			_name = attr<char>(RTA_DATA(rta), RTA_PAYLOAD(rta));
			break;

		case IFLA_MTU:
			BOOST_ASSERT(sizeof(opmip::uint) == RTA_PAYLOAD(rta));
			_mtu = reinterpret_cast<opmip::uint*>(RTA_DATA(rta));
			break;

		case IFLA_LINK:
			BOOST_ASSERT(sizeof(opmip::sint) == RTA_PAYLOAD(rta));
			_link_type = reinterpret_cast<opmip::sint*>(RTA_DATA(rta));
			break;

		case IFLA_WIRELESS:
			::iw_event* we = reinterpret_cast< ::iw_event*>(RTA_DATA(rta));

			switch (we->cmd) {
			case IWEVREGISTERED:
				_wevent = we_attach;
				_waddress = opmip::ll::mac_address(*reinterpret_cast<opmip::ll::mac_address::bytes_type*>(we->u.ap_addr.sa_data));
				break;

			case IWEVEXPIRED:
				_wevent = we_detach;
				_waddress = opmip::ll::mac_address(*reinterpret_cast<opmip::ll::mac_address::bytes_type*>(we->u.ap_addr.sa_data));
				break;

			default:
				_wevent = we_unknown;
				_waddress = opmip::ll::mac_address();
			}
			break;
		}
	}

	return *this;
}

// EOF ////////////////////////////////////////////////////////////////////////
