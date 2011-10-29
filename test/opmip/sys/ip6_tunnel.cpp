//=============================================================================
// Brief   : Tunnel Service Test Application
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

#include <opmip/sys/ip6_tunnel.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	using namespace opmip;

	boost::system::error_code ec;
	boost::asio::io_service ios;
	sys::ip6_tunnel tn0(ios);

	tn0.open("pmip0",
	         3,
	         sys::ip::address_v6::from_string("2001:690:2380:7770:226:22ff:fef0:6b94"),
	         sys::ip::address_v6::any(), ec);
	if (ec)
		std::cerr << "failed to create tunnel: " << ec.message() << std::endl;

	tn0.delete_on_close(false);

	tn0.add_address(sys::ip::address_v6::from_string("2001:690:2380:7770:226:cafe:dead:beef"), 64, ec);
	if (ec)
		std::cerr << "failed to add tunnel address: " << ec.message() << std::endl;

	tn0.set_enable(true, ec);
	if (ec)
		std::cerr << "failed to set tunnel up: " << ec.message() << std::endl;
}

// EOF ////////////////////////////////////////////////////////////////////////
