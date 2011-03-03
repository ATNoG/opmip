//=============================================================================
// Brief   : Chronometer
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

#ifndef OPMIP_CHRONO__HPP_
#define OPMIP_CHRONO__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
namespace posix {

struct timeval {
	::time_t      sec;
	::suseconds_t microsec;
};

namespace detail {

extern "C" int gettimeofday(timeval* tp, void* tzp = nullptr);

} /* namespace detail */

inline timeval gettimeofday()
{
	timeval tmp;

	detail::gettimeofday(&tmp);
	return tmp;
}

} /* namespace posix */

///////////////////////////////////////////////////////////////////////////////
class chrono {
public:
	chrono()
	{
		_start.sec = 0;
		_start.microsec = 0;
		_stop.sec = 0;
		_stop.microsec = 0;
	}

	void start()
	{
		_start = posix::gettimeofday();
	}

	void stop()
	{
		_stop = posix::gettimeofday();
	}

	double get()
	{
		double usec = _stop.microsec - _start.microsec;

		return usec / 1000000 + (_stop.sec - _start.sec);
	}

private:
	posix::timeval _start;
	posix::timeval _stop;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_CHRONO__HPP_ */
