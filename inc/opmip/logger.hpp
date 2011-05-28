//=============================================================================
// Brief   : Logging Facilities
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

#ifndef OPMIP_LOGGER__HPP_
#define OPMIP_LOGGER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/utility.hpp>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
class logger : boost::noncopyable {
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

public:
	logger(char const* const name, std::ostream& sink)
		: _name(name), _sink(sink), _level(~0)
	{ }

	void level(uint n) { _level = n; }

	uint          level() const { return _level; }
	std::ostream& sink() const  { return _sink; }

	template<class ...T>
	void operator()(uint level, const T& ...args)
	{
		if ((level > _level) || !_sink)
			return;

		_sink << _name << ": ";
		print_args(_sink, args...);
		std::flush(_sink);
	}

private:
	char const* const _name;
	std::ostream&     _sink;
	uint              _level;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LOGGER__HPP_ */
