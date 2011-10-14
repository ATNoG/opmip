//=============================================================================
// Brief   : <Brief Description>
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
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
//=============================================================================

#include <opmip/plugins/mag_driver.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace plugins {

///////////////////////////////////////////////////////////////////////////////
class test : public mag_driver {
public:
	void start(pmip::mag& mag, const std::vector<std::string>& options)
	{
		std::cout << "Hello from start" << std::endl;
	}
	
	void stop()
	{
		std::cout << "Hello from stop" << std::endl;
	}
};

///////////////////////////////////////////////////////////////////////////////
extern "C" mag_driver* opmip_mag_make_driver(boost::asio::io_service&)
{
	return new test;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace plugins */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
