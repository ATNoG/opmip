//=============================================================================
// Brief   : ICMP Generator
// Authors : Bruno Santos <bsantos@av.it.pt>
//           Sérgio Figueiredo <sfigueiredo@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2011-2012 Universidade de Aveiro
// Copyrigth (C) 2011-2012 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_PMIP_NET_IP_ICMP_GENERATOR__HPP_
#define OPMIP_PMIP_NET_IP_ICMP_GENERATOR__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/ip/address.hpp>
#include <opmip/net/link/address_mac.hpp>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip {

///////////////////////////////////////////////////////////////////////////////
struct icmp_mld_query {
	address_v6              group;
	std::vector<address_v6> sources;
	uint8                   type;
};

size_t icmp_mld_query_generator(const icmp_mld_query& imq, uchar* buffer, size_t length);

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_NET_IP_ICMP_GENERATOR__HPP_ */
