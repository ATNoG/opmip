//=============================================================================
// Brief   : L2 Event Logger (for measurement purposes)
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
#include <opmip/sys/signals.hpp>
#include "drivers/madwifi_driver.hpp"
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <ctime>

///////////////////////////////////////////////////////////////////////////////
static void interrupt(opmip::app::madwifi_driver& drv)
{
	drv.stop();
}

///////////////////////////////////////////////////////////////////////////////
static void link_event(const boost::system::error_code& ec,
                       const opmip::app::madwifi_driver::event& ev)
{
	if (ec)
		return;

	switch (ev.which) {
	case opmip::app::madwifi_driver_impl::attach:
		std::cout << "attach " << std::time(0) << std::endl;
		break;

	case opmip::app::madwifi_driver_impl::detach:
		std::cout << "detach " << std::time(0) << std::endl;
		break;

	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	try {
		std::vector<std::string> access_links;
		{
			namespace po = boost::program_options;

			po::options_description options("l2event command line options");
			po::variables_map       vm;

			options.add_options()
				("access-links,a", po::value<std::vector<std::string> >()->multitoken(),
				 "list of access link interfaces to monitor");

			po::store(po::parse_command_line(argc, argv, options), vm);
			po::notify(vm);

			access_links = vm["access-links"].as<std::vector<std::string> >();
		}
		boost::asio::io_service    ios;
		opmip::app::madwifi_driver drv(ios);

		drv.start(access_links, boost::bind(link_event, _1, _2));

		opmip::sys::interrupt_signal.connect(boost::bind(interrupt, boost::ref(drv)));
		opmip::sys::init_signals(opmip::sys::signal_mask::interrupt);

		ios.run();

	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// EOF ////////////////////////////////////////////////////////////////////////
