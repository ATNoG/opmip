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

#include <opmip/ip/mproto.hpp>
#include <opmip/ip/mproto_buffer.hpp>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	boost::asio::io_service ios;
	opmip::ip::mproto::endpoint ep(opmip::ip::address_v6::loopback());
	opmip::ip::mproto::socket sock(ios, opmip::ip::mproto::endpoint());
	opmip::ip::mproto::pbu msg1;
	opmip::ip::mproto::pba msg2;

	msg1.sequence(1234);
	msg1.a(true);
	msg1.p(true);
	msg1.lifetime(4321);

	msg2.sequence(3412);
	msg2.k(true);
	msg2.p(true);
	msg2.lifetime(2143);

	sock.send_to(opmip::ip::mproto_cbuffer(msg1), ep);
	sock.send_to(opmip::ip::mproto_cbuffer(msg2), ep);
}

// EOF ////////////////////////////////////////////////////////////////////////
