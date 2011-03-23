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
#include <boost/assert.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
namespace posix {

struct timeval {
	timeval()
		: sec(0), usec(0)
	{}

	::time_t      sec;
	::suseconds_t usec;
};

struct timespec {
	timespec()
		: sec(0), nsec(0)
	{}

	::time_t sec;
	long     nsec;
};

enum clock_kind {
	k_clock_realtime,
	k_clock_monotonic,
	k_clock_process_cpu_time_id,
	k_clock_thread_cpu_time_id,
	k_clock_monotonic_raw,
	k_clock_realtime_coarse,
	k_clock_monotonic_coarse,
};

namespace detail {

extern "C" int gettimeofday(timeval*, void* = nullptr) throw();
extern "C" int clock_gettime(int, const timespec*) throw();
extern "C" int clock_getres(int, const timespec*) throw();

} /* namespace detail */

inline timeval gettimeofday()
{
	timeval tmp;

	detail::gettimeofday(&tmp);
	return tmp;
}

inline timespec clock_gettime(clock_kind kind)
{
	timespec tmp;

	int er = detail::clock_gettime(kind, &tmp);
	BOOST_ASSERT(!er);
	OPMIP_NOT_USED(er);

	return tmp;
}

inline timespec clock_getres(clock_kind kind)
{
	timespec tmp;

	int er = detail::clock_getres(kind, &tmp);
	BOOST_ASSERT(!er && tmp.sec >= 0);
	OPMIP_NOT_USED(er);

	return tmp;
}

} /* namespace posix */

///////////////////////////////////////////////////////////////////////////////
class chrono {
	//
	// FIXME: this should be configurable has: default, raw or coarse
	// NOTE: CLOK_MONOTONIC_RAW seems to not give a resolution
	//
	static const posix::clock_kind k_clock_kind = posix::k_clock_monotonic;

public:
	static double get_resolution()
	{
		posix::timespec tmp = posix::clock_getres(k_clock_kind);

		return tmp.sec + tmp.nsec / 1.0e9;
	}

public:
	chrono()
	{ }

	void start()
	{
		_start = posix::clock_gettime(k_clock_kind);
	}

	void stop()
	{
		_stop = posix::clock_gettime(k_clock_kind);
	}

	double get()
	{
		return (_stop.sec - _start.sec)
			   + (_stop.nsec - _start.nsec) / 1.0e9;
	}

private:
	posix::timespec _start;
	posix::timespec _stop;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_CHRONO__HPP_ */
