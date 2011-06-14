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
#include <opmip/debug.hpp>
#include <opmip/exception.hpp>
#include <opmip/pmip/lma.hpp>
#include <opmip/pmip/node_db.hpp>
#include <opmip/sys/signals.hpp>
#include "options.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
static opmip::logger log_("opmip-lma", std::cout);

///////////////////////////////////////////////////////////////////////////////
static void load_node_database(const std::string& file_name, opmip::pmip::node_db& ndb)
{
	std::ifstream in(file_name);

	if (!in)
		opmip::throw_exception(opmip::errc::make_error_code(opmip::errc::no_such_file_or_directory),
		                       "Failed to open \"" + file_name + "\" node database file");

	size_t n = ndb.load(in);
	log_(0, "loaded ", n, " nodes from database");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	opmip::setup_crash_handler();

	try {
		opmip::app::cmdline_options opts;

		if (!opts.parse(argc, argv))
			return 1;

		size_t                  concurrency = boost::thread::hardware_concurrency();
		boost::asio::io_service ios(concurrency);
		opmip::pmip::node_db    ndb;
		opmip::pmip::lma        lma(ios, ndb, concurrency);

		log_(0, "chrono resolution ", opmip::chrono::get_resolution());

		load_node_database(opts.node_db, ndb);

		lma.start(opts.identifier.c_str());

		opmip::sys::interrupt_signal.connect([&lma]() {
			std::cout << "\r";
			log_(0, "stopping the LMA service");
			lma.stop();
		});

		opmip::sys::init_signals(opmip::sys::signal_mask::interrupt);

		boost::thread_group tg;
		for (size_t i = 1; i < concurrency; ++i)
			tg.create_thread(boost::bind(&boost::asio::io_service::run, &ios));

		ios.run();
		tg.join_all();

	} catch(opmip::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;

	} catch(std::exception& e) {
		std::cerr << "exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// EOF ////////////////////////////////////////////////////////////////////////
