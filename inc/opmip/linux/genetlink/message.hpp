//=============================================================================
// Brief   : Generic Netlink Message
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

#ifndef OPMIP_LINUX_GENETLINK_MESSAGE__HPP_
#define OPMIP_LINUX_GENETLINK_MESSAGE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/linux/netlink/message.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux_ {

///////////////////////////////////////////////////////////////////////////////
class genetlink::message : public netlink::message {
	static const size_t headers_size = align_to_<4, sizeof(netlink::message::header)>::value
	                                   + align_to_<4, sizeof(header)>::value;

	struct header {
		static header* cast(netlink::message::header* hdr)
		{
			if (hdr->length < (align_to_<4, sizeof(netlink::message::header)>::value + sizeof(header)))
				return nullptr;

			return offset_cast<header*>(hdr, align_to_<4, sizeof(netlink::message::header)>::value);
		}

		uint8  cmd;
		uint8  version;
		uint16 reserved;
	};

	OPMIP_UNDEFINED_BOOL;

public:
	enum m_cmd {
		cmd_new_family = 1,
		cmd_del_family,
		cmd_get_family,
		cmd_new_ops,
		cmd_del_ops,
		cmd_get_ops,
		cmd_new_mcast,
		cmd_del_mcast,
	};

	enum attr {
		attr_family_id = 1,	//uint16
		attr_family_name,	//string
		attr_version,		//uint32
		attr_hdrsize,		//uint32
		attr_maxattr,		//uint32
		attr_ops,			//nested list of attr_op
		attr_mcast_groups,	//nested list of attr_mcast
	};

	enum attr_op {
		attr_op_id = 1,		//uint32
		attr_op_flags,		//uint32
	};

	enum attr_mcast {
		attr_mcast_name = 1,	//string
		attr_mcast_id,			//uint32
	};

public:
	message()
		: netlink::message()
	{
		uchar* p = alloc(sizeof(header));

		type(netlink::message::min_type);
		_hdr = reinterpret_cast<header*>(p);
	}

	explicit message(netlink::message::header* hdr)
		: netlink::message(hdr, align_to<4>(sizeof(netlink::message::header)) + align_to<4>(sizeof(header))),
		  _hdr(header::cast(hdr))
	{ }

	void command(uint8 cmd) { _hdr->cmd = cmd; }
	void version(uint8 ver) { _hdr->version = ver; }

	uint8 command() const { return _hdr->cmd; }
	uint8 version() const { return _hdr->version; }

	operator undefined_bool()
	{
		return !_hdr ? OPMIP_UNDEFINED_BOOL_FALSE : OPMIP_UNDEFINED_BOOL_TRUE;
	}

	bool operator!()
	{
		return !_hdr ? true : false;
	}

private:
	header* _hdr;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace linux */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LINUX_GENETLINK_MESSAGE__HPP_ */
