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
#include <opmip/debug.hpp>
#include <opmip/logger.hpp>
#include <opmip/exception.hpp>
#include <opmip/pmip/mag.hpp>
#include <opmip/pmip/node_db.hpp>
#include "driver.hpp"
#include "options.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
static opmip::logger log_("opmip-mag", std::cout);

///////////////////////////////////////////////////////////////////////////////
static void load_node_database(const std::string& file_name, opmip::pmip::node_db& ndb)
{
	std::ifstream in(file_name.c_str());

	if (!in)
		opmip::throw_exception(opmip::errc::make_error_code(opmip::errc::no_such_file_or_directory),
		                       "Failed to open \"" + file_name + "\" node database file");

	std::pair<size_t, size_t> n = ndb.load(in);
	log_(0, "loaded ", n.first, " router nodes and ", n.second, " mobile nodes from database");
}

static void signal_handler(const boost::system::error_code& error, opmip::app::driver_ptr& drv, opmip::pmip::mag& mag)
{
	std::cout << "\r";
	log_(0, "stopping driver");
	drv->stop();
	log_(0, "stopping the MAG service");
	mag.stop();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	opmip::setup_crash_handler();

	try {
		opmip::app::cmdline_options opts;

		if (!opts.parse(argc, argv, std::cerr))
			return 1;

		size_t                       concurrency = boost::thread::hardware_concurrency();
		boost::asio::io_service      ios(concurrency);
		boost::asio::signal_set      sigs(ios, SIGINT, SIGTERM);
		opmip::pmip::node_db         ndb;
		opmip::pmip::addrconf_server addrconf(ios);
		opmip::pmip::mag             mag(ios, ndb, addrconf, concurrency);
		opmip::app::driver_ptr       drv;

		load_node_database(opts.database, ndb);

		log_(0, "chrono resolution ", opmip::chrono::get_resolution());

		mag.start(opts.identifier.c_str(), opts.link_local_ip, opts.tunnel_global_address);

		drv = opmip::app::make_driver(ios, mag, opts.driver);
		if (!drv) {
			log_(0, "driver not found: ", opts.driver);
			return 1;
		}
		drv->start(mag, opts.driver_options);

		sigs.async_wait(boost::bind(signal_handler, _1, drv, boost::ref(mag)));

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
