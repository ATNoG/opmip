//=============================================================================
// Brief   : Netlink Frame
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

#ifndef OPMIP_SYS_NETLINK_FRAME__HPP_
#define OPMIP_SYS_NETLINK_FRAME__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace netlink {

///////////////////////////////////////////////////////////////////////////////
struct frame {
	enum flags {
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

	static frame*       cast(void* buffer, std::size_t length);
	static const frame* cast(const void* buffer, std::size_t length);


	void*  payload_data();
	size_t payload_length();


	uint32 length;   ///Length of message including header
	uint32 type;     ///Message content
	uint16 flags;    ///Additional flags
	uint32 sequence; ///Sequence number
	uint32 port_id;  ///Sending process port ID
};

inline frame* frame::cast(void* buffer, std::size_t length)
{
	frame* tmp = reinterpret_cast<frame*>(buffer);

	if ((length < sizeof(frame)) || (length < tmp->length))
		return nullptr;

	return tmp;
}

inline const frame* frame::cast(const void* buffer, std::size_t length)
{
	const frame* tmp = reinterpret_cast<const frame*>(buffer);

	if ((length < sizeof(frame)) || (length < tmp->length))
		return nullptr;

	return tmp;
}

inline void* frame::payload_data()
{
	return reinterpret_cast<uchar*>(this) + align_to<4>(sizeof(*this));
}

inline size_t frame::payload_length()
{
	return this->length - align_to<4>(sizeof(*this));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace netlink */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_BASIC_NETLINK_SOCKET__HPP_ */
