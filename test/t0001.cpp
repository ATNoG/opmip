//=============================================================================
// Brief   : 0001 - Test opmip::ip::prefix
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
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	using namespace opmip;

	ip::address_v6 addr(ip::address_v6::from_string("2001:690:2380:7770:218:f3ff:fe90:6d6f"));
	ip::prefix_v6  pref0(addr, 64);
	ip::prefix_v6  pref1(addr, 25);
	ip::prefix_v6  pref2(addr, 15);
	ip::prefix_v6  pref3(addr, 0);

	std::cout << "address: " << addr << std::endl;
	std::cout << "prefix0: "  << pref0 << std::endl;
	std::cout << "prefix1: "  << pref1 << std::endl;
	std::cout << "prefix2: "  << pref2 << std::endl;
	std::cout << "prefix3: "  << pref3 << std::endl;
}

// EOF ////////////////////////////////////////////////////////////////////////
