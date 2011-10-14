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

#ifndef OPMIP_PLUGINS_MAG_DRIVER__HPP_
#define OPMIP_PLUGINS_MAG_DRIVER__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip { class mag;        } }
namespace boost { namespace asio { class io_service; } }

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace plugins {

////////////////////////////////////////////////////////////////////////////////
class mag_driver {
public:
	virtual ~mag_driver() { }

	virtual void start(pmip::mag& mag, const std::vector<std::string>& options) = 0;
	virtual void stop() = 0;
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace plugins */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PLUGINS_MAG_DRIVER__HPP__ */
