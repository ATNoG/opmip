//===========================================================================================================
// Brief   : Time
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------------------------------------
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
//===========================================================================================================

#ifndef OPMIP_PTIME__HPP_
#define OPMIP_PTIME__HPP_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/assert.hpp>
#include <boost/io/ios_state.hpp>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip {

struct ptime;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum clock_kind {
	k_clock_realtime,
	k_clock_monotonic,
	k_clock_process_cpu_time_id,
	k_clock_thread_cpu_time_id,
	k_clock_monotonic_raw,
	k_clock_realtime_coarse,
	k_clock_monotonic_coarse,
};

extern "C" int clock_gettime(int, const ptime*) throw();
extern "C" int clock_getres(int, const ptime*) throw();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace detail */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ptime {
public:
	static ptime get_realtime();
	static ptime get_realtime_resolution();
	static ptime get_monotonic();
	static ptime get_monotonic_resolution();

public:
	ptime()
		: _sec(0), _nsec(0)
	{}
	ptime(std::time_t sec, ulong nsec)
		: _sec(sec), _nsec(nsec)
	{
		if (nsec > 1000000000) {
			++_sec;
			_nsec -= 1000000000;
		}
	}

	void seconds(std::time_t sec)
	{
		_sec = sec;
	}

	void nanoseconds(ulong nsec)
	{
		_nsec = nsec;

		if (nsec > 1000000000) {
			++_sec;
			_nsec -= 1000000000;
		}
	}

	std::time_t seconds() const     { return _sec; }
	ulong       nanoseconds() const { return _nsec; }

	friend ptime operator-(const ptime& lhs, const ptime& rhs);
	friend ptime operator+(const ptime& lhs, const ptime& rhs);

private:
	std::time_t _sec;
	ulong       _nsec;
};

inline ptime ptime::get_realtime()
{
	ptime tmp;

	int er = detail::clock_gettime(detail::k_clock_realtime, &tmp);
	BOOST_ASSERT(!er);
	OPMIP_NOT_USED(er);

	return tmp;
}

inline ptime ptime::get_realtime_resolution()
{
	ptime tmp;

	int er = detail::clock_getres(detail::k_clock_realtime, &tmp);
	BOOST_ASSERT(!er && tmp._sec >= 0);
	OPMIP_NOT_USED(er);

	return tmp;
}

inline ptime ptime::get_monotonic()
{
	ptime tmp;

	int er = detail::clock_gettime(detail::k_clock_monotonic, &tmp);
	BOOST_ASSERT(!er);
	OPMIP_NOT_USED(er);

	return tmp;
}

inline ptime ptime::get_monotonic_resolution()
{
	ptime tmp;

	int er = detail::clock_getres(detail::k_clock_monotonic, &tmp);
	BOOST_ASSERT(!er && tmp._sec >= 0);
	OPMIP_NOT_USED(er);

	return tmp;
}

inline ptime operator+(const ptime& lhs, const ptime& rhs)
{
	ptime tm;

	tm._sec = lhs._sec + rhs._sec;
	tm._nsec = lhs._nsec + rhs._nsec;

	if (tm._nsec >= 1000000000) {
		tm._sec += 1;
		tm._nsec -= 1000000000;
	}

	return tm;
}

inline ptime operator-(const ptime& lhs, const ptime& rhs)
{
	ptime tm;

	tm._sec = lhs._sec - rhs._sec;
	tm._nsec = lhs._nsec - rhs._nsec;

	if (lhs._nsec < rhs._nsec) {
		tm._sec -= 1;
		tm._nsec += 1000000000;
	}

	return tm;
}

inline std::ostream& operator<<(std::ostream& os, const ptime& tm)
{
	boost::io::ios_fill_saver ios(os);

	return os << tm.seconds() << "." << std::setfill('0') << std::setw(9) << tm.nanoseconds();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PTIME__HPP_ */
