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
#include <opmip/exception.hpp>
#include <opmip/pmip/mag.hpp>
#include <opmip/pmip/node_db.hpp>
#include <opmip/sys/signals.hpp>
#include "drivers/madwifi_driver.hpp"
#include "options.hpp"
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
static void interrupt(opmip::app::madwifi_driver& drv, opmip::pmip::mag& mag)
{
	std::cout << "\r";
	drv.stop();
	mag.stop();
}

///////////////////////////////////////////////////////////////////////////////
static void link_event(const boost::system::error_code& ec,
                       const opmip::app::madwifi_driver::event& ev,
                       opmip::pmip::mag& mag)
{
	if (ec)
		return;

	opmip::pmip::mag::attach_info ai(ev.if_index,
	                                 ev.if_address,
	                                 ev.mn_address);

	switch (ev.which) {
	case opmip::app::madwifi_driver_impl::attach:
		mag.mobile_node_attach(ai);
		break;

	case opmip::app::madwifi_driver_impl::detach:
		mag.mobile_node_detach(ai);
		break;

	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
static void load_node_database(const std::string& file_name, opmip::pmip::node_db& ndb)
{
	std::ifstream in(file_name.c_str());

	if (!in)
		opmip::throw_exception(opmip::errc::make_error_code(opmip::errc::no_such_file_or_directory),
		                       "Failed to open \"" + file_name + "\" node database file");

	size_t n = ndb.load(in);
	std::cout << "app: loaded " << n << " nodes from database\n";
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	try {
		opmip::app::cmdline_options opts;

		if (!opts.parse(argc, argv))
			return 1;

		size_t                       concurrency = boost::thread::hardware_concurrency();
		boost::asio::io_service      ios(concurrency);
		opmip::pmip::node_db         ndb;
		opmip::pmip::addrconf_server addrconf(ios);
		opmip::pmip::mag             mag(ios, ndb, addrconf, concurrency);
		opmip::app::madwifi_driver   drv(ios);

		load_node_database(opts.node_db, ndb);

		mag.start(opts.identifier.c_str(), opts.link_local_ip);
		drv.start(opts.access_links, boost::bind(link_event, _1, _2,
		                                         boost::ref(mag)));

		opmip::sys::interrupt_signal.connect(boost::bind(interrupt,
		                                                 boost::ref(drv),
		                                                 boost::ref(mag)));

		opmip::sys::init_signals(opmip::sys::signal_mask::interrupt);

		std::cout << std::setprecision(9)
		          << "chrono resolution: " << opmip::chrono::get_resolution() << std::endl;

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
