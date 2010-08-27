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

	: _endpoint(rainfo.destination, 0), _length(0)
{
	std::fill(_buffer, _buffer + sizeof(_buffer), 0);

	ip::icmp::router_advertisement* ra = new(_buffer) ip::icmp::router_advertisement();
	size_t                          len = sizeof(ip::icmp::router_advertisement);

	ra->current_hop_limit(rainfo.hop_limit);
	ra->lifetime(rainfo.lifetime);

	//
	// Source link layer
	//
	ip::opt_source_link_layer* sll = new(_buffer + len) ip::opt_source_link_layer();

	*sll = rainfo.link_address;
	len += ip::option::size(sll);

	//
	// MTU
	//
	ip::opt_mtu* omtu = new(_buffer + len) ip::opt_mtu();

	omtu->set(rainfo.mtu);
	len += ip::option::size(omtu);

	//
	// Prefixes
	//
	for (prefix_list::const_iterator i = rainfo.prefix_list.begin(), e = rainfo.prefix_list.end(); i != e; ++i) {
		ip::opt_prefix_info* pref = new(_buffer + len) ip::opt_prefix_info();

		pref->L(true);
		pref->A(true);
		pref->valid_lifetime(7200);
		pref->prefered_lifetime(1800);
		pref->prefix(*i);

		len += ip::option::size(pref);
	}

	_length = len;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
