//=============================================================================
// Brief   : Generic Netlink
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

#ifndef OPMIP_LINUX_GENETLINK__HPP_
#define OPMIP_LINUX_GENETLINK__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/linux/netlink.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux {

///////////////////////////////////////////////////////////////////////////////
class genetlink : public netlink_<16> {
public:
	class message;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace linux */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LINUX_GENETLINK__HPP_ */
