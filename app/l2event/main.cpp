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
#include <opmip/linux/nl80211.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <ctime>

///////////////////////////////////////////////////////////////////////////////
static void interrupt(opmip::linux::nl80211& drv)
{
	drv.close();
}

///////////////////////////////////////////////////////////////////////////////
void link_event(const boost::system::error_code& ec, const opmip::linux::nl80211::event& ev)
{
	switch (ev.which) {
	case opmip::linux::nl80211::event::new_sta:      std::cout << "new_sta - "; break;
	case opmip::linux::nl80211::event::associate:    std::cout << "associate - "; break;
	case opmip::linux::nl80211::event::disassociate: std::cout << "disassociate - "; break;
	default: break;
	}

	std::cout << "event = {\n"
				 "    .phy = " << ev.phy_id << "\n"
				 "    .if_index = " << ev.if_index << "\n"
				 "    .generation = " << ev.generation << "\n"
				 "    .mac = " << ev.mac << "\n"
				 "    .frame = {\n"
				 "        .da       = " << ev.dst_addr << "\n"
				 "        .sa       = " << ev.src_addr << "\n"
				 "        .bssid    = " << ev.bssid << "\n"
				 "     }\n"
				 "}\n";

	std::time_t tm = std::time(0);
	std::cout << "Time: " << std::ctime(&tm) << std::endl;
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
		boost::asio::io_service ios;
		opmip::linux::nl80211   nl80211(ios);

		nl80211.open(link_event);

		opmip::sys::interrupt_signal.connect(boost::bind(interrupt, boost::ref(nl80211)));
		opmip::sys::init_signals(opmip::sys::signal_mask::interrupt);

		ios.run();

	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// EOF ////////////////////////////////////////////////////////////////////////
