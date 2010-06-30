//=============================================================================
// Brief   : Unit Test for Mobility Protocol Socket
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

#include <opmip/ip/prefix.hpp>
#include <opmip/ip/icmp.hpp>
#include <opmip/ip/icmp_options.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
static void send_ra(boost::asio::ip::icmp::socket& sock, boost::asio::ip::icmp::endpoint& ep);

int main()
{
	boost::asio::io_service         ios;
	boost::asio::ip::icmp::endpoint ep(opmip::ip::address_v6::loopback(), 0);
	boost::asio::ip::icmp::socket   sock(ios, ep);

	send_ra(sock, ep);
}

static void send_ra(boost::asio::ip::icmp::socket& sock, boost::asio::ip::icmp::endpoint& ep)
{
	using namespace opmip;
	typedef std::vector<ip::prefix_v6> prefix_list;
	char buffer[1460];
	prefix_list preflist;

	preflist.push_back(ip::prefix_v6(ip::address_v6::from_string("2002:c188:5db5::"), 64));
	preflist.push_back(ip::prefix_v6(ip::address_v6::from_string("2002:c188:5d84::"), 64));
	preflist.push_back(ip::prefix_v6(ip::address_v6::from_string("2001:6900:2380::"), 64));

	ip::icmp::router_advertisement* ra = new(buffer) ip::icmp::router_advertisement();
	::size_t                        len = sizeof(ip::icmp::router_advertisement);

	ra->lifetime(~0);

	//
	// Source link layer
	//
	ip::opt_source_link_layer* sll = new(buffer + len) ip::opt_source_link_layer();

	*sll = ll::mac_address::from_string("00:18:f3:90:6d:00");
	len += ip::option::size(sll);

	//
	// MTU
	//
	ip::opt_mtu* mtu = new(buffer + len) ip::opt_mtu();

	mtu->set(1460);
	len += ip::option::size(mtu);

	//
	// Prefixes
	//
	for (prefix_list::const_iterator i = preflist.begin(), e = preflist.end(); i != e; ++i) {
		ip::opt_prefix_info* pref = new(buffer + len) ip::opt_prefix_info();

		pref->L(true);
		pref->A(true);
		pref->valid_lifetime(7200);
		pref->prefered_lifetime(1800);
		pref->prefix(*i);

		len += ip::option::size(pref);
	}

	std::cout << "Sended = " << len << std::endl;
	sock.send_to(boost::asio::buffer(buffer, len), ep);
}

// EOF ////////////////////////////////////////////////////////////////////////
