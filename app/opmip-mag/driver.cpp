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
#include <opmip/logger.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
opmip::logger log_("madwifi", std::cout);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class madwifi_drv : public driver {
	static void link_event(const boost::system::error_code& ec, const madwifi_driver::event& ev,
	                       pmip::mag& mag)
	{
		if (ec)
			return;

		opmip::pmip::mag::attach_info ai(ev.if_index,
										 ev.if_address,
										 ev.mn_address);

		switch (ev.which) {
		case opmip::app::madwifi_driver_impl::attach:
			mag.mobile_node_attach(ai, [ev](uint ec) {
				log_(0, "node ", ev.mn_address, " attachment completed with code ", ec);
			});
			break;

		case opmip::app::madwifi_driver_impl::detach:
			mag.mobile_node_detach(ai, [ev](uint ec) {
				log_(0, "node ", ev.mn_address, " detachment completed with code ", ec);
			});
			break;

		default:
			break;
		}
	}

public:
	madwifi_drv(boost::asio::io_service& ios)
		: _drv(ios)
	{ }

	virtual void start(pmip::mag& mag, const boost::any& parameter)
	{
		auto interfaces = boost::any_cast<const std::vector<std::string>&>(parameter);

		_drv.start(interfaces, boost::bind(&madwifi_drv::link_event, _1, _2, boost::ref(mag)));
	}

	virtual void stop()
	{
		_drv.stop();
	}

private:
	opmip::app::madwifi_driver _drv;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
driver_ptr make_driver(boost::asio::io_service& ios, const std::string& name)
{
	if (name == "madwifi")
		return boost::make_shared<madwifi_drv>(ios);

	return driver_ptr();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////////
