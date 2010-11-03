//=============================================================================
// Brief   : opmip::net::link::ethernet::socket - Ipv6 packet send
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

#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/net/ip/icmp6_ra_packet.hpp>
#include <opmip/net/link/ethernet.hpp>
#include <boost/asio/buffer.hpp>
#include <algorithm>
#include <iostream>
#include <new>

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	boost::asio::io_service ios;
	opmip::net::link::ethernet::socket sk(ios);
	opmip::net::link::ethernet::endpoint ep(opmip::net::link::ethernet::ipv6,
	                                        2,
	                                        opmip::net::link::ethernet::endpoint::outgoing,
	                                        opmip::ll::mac_address::from_string("00:11:22:33:44:55"));

	opmip::net::ip::icmp6_ra_packet ra;
	opmip::net::ip::ipv6_packet     pk(opmip::ip::address_v6::from_string("fe80::1"),
	                                   opmip::ip::address_v6::from_string("ff02::1"),
	                                   128,
                                       ra);

	sk.open(opmip::net::link::ethernet(0x0003));
	sk.send_to(pk.cbuffer(), ep);
}

// EOF ////////////////////////////////////////////////////////////////////////
