//=============================================================================
// Brief   : Exception Facilities
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

#ifndef OPMIP_EXCEPTION__HPP_
#define OPMIP_EXCEPTION__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/lexical_cast.hpp>  
#include <exception>
#include <typeinfo>
#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

using boost::error_info;
using boost::get_error_info;
using boost::throw_exception;
using boost::system::error_code;
using boost::system::error_category;

///////////////////////////////////////////////////////////////////////////////
struct error_source_ {
	error_source_(const char* func, const char* fl, uint ln)
		: function(func), file(fl), line(ln)
	{ }

	const char* function;
	const char* file;
	uint        line;
};

typedef error_info<struct tag_error_source, error_source_> error_source;

#ifndef NDEBUG
#	define OPMIP_THROW_EXCEPTION(e)                                        \
		opmip::throw_exception(e << opmip::error_source(                   \
		                                opmip::error_source_(__FUNCTION__, \
		                                                     __FILE__,     \
		                                                     __LINE__)     \
		                                ))
#else
#	define OPMIP_THROW_EXCEPTION(e) opmip::throw_exception(e)
#endif

///////////////////////////////////////////////////////////////////////////////
class exception : public std::runtime_error, public boost::exception {
public:
	exception(error_code ec)
		: std::runtime_error(""), _ec(ec)
	{ }
	exception(error_code ec, const char* what)
		: std::runtime_error(what), _ec(ec)
	{ }
	exception(error_code ec, const std::string & what)
		: std::runtime_error(what), _ec(ec)
	{ }
	exception(int ev, const error_category& ecat)
		: std::runtime_error(""), _ec(error_code(ev, ecat))
	{ }
	exception(int ev, const error_category& ecat, const char* what)
		: std::runtime_error(what), _ec(error_code(ev, ecat))
	{ }
	exception(int ev, const error_category& ecat, const std::string& what)
		: std::runtime_error(what), _ec(error_code(ev, ecat))
	{ }
	~exception() throw()
	{ }


	const error_code& code() const throw() { return _ec; }
	const char*       what() const throw();

private:
	static std::string get_error_information(const boost::exception& e);

private:
	error_code          _ec;
	mutable std::string _what;
};

inline const char* exception::what() const throw()
{
	using boost::lexical_cast;

	try {
		if (_what.empty()) {
			_what.append(  "Exception  : ")
			         .append(typeid(*this).name())
			     .append("\nError      : ")
			         .append(lexical_cast<std::string>(_ec.value()))
			         .append(", \"")
			         .append(_ec.message())
			         .append(1, '\"')
			     .append("\nwhat       : ")
			     .append(std::runtime_error::what())
			     .append(get_error_information(*this));
		}

	} catch (...) {
		return _what.empty() ? std::runtime_error::what() : _what.c_str();
	}

	return _what.c_str();
}

inline std::string exception::get_error_information(const boost::exception& e)
{
	using boost::lexical_cast;

	std::string tmp("\nError Info : {");
	const std::size_t prev = tmp.length();


	const error_source_* src = get_error_info<error_source>(e);
	if (src) {
		tmp.append("\n    error source : ")
		   .append(src->function)
		   .append(" at ")
		   .append(src->file)
		   .append(1, ':')
		   .append(lexical_cast<std::string>(src->line));
	}


	if (tmp.length() == prev)
		tmp.clear();
	else
		tmp.append("\n}");

	return tmp;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_EXCEPTION__HPP_ */
