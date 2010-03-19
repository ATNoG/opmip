//=============================================================================
// Brief   : <Brief Description>
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

#include <opmip/exception.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	try {
		opmip::error_code ec(EPERM, boost::system::get_system_category());
		opmip::exception exp(ec, "Example 1");
		OPMIP_THROW_EXCEPTION(exp);

	} catch (opmip::exception& e) {
		std::cout << e.what() << std::endl;
	}

	try {
		opmip::error_code ec(0, boost::system::get_system_category());
		opmip::exception exp(ec, "Example 2");
		opmip::throw_exception(exp);

	} catch (opmip::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

// EOF ////////////////////////////////////////////////////////////////////////
