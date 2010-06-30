//=============================================================================
// Brief   : Local Mobility Anchor Main Entry Point
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
#include <opmip/pmip/lma.hpp>
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
static opmip::pmip::lma* main_service;

void terminate(int)
{
	std::cout << "\r";
	main_service->stop();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	if (argc != 5) {
		std::cerr << "usage: " << argv[0] << " id node-database home-network-link home-network-device-id\n"
			         "\n"
			         " id                     - this MAG identifier\n"
			         " node-database          - path to node database file\n"
			         " home-network-link      - MAG access link local ip6 address\n"
		             " home-network-device-id - MAG access link device id\n";
		return 1;
	}

	const char* id                     = argv[1];
	const char* node_database          = argv[2];
	const char* home_network_link_addr = argv[3];
	const char* home_network_link_id   = argv[4];

	try {
		size_t                  concurrency = boost::thread::hardware_concurrency();
		boost::asio::io_service ios(concurrency);
		opmip::pmip::node_db    ndb;
		opmip::pmip::lma        lma(ios, ndb, concurrency);

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
			main_service = &lma;
			sa.sa_handler = terminate;
			::sigaction(SIGINT, &sa, 0);
		}

		opmip::ip::address_v6 lla(opmip::ip::address_v6::from_string(home_network_link_addr));
		lla.scope_id(boost::lexical_cast<uint>(home_network_link_id));
		lma.start(id, lla);

		boost::thread_group tg;
		for (size_t i = 1; i < concurrency; ++i)
			tg.create_thread(boost::bind(&boost::asio::io_service::run, &ios));

		ios.run();
		tg.join_all();

	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// EOF ////////////////////////////////////////////////////////////////////////
