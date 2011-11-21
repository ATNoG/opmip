//==============================================================================
// Brief   : Driver
// Authors : Bruno Santos <bsantos@av.it.pt>
// -----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2011 Universidade de Aveiro
// Copyrigth (C) 2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//==============================================================================

#ifndef OPMIP_PLUGINS_MAG__HPP_
#define OPMIP_PLUGINS_MAG__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/link/address_mac.hpp>
#include <boost/function.hpp>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace boost { namespace asio { class io_service; } }

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace plugins {

////////////////////////////////////////////////////////////////////////////////
class mag {
public:
	template<class CompletionHandler>
	void attach(const std::string& mn_id, const net::link::address_mac& mn_laddr,
	            uint poa_dev_id, const net::link::address_mac& poad_dev_laddr,
	            CompletionHandler h);

	template<class CompletionHandler>
	void detach(const std::string& mn_id, CompletionHandler h);

protected:
	typedef boost::function<void(const boost::system::error_code&)>
		completion_handler;

	virtual void attach_(const std::string& mn_id, const net::link::address_mac& mn_laddr,
	                     uint poa_dev_id, const net::link::address_mac& poad_dev_laddr,
	                     completion_handler& h) = 0;
	virtual void detach_(const std::string& mn_id, completion_handler& h) = 0;
};

template<class CompletionHandler>
void mag::attach(const std::string& mn_id, const net::link::address_mac& mn_laddr,
                 uint poa_dev_id, const net::link::address_mac& poa_dev_laddr,
                 CompletionHandler h)
{
	completion_handler ch(h);

	attach_(mn_id, mn_laddr, poa_dev_id, poa_dev_laddr, ch);
}

template<class CompletionHandler>
void mag::detach(const std::string& mn_id, CompletionHandler h)
{
	completion_handler ch(h);

	detach_(mn_id, ch);
}

////////////////////////////////////////////////////////////////////////////////
} /* namespace plugins */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PLUGINS_MAG__HPP__ */
