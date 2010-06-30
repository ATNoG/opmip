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
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
static void send_pba(opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep);
static void receive_pba(opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep);

int main()
{
	boost::asio::io_service     ios;
	opmip::ip::mproto::endpoint ep(opmip::ip::address_v6::loopback());
	opmip::ip::mproto::socket   sock(ios, opmip::ip::mproto::endpoint());

	send_pba(sock, ep);
	receive_pba(sock, ep);
}

static void send_pba(opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep)
{
	using namespace opmip;
	char buffer[1460];

	std::fill(buffer, buffer + sizeof(buffer), 0);

	std::string         id("mobile-node");
	ip::mproto::pba*    pba = new(buffer) ip::mproto::pba;
	ip::mproto::option* opt;
	::size_t            len = sizeof(ip::mproto::pba);

	pba->status(ip::mproto::pba::status_ok);
	pba->proxy_reg(true);
	pba->sequence(1234);
	pba->lifetime(~0);

	//
	// NAI Option
	//
	ip::mproto::option::nai* nai;

	opt = new(buffer + len) ip::mproto::option(ip::mproto::option::nai(), id.length());
	nai = opt->get<ip::mproto::option::nai>();
	nai->subtype = 1;
	std::copy(id.begin(), id.end(), nai->id);
	len += ip::mproto::option::size(opt);

	//
	// Handoff Option
	//
	ip::mproto::option::handoff* hof;

	opt = new(buffer + len) ip::mproto::option(ip::mproto::option::handoff());
	hof = opt->get<ip::mproto::option::handoff>();
	hof->indicator = 4; //Handoff state unknown
	len += ip::mproto::option::size(opt);

	//
	//	Access Type Technology
	//
	ip::mproto::option::att* att;

	opt = new(buffer + len) ip::mproto::option(ip::mproto::option::att());
	att = opt->get<ip::mproto::option::att>();
	att->tech_type = ip::mproto::option::att::ieee802_11abg;
	len += ip::mproto::option::size(opt);

	len = align_to<8>(len);
	pba->init(ip::mproto::pba::mh_type, len);

	std::cout << "Sended = " << len << std::endl;
	sock.send_to(boost::asio::buffer(buffer, len), ep);
}

static void receive_pba(opmip::ip::mproto::socket& sock, opmip::ip::mproto::endpoint& ep)
{
	using namespace opmip;
	char     buffer[1460];
	::size_t len;

	std::fill(buffer, buffer + sizeof(buffer), 0);
	len = sock.receive_from(boost::asio::buffer(buffer), ep);
	std::cout << "Received = " << len << std::endl;

	ip::mproto::header* hdr = ip::mproto::header::cast(buffer, len);
	ip::mproto::pba*    pba = ip::mproto::pba::cast(hdr);
	ip::mproto::option* opt;
	::size_t            pos = sizeof(ip::mproto::pba);
	std::string         id;

	std::cout << "pba: sequence " << pba->sequence() << std::endl;
	std::cout << "pba: lifetime " << pba->lifetime() << std::endl;

	if (pba->proxy_reg())
		std::cout << "pba: proxy registration\n";
	else
		std::cerr << "pba: no proxy registration\n";

	if (pba->k() || pba->r())
		std::cerr << "pba: invalid flag options " << (pba->k() ? 'k' : '-')
		                                          << (pba->r() ? 'r' : '-') << std::endl;

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
					std::cerr << "option identifier: duplicate ";
				else
					std::cerr << "option identifier: ";
				id.assign(nai->id, opt->length - 1);
				std::cout << " NAI " << id << std::endl;
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
