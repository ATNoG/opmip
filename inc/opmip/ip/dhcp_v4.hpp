//=============================================================================
// Brief   : DHCPv4
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

#ifndef OPMIP_IP_DHCPV4__HPP_
#define OPMIP_IP_DHCPV4__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
struct dhcp_v4 {
	struct header;
	enum flags;
};

enum dhcp_v4::flags {
	broadcast = 0x01,
};

union dhcp_v4::header {
	uint8  opcode;
	uint8  hw_type;
	uint8  hw_length;
	uint8  hops;
	uint32 xid;
	uint16 elapsed_secs;
	uint16 flags;
	uint32 client_addr;
	uint32 your_addr;
	uint32 server_addr;
	uint32 relay_addr;
	uint8  hw_addr[16];
	char   server_hostname[64];
	char   boot_file[128];
};

OPMIP_STATIC_ASSERT(sizeof(dhcp_v4::header) == 236);

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_DHCPV4__HPP_ */
