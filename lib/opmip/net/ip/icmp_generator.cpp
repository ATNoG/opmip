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

#include <opmip/net/ip/icmp_generator.hpp>
#include <opmip/ip/icmp.hpp>
#include <opmip/ip/icmp_options.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip {

///////////////////////////////////////////////////////////////////////////////
size_t icmp_mld_query_generator(const icmp_mld_query& imq, uchar* buffer, size_t length)
{
	using namespace opmip::ip;
	size_t len = 0, cnt = 0;

	if (length < sizeof(icmp::mld_query))
		return 0;

	icmp::mld_query* mld = new(buffer) icmp::mld_query;
	mld->max_resp_code = 0; // in ms
	mld->reserved = 0;
	mld->flags = 0;
	mld->qqic = 125;

	len = sizeof(icmp::mld_query);
	switch (imq.type) {
	case 0: /* General Query */
		mld->group.fill(0);
		mld->source_count = 0;
		break;

	case 1: /* Multicast Address Specífic Query */
		mld->group = imq.group.to_bytes();
		mld->source_count = 0;
		break;

	case 2: /* Multicast Address and Source Specific Query */
		cnt = imq.sources.size();
		len += sizeof(address_v6::bytes_type) * cnt;
		if (len > length)
			return 0;
		mld->group = imq.group.to_bytes();
		mld->source_count = htons(cnt);
		for (size_t i = 0; i < cnt; ++i)
			mld->sources[i] = imq.sources[i].to_bytes();
		break;

	default	:
		break;
	}

	return len;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
