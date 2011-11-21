//==============================================================================
// Brief   : MadWifi Driver
// Authors : Bruno Santos <bsantos@av.it.pt>
// -----------------------------------------------------------------------------
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
//==============================================================================

#ifndef OPMIP_APP_DRIVER_MADWIFI_DRIVER__HPP_
#define OPMIP_APP_DRIVER_MADWIFI_DRIVER__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/pmip/mag.hpp>
#include "madwifi_driver_impl.hpp"
#include "../driver.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
class madwifi_driver : public plugins::mag_driver {
public:
	typedef madwifi_driver_impl::address_mac   address_mac;
	typedef madwifi_driver_impl::event_type    event_type;
	typedef madwifi_driver_impl::event         event;
	typedef madwifi_driver_impl::event_handler event_handler;
	typedef madwifi_driver_impl::if_list       if_list;

public:
	madwifi_driver(boost::asio::io_service& ios, pmip::mag& mag);
	~madwifi_driver();

	virtual void start(const std::vector<std::string>& options);
	virtual void stop();

private:
	opmip::app::madwifi_driver_impl _impl;
	pmip::mag&                      _mag;
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_DRIVER_MADWIFI_DRIVER__HPP_ */
