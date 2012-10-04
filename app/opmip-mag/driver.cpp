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
#include "drivers/icmp.hpp"
#include "drivers/madwifi_driver.hpp"
#ifdef OPMIP_CFG_ODTONE
	#include "drivers/ieee802_21_driver.hpp"
#endif /* #ifdef OPMIP_CFG_ODTONE */
#include "drivers/dummy.hpp"
#include <opmip/plugins/mag.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <dlfcn.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct mag_proxy : public plugins::mag {
	void attach_(const std::string& mn_id, const net::link::address_mac& mn_laddr,
	                     uint poa_dev_id, const net::link::address_mac& poad_dev_laddr,
	                     completion_handler& h)
	{
		pmip::mag::attach_info ai(poa_dev_id, poad_dev_laddr, mn_id, mn_laddr);

		mag->mobile_node_attach(ai, h);
	}

	void detach_(const std::string& mn_id, completion_handler& h)
	{
		pmip::mag::attach_info ai(0, net::link::address_mac(), mn_id, net::link::address_mac());

		mag->mobile_node_detach(ai, h);
	}

	pmip::mag* mag;
};

static mag_proxy s_mag_proxy;

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
driver_ptr make_driver(boost::asio::io_service& ios, pmip::mag& mag, const std::string& name)
{
	if (name == "icmp")
		return boost::make_shared<icmp_drv>(boost::ref(ios), boost::ref(mag));
	if (name == "madwifi")
		return boost::make_shared<madwifi_driver>(boost::ref(ios), boost::ref(mag));
#ifdef OPMIP_CFG_ODTONE
	else if (name == "ieee802.21")
		return boost::make_shared<ieee802_21_driver>(boost::ref(ios), boost::ref(mag));
#endif /* #ifdef OPMIP_CFG_ODTONE */
	else if (name == "dummy")
		return boost::make_shared<dummy_driver>(boost::ref(ios), boost::ref(mag));


	void* dl = ::dlopen(name.c_str(), RTLD_NOW);
	if (!dl) {
		std::cerr << ::dlerror() << std::endl;
		return driver_ptr();
	}

	//
	// TODO: add some sort of tag at the end of the driver import method name. This tage should encode
	// some ABI info, ex.: debug/release, Boost version, ASIO io_service engine, compiler, std ABI version.
	//
	// The idead is to provide some ABI incompatiblity detection to narrow down to a minimum crashes related
	// to ABI issues.
	//
	void* sm = ::dlsym(dl, "opmip_mag_make_driver");
	if (!sm) {
		::dlclose(dl);
		return driver_ptr();
	}

	typedef plugins::mag_driver* (*plugin_entry_type)(boost::asio::io_service&, plugins::mag&);

	//
	// TODO: This is an hack, instead the passed class should derive from plugins::mag thus not requiring a
	// proxy. This must be dones such that the builtin drivers can use the mag interface without using the
	// virtual methods.
	//
	if (!s_mag_proxy.mag)
		s_mag_proxy.mag = boost::addressof(mag);

	plugins::mag_driver* drv = reinterpret_cast<plugin_entry_type>(sm)(ios, s_mag_proxy);

	return driver_ptr(drv, delete_dl_driver(dl));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////////
