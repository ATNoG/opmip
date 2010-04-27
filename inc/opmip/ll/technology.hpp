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
	unknown       = 0, ///Unknown
	ethernet_10,       ///Ethernet 10 Mbits     - Ethernet
	ethernet_100,      ///Ethernet 100 Mbits    - Fast Ethernet
	ethernet_1000,     ///Ethernet 1000 Mbits   - Gigabit Ethernet
	ieee802_11,        ///IEEE 802.11           - WiFi
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ll */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LL_TYPE__HPP_ */
