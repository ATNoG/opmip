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
#include <opmip/pmip/mp_sender.hpp>
#include <opmip/pmip/mp_receiver.hpp>
#include <boost/bind.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
static void send_pbu(boost::asio::io_service& ios, opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep);
static void receive_pbu(boost::asio::io_service& ios, opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep);

inline void send_handler()
{
}

int main()
{
	boost::asio::io_service     ios;
	opmip::ip::mproto::endpoint ep(opmip::ip::address_v6::loopback());
	opmip::ip::mproto::socket   sock(ios, opmip::ip::mproto::endpoint());

	send_pbu(ios, sock, ep);
	receive_pbu(ios, sock, ep);
}

static void send_pbu(boost::asio::io_service& ios, opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep)
{
	opmip::pmip::proxy_binding_info pbinfo;


	pbinfo.id = "mobile-node";
	pbinfo.sequence = 1234;
	pbinfo.lifetime = ~0;

	opmip::pmip::pbu_sender_ptr pbus(new opmip::pmip::pbu_sender(pbinfo));

	pbus->async_send(sock, boost::bind(&send_handler));
	ios.run();
}


static void receive_pbu(boost::asio::io_service& ios, opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep)
{
	using namespace opmip;
	char     buffer[1460];
	::size_t len;

	std::fill(buffer, buffer + sizeof(buffer), 0);
	len = sock.receive_from(boost::asio::buffer(buffer), ep);
	std::cout << "Received = " << len << std::endl;

	ip::mproto::header* hdr = ip::mproto::header::cast(buffer, len);
	ip::mproto::pbu*    pbu = ip::mproto::pbu::cast(hdr);
	ip::mproto::option* opt;
	::size_t            pos = sizeof(ip::mproto::pbu);
	std::string         id;

	std::cout << "pbu: sequence " << pbu->sequence() << std::endl;
	std::cout << "pbu: lifetime " << pbu->lifetime() << std::endl;

	if (pbu->ack())
		std::cout << "pbu: acknowledge request\n";

	if (pbu->proxy_reg())
		std::cout << "pbu: proxy registration\n";
	else
		std::cerr << "pbu: no proxy registration\n";

	if (pbu->h() || pbu->l() || pbu->k() || pbu->m() || pbu->r())
		std::cerr << "pbu: invalid flag options " << (pbu->h() ? 'h' : '-')
		                                          << (pbu->l() ? 'l' : '-')
		                                          << (pbu->k() ? 'k' : '-')
		                                          << (pbu->m() ? 'm' : '-')
		                                          << (pbu->r() ? 'r' : '-') << std::endl;

	while ((pos < len) && (opt = ip::mproto::option::cast(buffer + pos, len - pos))) {
		pos += ip::mproto::option::size(opt);
		switch (opt->type) {
		case 0:
		case 1:
			std::cout << "option padding\n";
			break;

		case ip::mproto::option::nai::type_value: {
				ip::mproto::option::nai* nai = opt->get<ip::mproto::option::nai>();

				if (nai->subtype != 1) {
					std::cerr << "option identifier: unsupported subtype (" << opmip::uint(nai->subtype) << ")\n";
					break;
				}
				if (!id.empty())
					std::cout << "option identifier: duplicate";
				else
					std::cout << "option identifier: ";
				id.assign(nai->id, opt->length - 1);
				std::cout << id << std::endl;
			}
			break;

		case ip::mproto::option::handoff::type_value: {
				ip::mproto::option::handoff* hof = opt->get<ip::mproto::option::handoff>();

				std::cout << "option handoff: " << opmip::uint(hof->indicator) << std::endl;
			}
			break;

		case ip::mproto::option::att::type_value: {
				ip::mproto::option::att* att = opt->get<ip::mproto::option::att>();

				std::cout << "option access technology type: " << opmip::uint(att->tech_type) << std::endl;
			}
			break;

		default:
			std::cerr << "unknow option: " << opmip::uint(opt->type) << std::endl;
		}
	}
}

// EOF ////////////////////////////////////////////////////////////////////////
