//==============================================================================
// Brief   : Dummy Driver
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

#ifndef OPMIP_APP_DRIVER_DUMMY__HPP_
#define OPMIP_APP_DRIVER_DUMMY__HPP_

////////////////////////////////////////////////////////////////////////////////
#include "../driver.hpp"
#include <opmip/chrono.hpp>
#include <opmip/pmip/mag.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

////////////////////////////////////////////////////////////////////////////////
class dummy_driver : public plugins::mag_driver {
	typedef boost::mt19937                   rand_engine;
	typedef boost::uniform_int<uint>         rand_distribution;
	typedef std::pair<ll::mac_address, bool> client_state;

public:
	dummy_driver(boost::asio::io_service& ios);
	~dummy_driver();

	virtual void start(pmip::mag& mag, const std::vector<std::string>& options);
	virtual void stop();

private:
	void start_(float frequency, pmip::mag& mag);
	void stop_();
	void timer_handler(const boost::system::error_code& ec);
	void schedule();

private:
	boost::asio::strand         _strand;
	boost::asio::deadline_timer _timer;
	rand_engine                 _rand;
	std::vector<client_state>   _clients;
	pmip::mag*                  _mag;
	float                       _frequency;
	chrono                      _chrono;
};

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_APP_DRIVER_DUMMY__HPP_ */
