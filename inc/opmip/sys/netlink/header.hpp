//=============================================================================
// Brief   : Netlink Header
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

#ifndef OPMIP_SYS_NETLINK_HEADER__HPP_
#define OPMIP_SYS_NETLINK_HEADER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace nl {

///////////////////////////////////////////////////////////////////////////////
class header {
public:
	enum aflags {
		request   = 1, ///It is request message
		multipart = 2, ///Multipart message, terminated by NLMSG_DONE
		ack       = 4, ///Reply with ack, with zero or error code
		echo      = 8, ///Echo this request
	};

	enum get_flags {
		root   = 0x100, ///Specify tree root
		match  = 0x200, ///Return all matching
		atomic = 0x400, ///Atomic GET
		dump   = root | match
	};

	enum new_flags {
		replace   = 0x100, ///Override existing
		exclusive = 0x200, ///Do not touch, if it exists
		create    = 0x400, ///Create, if it does not exist
		append    = 0x800, ///Add to end of list
	};

public:
	static header*       cast(void* buffer, std::size_t length);
	static const header* cast(const void* buffer, std::size_t length);

public:
	header()
		: length(0), type(0), flags(0),
		  sequence(0), port_id(0)
	{ }

public:
	uint32 length;   ///Length of message including header
	uint16 type;     ///Message content
	uint16 flags;    ///Additional flags
	uint32 sequence; ///Sequence number
	uint32 port_id;  ///Sending process port ID
};

inline header* header::cast(void* buffer, std::size_t length)
{
	header* tmp = reinterpret_cast<header*>(buffer);

	if ((length < sizeof(header)) || (length < tmp->length))
		return nullptr;

	return tmp;
}

inline const header* header::cast(const void* buffer, std::size_t length)
{
	const header* tmp = reinterpret_cast<const header*>(buffer);

	if ((length < sizeof(header)) || (length < tmp->length))
		return nullptr;

	return tmp;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace nl */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_NETLINK_HEADER__HPP_ */
