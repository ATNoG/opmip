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
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

namespace po = boost::program_options;

///////////////////////////////////////////////////////////////////////////////
bool cmdline_options::parse(int argc, char** argv)
{
	po::options_description options("opmip-lma command line options");
	po::options_description config;
	po::variables_map vm;

	options.add_options()
		("help,h",        "display command line options");

	config.add_options()
		("id,i",           po::value<std::string>()->default_value(boost::asio::ip::host_name()),
		                   "router identifier on the node database")
		("database,d",     po::value<std::string>()->default_value("node.db"),
		                   "node database")
		("log,l",          "optional log file, defaults to the standard output");

	options.add(config);

	po::store(po::parse_command_line(argc, argv, options), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cerr << options << std::endl;
		return false;
	}

	identifier = vm["id"].as<std::string>();
	node_db = vm["database"].as<std::string>();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
