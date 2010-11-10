//=============================================================================
// Brief   : Route Table Test
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
#include <opmip/sys/route_table.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	using namespace opmip;
	boost::asio::io_service ios;
	sys::route_table rt(ios);

	ip::prefix_v6 pref1(ip::address_v6::from_string("3001:c188:5d9a:a::"), 64);
	ip::prefix_v6 pref2(ip::address_v6::from_string("3001:c188:5d9a:b::"), 64);

	rt.add_by_dst(pref1, 2, ip::address_v6::from_string("2001:106:2222::1"));
	rt.add_by_dst(pref2, 3);
	rt.remove_by_dst(pref1);
	rt.remove_by_dst(pref2);

	rt.add_by_src(pref1, 2, ip::address_v6::from_string("2001:106:2222::1"));
	rt.add_by_src(pref2, 3);
	rt.remove_by_src(pref1);
	rt.remove_by_src(pref2);
}

// EOF ////////////////////////////////////////////////////////////////////////
