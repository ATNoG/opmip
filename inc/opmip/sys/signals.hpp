//=============================================================================
// Brief   : Signal Handling
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

#ifndef OPMIP_SYS_SIGNALS__HPP_
#define OPMIP_SYS_SIGNALS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/signals2/signal.hpp>

///////////////////////////////////////////////////////////////////////////////
extern template class boost::signals2::signal<void()>;

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
extern boost::signals2::signal<void()> interrupt_signal;

///////////////////////////////////////////////////////////////////////////////
struct signal_mask {
	enum {
		interrupt = 0x1,
	};
};

void init_signals(uint mk);

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_SIGNALS__HPP_ */
