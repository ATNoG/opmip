//=============================================================================
// Brief   : Netlink Socket Test
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
#include <opmip/sys/netlink.hpp>
#include <opmip/sys/netlink/message.hpp>
#include <opmip/sys/netlink/error.hpp>
#include <opmip/sys/rtnetlink/address.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	using namespace opmip::sys;
	nl::message<rtnl::address> address_msg;
	opmip::ip::address_v6 addr(opmip::ip::address_v6::from_string("2002:c188:5dce:7::10"));
	char resp[1024];
	size_t len;

	boost::asio::io_service ios;
	netlink<0>::socket sock(ios);
	netlink<0>::endpoint ep;

	address_msg.mtype(rtnl::address::m_new);
	address_msg.flags(nl::header::request | nl::header::create | nl::header::ack | nl::header::exclusive);
	address_msg.sequence(time(0));
	address_msg->family = AF_INET6;
	address_msg->prefixlen = 64;
	address_msg->flags = rtnl::address::permanent;
	address_msg->index = 2;

	opmip::ip::address_v6::bytes_type a = addr.to_bytes();
	address_msg.push_attribute(rtnl::address::attr_local, a.elems, a.size());
	address_msg.push_attribute(rtnl::address::attr_address, a.elems, a.size());

	sock.open(netlink<0>());
	sock.bind(ep);

	len = sock.send(address_msg.cbuffer());
	std::cout << "sended " << len << " bytes\n";

	len = sock.receive(boost::asio::buffer(resp));
	std::cout << "received " << len << " bytes\n";

	nl::message_iterator mit(resp, len);
	nl::message_iterator end;

	for (; mit != end; ++mit) {
		switch(mit->type) {
		case nl::header::m_error: {
				nl::message<nl::error> err(mit);

				std::cout << "error: " << err->error << std::endl;
			}
		}
	}
}

// EOF ////////////////////////////////////////////////////////////////////////
