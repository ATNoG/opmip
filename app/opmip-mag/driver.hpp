//==============================================================================
// Brief   : Driver
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

#ifndef OPMIP_APP_DRIVER__HPP_
#define OPMIP_APP_DRIVER__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/pmip/mag.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
class driver {
public:
	virtual ~driver() { }

	virtual void start(pmip::mag& mag, const std::vector<std::string>& options) = 0;
	virtual void stop() = 0;
};

typedef boost::shared_ptr<driver> driver_ptr;

////////////////////////////////////////////////////////////////////////////////
driver_ptr make_driver(boost::asio::io_service& ios, const std::string& name);

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_DRIVER__HPP_ */
