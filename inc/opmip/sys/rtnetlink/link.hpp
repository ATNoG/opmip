//=============================================================================
// Brief   : RT Netlink Link Message
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

#ifndef OPMIP_SYS_RTNETLINK_LINK__HPP_
#define OPMIP_SYS_RTNETLINK_LINK__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace rtnetlink {

///////////////////////////////////////////////////////////////////////////////
class link {
	enum flags {
		up             = 0x00001, ///Interface is up
		broadcast      = 0x00002, ///Broadcast address valid
		debug          = 0x00004, ///Turn on debugging
		loopback       = 0x00008, ///Is a loopback net
		point_to_point = 0x00010, ///Interface is has p-p link
		no_trailers    = 0x00020, ///Avoid use of trailers
		running        = 0x00040, ///Interface RFC2863 OPER_UP
		no_arp         = 0x00080, ///No ARP protocol
		promiscuous    = 0x00100, ///Receive all packets
		all_multicast  = 0x00200, ///Receive all multicast packets
		master         = 0x00400, ///Master of a load balancer
		slave          = 0x00800, ///Slave of a load balancer
		multicast      = 0x01000, ///Supports multicast
		portsel        = 0x02000, ///Can set media type
		automedia      = 0x04000, ///Auto media select active
		dynamic        = 0x08000, ///Dialup device with changing addresses
		lower_up       = 0x10000, ///Driver signals L1 up
		dormant        = 0x20000, ///Driver signals dormant
		echo           = 0x40000, ///Echo sent packets
	};

	enum type {
		ethernet            = 1,   ///Ethernet 10Mbps
		ieee802_2           = 6,   ///FastEthernet 100Mbps
		ieee802_11          = 801, ///Wireless 802.11
		ieee802_11_prism    = 802, ///Wireless 802.11 + Prism
		ieee802_11_radiotap = 803, ///Wireless 802.11 + Radiotap
	};

private:
	uchar  _family;
	uchar  _pad;
	ushort _type;
	int    _index;
	uint   _flags;
	uint   _change;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace rtnetlink */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_RTNETLINK_MESSAGES__HPP_ */
