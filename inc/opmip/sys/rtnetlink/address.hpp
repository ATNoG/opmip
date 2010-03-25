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

private:
	uint8  _family;
	uint8  _prefixlen;
	uint8  _flags;
	uint8  _scope;     ///Address scope
	uint32 _index;     ///Link index
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace rtnetlink */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_RTNETLINK_ADDRESS__HPP_ */
