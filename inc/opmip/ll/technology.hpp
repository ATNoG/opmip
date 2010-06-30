//=============================================================================
// Brief   : Link Layer Technology
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

#ifndef OPMIP_LL_TYPE__HPP_
#define OPMIP_LL_TYPE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ll {

///////////////////////////////////////////////////////////////////////////////
enum technology {
	k_tech_unknown       = 0,
	k_tech_virtual       = 1,
	k_tech_ppp           = 2,
	k_tech_ieee802_3     = 3,
	k_tech_ieee802_11abg = 4,
	k_tech_ieee802_16e   = 5,
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ll */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LL_TYPE__HPP_ */
