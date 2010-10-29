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
#include <opmip/ip/icmp.hpp>
#include <opmip/net/link/ethernet.hpp>
#include <boost/asio/buffer.hpp>
#include <algorithm>
#include <iostream>
#include <new>

///////////////////////////////////////////////////////////////////////////////
struct ipv6_header {
	opmip::uint8  version;
	opmip::uint8  flow_label[3];
	opmip::uint16 payload_length;
	opmip::uint8  next_header;
	opmip::uint8  hop_limit;
	opmip::uint8  src_addr[16];
	opmip::uint8  dst_addr[16];
};

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	opmip::ip::address_v6 src = opmip::ip::address_v6::from_string("fe80::1");
	opmip::ip::address_v6 dst = opmip::ip::address_v6::from_string("ff02::1");
	opmip::uchar buffer[sizeof(ipv6_header) + sizeof(opmip::ip::icmp::router_advertisement)];
	ipv6_header* hdr = reinterpret_cast<ipv6_header*>(buffer);

	std::fill(buffer, buffer + sizeof(buffer), 0);

	hdr->version = 0x60;
	hdr->payload_length = htons(sizeof(opmip::ip::icmp::router_advertisement));
	hdr->hop_limit = 128;
	hdr->next_header = 58; //icmpv6
	*reinterpret_cast<opmip::ip::address_v6::bytes_type*>(hdr->src_addr) = src.to_bytes();
	*reinterpret_cast<opmip::ip::address_v6::bytes_type*>(hdr->dst_addr) = dst.to_bytes();

	opmip::ip::icmp::router_advertisement* ra
		= new(buffer + sizeof(ipv6_header)) opmip::ip::icmp::router_advertisement;

	opmip::ll::mac_address hdst = opmip::ll::mac_address::from_string("00:11:22:33:44:55");
	boost::asio::io_service ios;
	opmip::net::link::ethernet::socket sk(ios);
	opmip::net::link::ethernet::endpoint ep(opmip::net::link::ethernet::ipv6,
	                                        2,
	                                        opmip::net::link::ethernet::endpoint::outgoing,
	                                        hdst);

	sk.open(opmip::net::link::ethernet(0x0003));
	sk.send_to(boost::asio::buffer(buffer), ep);
}

// EOF ////////////////////////////////////////////////////////////////////////
