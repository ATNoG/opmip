//=============================================================================
// Brief   : DHCPv6
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

#ifndef OPMIP_NET_IP_DHCPV6__HPP_
#define OPMIP_NET_IP_DHCPV6__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/ip/address.hpp>
#include <opmip/net/link/address_mac.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip { namespace dhcp_v6 {

///////////////////////////////////////////////////////////////////////////////
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

static const ip::address_v6::bytes_type all_servers = {{
	0xff, 0x05,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x01,
	0x00, 0x03
}};

static const ip::address_v6::bytes_type all_servers_and_relay_agents = {{
	0xff, 0x02,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x01,
	0x00, 0x02
}};

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

enum option {
	client_id    = 1, //DUID
	server_id    = 2, //DUID
	ia_na        = 3,
	ia_ta        = 4,
	ia_addr      = 5,
	req_opts     = 6,
	preference   = 7,
	elapsed_time = 8,
	status_code  = 13,
	rapid_commit = 14,
};

enum status {
	status_success       = 0,
	status_failure       = 1, ///Failure, reason unspecified
	status_no_addresses  = 2, ///Server has no addresses available to assign to the IA(s).
	status_no_binding    = 3, ///Client record (binding) unavailable.
	status_not_on_link   = 4, ///The prefix for the address is not appropriate for the link to which the client is attached.
	status_use_multicast = 5, ///Client must use the All_DHCP_Relay_Agents_and_Servers address
};

typedef std::pair<uchar*, uchar*>             buffer_type;
typedef std::pair<const uchar*, const uchar*> const_buffer_type;

inline size_t buffer_size(const buffer_type& buff)
{
	return buff.second - buff.first;
}

inline size_t buffer_size(const const_buffer_type& buff)
{
	return buff.second - buff.first;
}

// Generators /////////////////////////////////////////////////////////////////
bool gen_header(buffer_type& buff, opcode op, uint tid);
bool gen_option_begin(buffer_type& buff, option opt, buffer_type& state);
bool gen_option_end(buffer_type& buff, buffer_type state);
bool gen_option_server_id(buffer_type& buff, const link::address_mac& link_addr);
bool gen_option_client_id(buffer_type& buff, const buffer_type& cid);
bool gen_option_ia(buffer_type& buff, uint32 id, uint32 t1, uint32 t2,
                   buffer_type* state = 0);
bool gen_option_addr(buffer_type& buff, const address_v6& addr, uint32 pref_lifetime,
                     uint32 val_lifetime, buffer_type state);
bool gen_option_status(buffer_type& buff, status st, const char* msg);
bool gen_message(buffer_type& buff, opcode op, uint32 tid,
                 const link::address_mac& link_addr,
                 const buffer_type& client_id);

// Parsers ////////////////////////////////////////////////////////////////////
bool parse_header(buffer_type& buff, opcode& op, uint& tid);
bool parse_option(buffer_type& buff, option& opt, buffer_type& data);
bool parse_option_duid(buffer_type buff, link::address_mac& link_addr);
bool parse_option_ia(buffer_type& buff, uint32& id, uint32& t1, uint32& t2);

///////////////////////////////////////////////////////////////////////////////
} /* namespace dhcp_v6 */ } /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_NET_IP_DHCPV6__HPP_ */
