//=============================================================================
// Brief   : Mobile Access Gateway Main Entry Point
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
#include <opmip/pmip/mag.hpp>
#include <opmip/pmip/node_db.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////
void link_sap(opmip::pmip::mag& mag, const opmip::ip::address_v6& ll_ip_address, const opmip::ll::mac_address& ll_mac_address);
static opmip::pmip::mag* main_service;

void terminate(int)
{
	std::cout << "\r";
	main_service->stop();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	if (argc != 6) {
		std::cerr << "usage: " << argv[0] << " id node-database mn-access-link-ip-address mn-access-link-mac-address mn-access-link-device-id\n"
			         "\n"
			         " id                         - this MAG identifier\n"
			         " node-database              - path to node database file\n"
			         " mn-access-link-ip-address  - mobile node(s) access link local ip6 address\n"
			         " mn-access-link-mac-address - mobile node(s) access link mac address\n"
			         " mn-access-link-device-id   - mobile node(s) access link device id\n";
		return 1;
	}

	const char* id               = argv[1];
	const char* node_database    = argv[2];
	const char* access_link_addr = argv[3];
	const char* access_link_mac  = argv[4];
	const char* access_link_id   = argv[5];

	try {
		size_t                  concurrency = boost::thread::hardware_concurrency();
		boost::asio::io_service ios(concurrency);
		opmip::pmip::node_db    ndb;
		opmip::pmip::mag        mag(ios, ndb, concurrency);

		{
			std::ifstream in(node_database);

			if (!in) {
				std::cerr << "Failed to open \"" << node_database << " node database file\n";
				return 1;
			}

			size_t n = ndb.load(in);
			std::cout << "Loaded " << n << " nodes from database\n";
		}

		{
			struct ::sigaction sa;

			std::memset(&sa, 0, sizeof(sa));
			main_service = &mag;
			sa.sa_handler = terminate;
			::sigaction(SIGINT, &sa, 0);
		}

		opmip::ip::address_v6 lla(opmip::ip::address_v6::from_string(access_link_addr));
		lla.scope_id(boost::lexical_cast<uint>(access_link_id));
		mag.start(id, lla);

		boost::thread_group tg;
		for (size_t i = 1; i < concurrency; ++i)
			tg.create_thread(boost::bind(&boost::asio::io_service::run, &ios));

		opmip::ll::mac_address mac(opmip::ll::mac_address::from_string(access_link_mac));
		boost::thread* td = tg.create_thread(boost::bind(link_sap, boost::ref(mag), boost::cref(lla), boost::cref(mac)));

		ios.run();
		td->interrupt();
		td->detach();
		tg.join_all();

	} catch(std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// EOF ////////////////////////////////////////////////////////////////////////
