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

#ifndef OPMIP_APP_OPTIONS__HPP_
#define OPMIP_APP_OPTIONS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ll/mac_address.hpp>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

///////////////////////////////////////////////////////////////////////////////
struct cmdline_options {
	std::string              identifier;
	std::string              node_db;
	ip::address_v6           link_local_ip;
	std::vector<std::string> access_links;


	bool parse(int argc, char** argv);
};

///////////////////////////////////////////////////////////////////////////////
bool parse_options(int argc, char** argv, cmdline_options& opts);

///////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_OPTIONS__HPP_ */
