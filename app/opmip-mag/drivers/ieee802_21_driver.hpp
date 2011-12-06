//==============================================================================
// Brief   : IEEE 802.21 MIH User Driver
// Authors : Bruno Santos <bsantos@av.it.pt>
//           Carlos Guimarães <cguimaraes@av.it.pt>
// -----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//==============================================================================

#ifndef OPMIP_APP_DRIVER_IEEE802_21_DRIVER__HPP_
#define OPMIP_APP_DRIVER_IEEE802_21_DRIVER__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/plugins/mag_driver.hpp>
#include <opmip/pmip/mag.hpp>
#include <boost/scoped_ptr.hpp>

#include <odtone/base.hpp>
#include <odtone/sap/user.hpp>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
/**
 * This class provides an implementation of an IEEE 802.21 driver for the MAG.
 */
class ieee802_21_driver : public plugins::mag_driver {
public:
	/**
	 * Construct the IEEE 802.21 driver.
	 *
	 * @param ios The io_service object that the IEEE 802.21 driver will use to
	 * dispatch handlers for any asynchronous operations performed on the socket.
	 * @param mag The associated MAG module.
	 */
	ieee802_21_driver(boost::asio::io_service& ios, pmip::mag& mag);

	/**
	 * Destruct the IEEE 802.21 driver.
	 */
	~ieee802_21_driver();

	/**
	 * Start the IEEE 802.21 driver.
	 *
	 * @param options The input driver options.
	 */
	void start(const std::vector<std::string>& options);

	/**
	 * Stop the IEEE 802.21 driver.
	 */
	void stop();

private:
	/**
	 * Decode driver input options.
	 *
	 * @param options The input driver options.
	 */
	void decode_options(const std::vector<std::string>& options);

	/**
	 * User registration handler.
	 *
	 * @param msg Received message.
	 * @param ec Error Code.
	 */
	void user_reg_handler(odtone::mih::message &msg, const boost::system::error_code& ec);

	/**
	 * Capability Discover handler.
	 *
	 * @param msg Received message.
	 * @param ec Error Code.
	 */
	void capability_discover_confirm(odtone::mih::message& msg, const boost::system::error_code& ec);


	void event_subscribe_response(odtone::mih::message& msg, const boost::system::error_code& ec);

	/**
	 * Default MIH event handler.
	 *
	 * @param msg Received message.
	 * @param ec Error code.
	 */
	void event_handler(odtone::mih::message& msg, const boost::system::error_code& ec);

private:
	boost::scoped_ptr<odtone::sap::user> _user_sap;
	boost::asio::io_service&             _service;
	pmip::mag&                           _mag;

	// mapping<MIHF ID, MAG interface name>
	std::map<std::string, std::string>	_poa;
	// mapping<Interface, MAG interface name>
	std::map<std::string, std::string>	_local;
	std::string _local_mihf;
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_DRIVER_IEEE802_21_DRIVER__HPP_ */
