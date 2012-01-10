//=============================================================================
// Brief   : ICMP Parser
// Authors : Bruno Santos <bsantos@av.it.pt>
//           Sérgio Figueiredo <sfigueiredo@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2012 Universidade de Aveiro
// Copyrigth (C) 2010-2012 Instituto de Telecomunicações - Pólo de Aveiro
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
bool icmp_mld_report_parser::parse(uchar* buffer, size_t length)
{
	using namespace opmip::ip;

	icmp::mld_report* mld = icmp::header::cast<icmp::mld_report>(buffer, length);
	if (!mld)
		return false;

	uchar* end = buffer + length;
	uint count = mld->count;
	icmp::mld_report::mcast_address* mca = mld->mcast_addresses;
	while (count--) {
		if (reinterpret_cast<uchar*>(mca->next()) > end)
			return false;

		const uint scount = mca->source_count;
		source_list slist;

		for (uint i = 0; i < scount; ++i)
			slist.push_back(address_v6(mca->sources[i]));

		switch (mca->type) {
		case 1: /* MODE_IS_INCLUDE */
			includes.push_back(mcast_address(address_v6(mca->group), slist));
			break;
		case 2: /* MODE_IS_EXCLUDE */
			excludes.push_back(mcast_address(address_v6(mca->group), slist));
			break;
		case 3: /* CHANGE_TO_INCLUDE */
			change_to_includes.push_back(mcast_address(address_v6(mca->group), slist));
			break;
		case 4: /* CHANGE_TO_EXCLUDE */
			change_to_excludes.push_back(mcast_address(address_v6(mca->group), slist));
			break;
		case 5: /* ALLOW_NEW_SOURCES */
			allow_new_sources.push_back(mcast_address(address_v6(mca->group), slist));
			break;
		case 6: /* BLOCK_OLD_SOURCES */
			block_old_sources.push_back(mcast_address(address_v6(mca->group), slist));
			break;
		}
		mca = mca->next();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
