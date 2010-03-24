//=============================================================================
// Brief   : RT Netlink Messages
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

#ifndef OPMIP_SYS_RTNETLINK_MESSAGES__HPP_
#define OPMIP_SYS_RTNETLINK_MESSAGES__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace rtnetlink {

///////////////////////////////////////////////////////////////////////////////
enum msg {
	msg_new_link          = 16,
	msg_del_link,
	msg_get_link,
	msg_set_link,

	msg_new_address       = 20,
	msg_del_address,
	msg_get_address,
	msg_set_address,

	msg_new_route         = 24,
	msg_del_route,
	msg_get_route,
	msg_set_route,
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace rtnetlink */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_RTNETLINK_MESSAGES__HPP_ */
