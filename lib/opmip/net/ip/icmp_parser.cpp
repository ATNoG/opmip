//=============================================================================
// Brief   : ICMP Parser
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#include <opmip/net/ip/icmp_parser.hpp>

#include <opmip/ip/icmp.hpp>
#include <opmip/ip/icmp_options.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip {

///////////////////////////////////////////////////////////////////////////////
bool icmp_rs_parse(uchar* buffer, size_t length, link::address_mac& source_link_layer)
{
	using namespace opmip::ip;

	icmp::router_solicitation* rs = icmp::header::cast<icmp::router_solicitation>(buffer, length);
	if (!rs)
		return false;

	size_t pos = sizeof(icmp::router_solicitation);

	while (pos < length) {
		option*                opt = option::cast(buffer + pos, length - pos);
		opt_source_link_layer* sll = opt_source_link_layer::cast(opt);
		size_t                 len = option::size(opt);

		if (!len)
			break;

		if (sll) {
			source_link_layer = sll->to_mac();
			return true;
		}
		pos += len;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
