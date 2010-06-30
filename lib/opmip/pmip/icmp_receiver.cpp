//=============================================================================
// Brief   : ICMP Receiver
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

#include <opmip/pmip/icmp_receiver.hpp>
#include <opmip/ip/icmp.hpp>
#include <opmip/ip/icmp_options.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
bool icmp_rs_receiver::parse(size_t rbytes, ll::mac_address& source_link_layer)
{
	ip::icmp::router_solicitation* rs = ip::icmp::header::cast<ip::icmp::router_solicitation>(_buffer, rbytes);
	if (!rs)
		return false;

	size_t pos = sizeof(ip::icmp::router_solicitation);

	while (pos < rbytes) {
		ip::option*                opt = ip::option::cast(_buffer + pos, rbytes - pos);
		ip::opt_source_link_layer* sll = ip::opt_source_link_layer::cast(opt);
		size_t                     len = ip::option::size(opt);

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
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
