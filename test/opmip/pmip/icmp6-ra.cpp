//=============================================================================
// Brief   : opmip::pmip::icmp_sender
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
#include <opmip/pmip/icmp_sender.hpp>
#include <opmip/net/ip/icmp6_ra_packet.hpp>
#include <opmip/net/link/ethernet.hpp>
#include <boost/bind.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
void foo()
{
}

int main(int argc, char** argv)
{
	boost::asio::io_service ios;
	opmip::net::link::ethernet::socket sk(ios);
	opmip::net::link::ethernet::endpoint ep(opmip::net::link::ethernet::ipv6,
	                                        2,
	                                        opmip::net::link::ethernet::endpoint::outgoing,
	                                        opmip::ll::mac_address::from_string("00:11:22:33:44:55"));

	sk.open(opmip::net::link::ethernet(0x0003));

	opmip::pmip::router_advertisement_info rainfo;

	rainfo.link_address = opmip::ll::mac_address::from_string("00:12:34:56:78:90");
	rainfo.prefix_list.push_back(opmip::ip::prefix_v6::from_string("3000::1/64"));
	rainfo.destination = opmip::ip::address_v6::from_string("ff02::1");

	opmip::pmip::icmp_ra_sender_ptr pk = new opmip::pmip::icmp_ra_sender(rainfo);

	pk->async_send(sk, ep, boost::bind(foo));
	ios.run();
}

// EOF ////////////////////////////////////////////////////////////////////////
