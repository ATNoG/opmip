//=============================================================================
// Brief   : PIMv2 Unit Test
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

#include <opmip/base.hpp>
#include <opmip/net/ip/pim.hpp>
#include <opmip/net/ip/pim_gen_parser.hpp>
#include <opmip/net/ip/mcast_router.hpp>
#include <opmip/net/ip/address.hpp>
#include <iostream>

void h_send_handler(boost::system::error_code const& ec, size_t bytes)
{
	std::cout << "PIM Hello sent: " << ec.message() << std::endl;
}

void j_send_handler(boost::system::error_code const& ec, size_t bytes)
{
	std::cout << "PIM Join-Prune sent: " << ec.message() << std::endl;
}

int main(int argc, char* argv[])
{
	using namespace opmip::net::ip;

	boost::asio::io_service ios;
	unsigned char* buffer = new unsigned char[1460];
	std::fill(buffer, buffer + 1460, 0);
	size_t len = 0;

	pim::socket sock(ios, pim());
	pim::endpoint ep(address_v6::loopback());

	int test = 0;

	switch (test)
	{
		case 0: {
			hello_msg msg;

			msg.holdtime = 5;
			msg.dr_priority = 7;
			msg.generation_id = 9;
			msg.maddr_list.push_back(address_v6::from_string("2001::cece:bebe"));

			len = msg.gen(buffer, 3000);
			assert(len);
		}	break;
		case 1 : {
			join_prune_msg msg;
			msg.uplink = address_v6::from_string("2011::4331");

			join_prune_msg::mcast_group g;
			g.group = address_v6::from_string("2013::1376");
			g.joins.push_back(address_v6::from_string("2013::1234"));
			g.prunes.push_back(address_v6::from_string("2013::ffff"));
			msg.mcast_groups.push_back(g);

			join_prune_msg::mcast_group j;
			j.group = address_v6::from_string("2014::1233");
			j.joins.push_back(address_v6::from_string("2014::dead:beaf"));
			j.prunes.push_back(address_v6::from_string("2014::dead:b00b"));
			msg.mcast_groups.push_back(j);

			len = msg.gen(buffer, 3000);
		}
		break;
		default :
			return 0;
	}

	if (test == 0)
		sock.async_send_to(boost::asio::buffer(buffer, len), ep, h_send_handler);
	else
		sock.async_send_to(boost::asio::buffer(buffer, len), ep, j_send_handler);

	ios.run();
}
