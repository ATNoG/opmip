//=============================================================================
// Brief   : MLDv2 Unit Test
// Authors : Sérgio Figueiredo <sfigueiredo@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2012 Universidade de Aveiro
// Copyrigth (C) 2012 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#include <boost/asio/ip/icmp.hpp>
#include <opmip/base.hpp>
#include <opmip/ip/icmp.hpp>
#include <opmip/net/ip/mcast_router.hpp>
#include <opmip/net/ip/icmp_generator.hpp>
#include <opmip/net/ip/address.hpp>
#include <iostream>

void send_handler(boost::system::error_code const& ec, size_t bytes)
{
	std::cout << "MLD Query sent " << ec.message() << std::endl;
}

int main(int argc, char* argv[])
{
	using namespace opmip::net::ip;

	boost::asio::io_service ios;
	icmp_mld_query imq;

	imq.group = address_v6::from_string("2002:cafe:cafe::2");
	imq.sources.push_back(address_v6::from_string("2001::1"));
	imq.type = 2;

	boost::asio::ip::icmp::socket sock(ios, boost::asio::ip::icmp::v6());
	boost::asio::ip::icmp::endpoint ep(opmip::ip::address_v6::loopback(), 0);

	unsigned char* buffer = new unsigned char[1460];
	size_t len = icmp_mld_query_generator(imq, buffer, 1460);
	assert(len);

	sock.async_send_to(boost::asio::buffer(buffer, len), ep, send_handler);

	ios.run();
}
