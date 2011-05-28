//=============================================================================
// Brief   : Chronometer
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_CHRONO__HPP_
#define OPMIP_CHRONO__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ptime.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
class chrono {
public:
	static ptime get_resolution()
	{
		return ptime::get_monotonic_resolution();
	}

public:
	chrono()
	{ }

	void start()
	{
		_start = ptime::get_monotonic();
	}

	void stop()
	{
		_stop = ptime::get_monotonic();
	}

	ptime get()
	{
		return _stop - _start;
	}

private:
	ptime _start;
	ptime _stop;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_CHRONO__HPP_ */
