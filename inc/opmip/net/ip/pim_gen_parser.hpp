//=============================================================================
// Brief   : PIM Generator and Parser
// Authors : Bruno Santos <bsantos@av.it.pt>
// 		   : Sérgio Figueiredo <sfigueiredo@av.it.pt>
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

#ifndef OPMIP_NET_IP_PIM_GEN_PARSER__HPP_
#define OPMIP_NET_IP_PIM_GEN_PARSER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/ip/address.hpp>
#include <opmip/net/ip/pim.hpp>
#include <boost/optional.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {	namespace net {	namespace ip {

///////////////////////////////////////////////////////////////////////////////

/*
uint16 inchksum(const void *data, uint32 length);

uint16 ipv6_checksum(uint8 protocol, const address_v6 &src, const address_v6 &dst, const void *data, uint16 len);
*/

struct hello_msg {
	boost::optional<uint16> holdtime;
	boost::optional<uint32> dr_priority;
	boost::optional<uint32> generation_id;
	std::vector<address_v6> maddr_list;

	size_t gen(uchar* buffer, size_t length) const;
	bool parse(uchar* buffer, size_t length);
};
/*
struct register_msg {
	uint32 data;

	size_t gen(uchar* buffer, size_t length) const;
};

struct register_stop_msg {
	enc_group grp;
	enc_unicast src;

	size_t gen(uchar* buffer, size_t length) const;
};
*/
struct join_prune_msg {
	address_v6 uplink;
	struct mcast_group {
		address_v6 group;
		std::vector<address_v6> joins;
		std::vector<address_v6> prunes;
	};
	std::vector<mcast_group> mcast_groups;
	size_t gen(uchar* buffer, size_t length) const;
	bool parse(uchar* buffer, size_t length);
};

/*
size_t register_gen(const pim_register_ imq, uchar* buffer, size_t length);

size_t register_stop_gen(const pim::register_stop& imq, uchar* buffer, size_t length);

size_t join_prune_gen(const pim::join_prune& imq, uchar* buffer, size_t length);
*/



//TODO PIM-Registers and Register-Stops are unnecessary at this stage,
//	First, we're aiming at a solution where DR sends the multicast traffic
//	Second, the kernel already implement multicast forwarding functionality, such as encapsulation through virtual interface


///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_NET_IP_PIM_GEN_PARSER__HPP_ */
