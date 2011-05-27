//=============================================================================
// Brief   : Debug Helpers
// Authors : Bruno Santos <bsantos@av.it.pt>
//------------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2009-2011 Universidade Aveiro
// Copyright (C) 2009-2011 Instituto de Telecomunicações - Pólo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//==============================================================================

#ifndef OPMIP_DEBUG__HPP_
#define OPMIP_DEBUG__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
struct crash_ctx {
	crash_ctx()
		: code(0), address(0), function(0), file(0),
		  line(0), expression(0), context(0)
	{ }

	sint        code;
	void*       address;
	const char* function;
	const char* file;
	uint        line;
	const char* expression;
	void*       context;
};

void setup_crash_handler();

OPMIP_ATTR_NO_RETURN void crash(const crash_ctx& ctx);

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_DEBUG__HPP_ */
