//=============================================================================
// Brief   : ICMP Sender
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

#include <opmip/pmip/icmp_sender.hpp>
#include <opmip/ip/icmp.hpp>
#include <opmip/ip/icmp_options.hpp>
#include <boost/bind.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
icmp_ra_sender::icmp_ra_sender(const router_advertisement_info& rainfo)
	: _endpoint(rainfo.destination, 0)
{
	_ra_pkt(rainfo.hop_limit, rainfo.lifetime, 0, 0);

	//
	// Source link layer
	//
	ip::opt_source_link_layer* sll = _ra_pkt.add_option<ip::opt_source_link_layer>();

	*sll = rainfo.link_address;

	//
	// MTU
	//
	ip::opt_mtu* omtu = _ra_pkt.add_option<ip::opt_mtu>();

	omtu->set(rainfo.mtu);

	//
	// Prefixes
	//
	for (prefix_list::const_iterator i = rainfo.prefix_list.begin(), e = rainfo.prefix_list.end(); i != e; ++i) {
		ip::opt_prefix_info* pref = _ra_pkt.add_option<ip::opt_prefix_info>();

		pref->L(true);
		pref->A(true);
		pref->valid_lifetime(7200);
		pref->prefered_lifetime(1800);
		pref->prefix(*i);
	}

	_ipv6_pkt(rainfo.source, rainfo.destination, 255, _ra_pkt);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
