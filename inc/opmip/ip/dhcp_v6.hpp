//=============================================================================
// Brief   : DHCPv6
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

#ifndef OPMIP_IP_DHCPV6__HPP_
#define OPMIP_IP_DHCPV6__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
struct dhcp_v6 {
	static const uint solicitation_max_delay   = 1;   /// Max delay of first Solicit
	static const uint solicitation_timeout     = 1;   /// Initial Solicit timeout
	static const uint solicitation_max_timeout = 120; /// Max Solicit timeout value
	static const uint request_timeout          = 1;   /// Initial Request timeout
	static const uint request_max_timeout      = 30;  /// Max Request timeout value
	static const uint request_max_retry        = 10;  /// Max Request retry attempts
	static const uint confirm_max_delay        = 1;   /// Max delay of first Confirm
	static const uint confirm_timeout          = 1;   /// Initial Confirm timeout
	static const uint confirm_max_timeout      = 4;   /// Max Confirm timeout
	static const uint confirm_max_duration     = 10;  /// Max Confirm duration
	static const uint renew_timeout            = 10;  /// Initial Renew timeout
	static const uint renew_max_timeout        = 600; /// Max Renew timeout value
	static const uint rebind_timeout           = 10;  /// Initial Rebind timeout
	static const uint rebind_max_timeout       = 600; /// Max Rebind timeout value
	static const uint information_max_delay    = 1;   /// Max delay of first Information-request
	static const uint information_timeout      = 1;   /// Initial Information-request timeout
	static const uint information_max_timeout  = 120; /// Max Information-request timeout value
	static const uint release_timeout          = 1;   /// Initial Release timeout
	static const uint release_max_retry        = 5;   /// MAX Release attempts
	static const uint decline_timeout          = 1;   /// Initial Decline timeout
	static const uint decline_max_retry        = 5;   /// Max Decline attempts
	static const uint reconf_timeout           = 2;   /// Initial Reconfigure timeout
	static const uint reconf_max_retry         = 8;   /// Max Reconfigure attempts
	static const uint hop_count_limit          = 32;  /// Max hop count in a Relay-forward message

	static const ushort client_port = 546;
	static const ushort server_port = 547;

	static const uint8 all_servers[16] = {
		0xff, 0x05,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x01, 0x03
	};

	static const uint8 all_servers_and_relay_agents[16] = {
		0xff, 0x02,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x01, 0x02
	};

	enum opcode {
		solicitation  = 1,
		advertise     = 2,
		request       = 3,
		confirm       = 4,
		renew         = 5,
		rebind        = 6,
		reply         = 7,
		release       = 8,
		decline       = 9,
		reconfigure   = 10,
		info_request  = 11,
		relay_forward = 12,
		relay_reply   = 13,
	};

	enum status {
		status_success       = 0,
		status_failure       = 1, ///Failure, reason unspecified
		status_no_addresses  = 2, ///Server has no addresses available to assign to the IA(s).
		status_no_binding    = 3, ///Client record (binding) unavailable.
		status_not_on_link   = 4, ///The prefix for the address is not appropriate for the link to which the client is attached.
		status_use_multicast = 5, ///Client must use the All_DHCP_Relay_Agents_and_Servers address
	};

	struct header;
};

struct dhcp_v6::header {
	uint8  opcode;
	uint8  tid[3];

	void transaction_id(uint val)
	{
		tid[0] = uint8(val);
		tid[1] = uint8(val >> 8);
		tid[2] = uint8(val >> 16);
		BOOST_ASSERT(!(val >> 24));
	}

	uint transaction_id()
	{
		return tid[0] | (uint(tid[1]) >> 8)
		              | (uint(tid[2]) >> 16);
	}
};

OPMIP_STATIC_ASSERT(sizeof(dhcp_v6::header) == 4);

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_DHCPV6__HPP_ */
