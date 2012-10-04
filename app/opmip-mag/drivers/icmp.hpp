//==============================================================================
// Brief   : ICMP Driver
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

#ifndef OPMIP_APP_DRIVER_ICMP__HPP_
#define OPMIP_APP_DRIVER_ICMP__HPP_

////////////////////////////////////////////////////////////////////////////////
#include <opmip/plugins/mag_driver.hpp>
#include <opmip/net/link/address_mac.hpp>
#include <opmip/pmip/mag.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

#include <map>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
class icmp_drv : public plugins::mag_driver {
	typedef boost::asio::ip::icmp::socket          icmp_sock;
	typedef boost::asio::ip::icmp::endpoint        icmp_endpoint;
	typedef std::map<uint, net::link::address_mac> poa_dev_map;

	struct msg_buffer;

public:
	icmp_drv(boost::asio::io_service& ios, pmip::mag& mag);
	~icmp_drv();

	virtual void start(const std::vector<std::string>& options);
	virtual void stop();

private:
	void recv();
	void handle_recv(const boost::system::error_code& ec, size_t rbytes, boost::shared_ptr<msg_buffer> msg);
	void handle_rs(net::ip::address_v6& ep, net::link::address_mac& laddr);
	void handle_attach(const boost::system::error_code& ec, net::ip::address_v6& ep);

private:
	pmip::mag&          _mag;
	icmp_sock           _icmp_sock;
	boost::asio::strand _strand;
	poa_dev_map         _poa_dev_map;
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_DRIVER_ICMP__HPP_ */
