//===========================================================================================================
// Brief   : Driver
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

#include "driver.hpp"
#include "drivers/madwifi_driver.hpp"
#include "drivers/dummy.hpp"
#include <boost/make_shared.hpp>
#include <dlfcn.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class delete_dl_driver {
public:
	delete_dl_driver(void* dl)
		: _dl(dl)
	{ }

	void operator()(plugins::mag_driver* drv)
	{
		delete drv;
		::dlclose(_dl);
	}

private:
	void* _dl;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
driver_ptr make_driver(boost::asio::io_service& ios, const std::string& name)
{
	if (name == "madwifi")
		return boost::make_shared<madwifi_driver>(boost::ref(ios));
	else if (name == "dummy")
		return boost::make_shared<dummy_driver>(boost::ref(ios));

	void* dl = ::dlopen(name.c_str(), RTLD_NOW);
	if (!dl) {
		std::cerr << ::dlerror() << std::endl;
		return driver_ptr();
	}

	void* sm = ::dlsym(dl, "opmip_mag_make_driver");
	if (!sm) {
		::dlclose(dl);
		return driver_ptr();
	}

	plugins::mag_driver* drv = reinterpret_cast<plugins::mag_driver* (*)(boost::asio::io_service&)>(sm)(ios);

	return driver_ptr(drv, delete_dl_driver(dl));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////////
