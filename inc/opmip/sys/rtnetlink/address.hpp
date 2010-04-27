//=============================================================================
// Brief   : RT Netlink Address Message
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

#ifndef OPMIP_SYS_RTNETLINK_ADDRESS__HPP_
#define OPMIP_SYS_RTNETLINK_ADDRESS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace rtnetlink {

///////////////////////////////////////////////////////////////////////////////
class address {
public:
	enum m_type {
		m_begin = 20,

		m_new = m_begin,
		m_del,
		m_get,

		m_end
	};

	enum flags {
		temporary    = 0x01,
		no_dad       = 0x02,
		optimistic   = 0x04,
		dad_failed   = 0x08,
		home_address = 0x10,
		deprecated   = 0x20,
		tentaive     = 0x40,
		permanent    = 0x80,
	};

	enum attr_type {
		attr_begin   = 1,

		attr_address = attr_begin,
		attr_local,
		attr_label,
		attr_broadcast,
		attr_anycast,
		attr_cacheinfo,
		attr_multicast,

		attr_end
	};

	enum scope_type {
		scope_universe = 0,
		scope_site     = 200,
		scope_link     = 253,
		scope_host     = 254,
		scope_nowhere  = 255,
	};

	struct cacheinfo {
		uint32 prefered;
		uint32 valid;
		uint32 cstamp; ///Created timestamp, hundredths of seconds
		uint32 ustamp; ///Updated timestamp, hundredths of seconds
	};

public:
	address()
		: family(0), prefixlen(0), flags(0), scope(0), index(0)
	{ }


public:
	uint8  family;
	uint8  prefixlen;
	uint8  flags;
	uint8  scope;     ///Address scope
	uint32 index;     ///Link index
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace rtnetlink */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_RTNETLINK_ADDRESS__HPP_ */
