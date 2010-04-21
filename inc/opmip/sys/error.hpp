//=============================================================================
// Brief   : Utilities for System Errors
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

#ifndef OPMIP_SYS_ERROR__HPP_
#define OPMIP_SYS_ERROR__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/exception.hpp>
#include <boost/system/system_error.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
void throw_on_error(error_code& ec)
{
	if (ec)
		throw_exception(boost::system::system_error(ec));
}

void throw_on_error(error_code& ec, const char* what)
{
	if (ec)
		throw_exception(boost::system::system_error(ec, what));
}

void throw_on_error(error_code& ec, const std::string& what)
{
	if (ec)
		throw_exception(boost::system::system_error(ec, what));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_ERROR__HPP_ */
