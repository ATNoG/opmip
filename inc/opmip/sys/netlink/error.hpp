//=============================================================================
// Brief   : Netlink Error
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

#ifndef OPMIP_SYS_NETLINK_ERROR__HPP_
#define OPMIP_SYS_NETLINK_ERROR__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/netlink/header.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace nl {

///////////////////////////////////////////////////////////////////////////////
struct error {
	enum m_type {
		m_begin = 2,
		m_error = m_begin,
		m_end
	};

	enum attr_type {
	};

	int    error;
	header hdr;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace nl */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_NETLINK_ERROR__HPP_ */
