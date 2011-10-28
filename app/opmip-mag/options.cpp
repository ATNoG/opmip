//=============================================================================
// Brief   : Config Options
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

#include "options.hpp"
#include <boost/program_options.hpp>
#include <boost/asio/ip/host_name.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

namespace po = boost::program_options;

///////////////////////////////////////////////////////////////////////////////
bool cmdline_options::parse(int argc, char** argv, std::ostream& out)
{
	po::options_description options("opmip-mag command line options");
	po::positional_options_description po;

	po::variables_map vm;

	options.add_options()
		("help,h",         "display command line options")

		("id,i",           po::value<std::string>()->default_value(boost::asio::ip::host_name()),
		                   "router identifier on the node database")
		("database,d",     po::value<std::string>()->default_value("node.db"),
		                   "node database")
		("log,l",          "optional log file, defaults to the standard output")
		("tga,t",          po::value<bool>()->default_value(false),
                                   "set tunnel global address")
		("driver,e",       po::value<std::string>()->default_value("madwifi"),
		                   "event driver to be used, available: madwifi, 802.11, dummy")
		("link-local-ip",  po::value<std::string>()->default_value("fe80::1"),
		                   "link local IP address for all access links")
		("driver-options",  po::value<std::vector<std::string> >(), "driver specific options");

	po.add("driver-options", -1);

	po::store(po::command_line_parser(argc, argv).options(options).positional(po).run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		out << options << std::endl;
		return false;
	}

	identifier = vm["id"].as<std::string>();
	database = vm["database"].as<std::string>();
	driver = vm["driver"].as<std::string>();
	tunnel_global_address = vm["tga"].as<bool>();

	if (vm.count("driver-options"))
		driver_options = vm["driver-options"].as<std::vector<std::string> >();

	link_local_ip = ip::address_v6::from_string(vm["link-local-ip"].as<std::string>());

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
