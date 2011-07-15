//===========================================================================================================
// Brief   : Logging Facilities
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------------------------------------
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
//===========================================================================================================

#ifndef OPMIP_LOGGER__HPP_
#define OPMIP_LOGGER__HPP_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/utility.hpp>
#include <ostream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class logger : boost::noncopyable {
#ifndef BOOST_NO_VARIADIC_TEMPLATES
	template<class T, class ...Args>
	static void print_args(std::ostream& os, const T& arg, const Args& ...args)
	{
		os << arg;
		print_args(os, args...);
	}

	template<class T>
	static void print_args(std::ostream& os, const T& arg)
	{
		os << arg << std::endl;
	}
#endif

public:
	logger(char const* const name, std::ostream& sink)
		: _name(name), _sink(sink), _level(~0)
	{ }

	void level(uint n) { _level = n; }

	uint          level() const { return _level; }
	std::ostream& sink() const  { return _sink; }

#ifndef BOOST_NO_VARIADIC_TEMPLATES
	template<class ...T>
	void operator()(uint level, const T& ...args)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": ";
		print_args(_sink, args...);
		std::flush(_sink);
	}
#else
	template<class T1>
	void operator()(uint level, const T1& arg1)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2>
	void operator()(uint level, const T1& arg1, const T2& arg2)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5, class T6>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9, const T10& arg10)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10 << std::endl;
		std::flush(_sink);
	}

	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11>
	void operator()(uint level, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9, const T10& arg10, const T11& arg11)
	{
		if (level > _level)
			return;

		_sink << _name << ": " << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 << arg10 << arg11 << std::endl;
		std::flush(_sink);
	}
#endif

private:
	char const* const _name;
	std::ostream&     _sink;
	uint              _level;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LOGGER__HPP_ */
