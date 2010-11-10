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
#include <boost/throw_exception.hpp>
#include <iostream>
#include <exception>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

namespace po = boost::program_options;

///////////////////////////////////////////////////////////////////////////////
bool cmdline_options::parse(int argc, char** argv)
{
	po::options_description options("opmip-mag command line options");
	po::options_description config;
	po::variables_map vm;

	options.add_options()
		("help,h", "display command line options");

	config.add_options()
		("node-db",        po::value<std::string>()->default_value("node.db"),
		                   "node database")
		("id",             po::value<std::string>()->default_value(boost::asio::ip::host_name()),
		                   "MAG identifier on the node database")
		("link-local-ip",  po::value<std::string>()->default_value("fe80::1"),
		                   "link local IP address for all access links")
		("access-links,a", po::value<std::vector<std::string> >()->multitoken(),
		                   "list of access link interfaces");

	options.add(config);

	po::store(po::parse_command_line(argc, argv, options), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cerr << options << std::endl;
		return false;
	}

	identifier = vm["id"].as<std::string>();
	node_db = vm["node-db"].as<std::string>();

	link_local_ip = ip::address_v6::from_string(vm["link-local-ip"].as<std::string>());

	if (vm.count("access-links"))
		access_links = vm["access-links"].as<std::vector<std::string> >();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
