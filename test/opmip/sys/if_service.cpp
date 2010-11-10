//=============================================================================
// Brief   : Network Interface Service Test
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
#include <opmip/sys/if_service.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
void event_handler(const boost::system::error_code& ec, const opmip::sys::if_service::event& e)
{
	if (ec) {
		std::cerr << "Error : " << ec.message() << std::endl;
		return;
	}

	std::cout << "Link : {\n"
				 "    which      = " << e.which <<  ",\n"
				 "    if_index   = " << e.if_index <<  ",\n"
				 "    if_type    = " << e.if_type <<  ",\n"
				 "    if_name    = " << e.if_name << ",\n"
				 "    if_flags   = " << std::hex << e.if_flags << std::dec <<  ",\n"
				 "    if_change  = " << std::hex << e.if_change << std::dec <<  ",\n"
				 "    if_state   = " << e.if_state <<  ",\n"
				 "    if_address = " << e.if_address <<  ",\n"
				 "    if_mtu     = " << e.if_mtu <<  ",\n"
				 "    if_wireless.which = " << e.if_wireless.which;

	switch (e.if_wireless.which) {
	case opmip::sys::impl::if_service::wevent_attach:
	case opmip::sys::impl::if_service::wevent_detach:
		std::cout << ",\n"
		             "    if_wireless.address = " << e.if_wireless.address <<  "\n"
		             "}\n";
		break;

	default:
		std::cout << "\n}\n";
		break;
	}
}

int main()
{
	boost::asio::io_service ios;
	opmip::sys::if_service srv(ios);

	srv.start(event_handler);
	ios.run();
}

// EOF ////////////////////////////////////////////////////////////////////////
