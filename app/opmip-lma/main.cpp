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
#include <opmip/sys/signals.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
void interrupt(opmip::pmip::lma& lma)
{
	std::cout << "\r";
	lma.stop();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cerr << "usage: " << argv[0] << " id node-database\n"
			         "\n"
			         " id                     - this MAG identifier\n"
			         " node-database          - path to node database file\n";
		return 1;
	}

	const char* id                     = argv[1];
	const char* node_database          = argv[2];

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

		opmip::sys::interrupt_signal.connect(boost::bind(interrupt,
		                                                 boost::ref(lma)));

		opmip::sys::init_signals(opmip::sys::signal_mask::interrupt);

		lma.start(id);

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
