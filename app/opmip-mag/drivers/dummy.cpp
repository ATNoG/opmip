//===========================================================================================================
// Brief   : Dummy Driver
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------------------------------------
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
//===========================================================================================================

#include "dummy.hpp"
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <ctime>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static opmip::logger log_("dummy", std::cout);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void dummy()
{
}

dummy_driver::dummy_driver(boost::asio::io_service& ios)
	: _strand(ios), _timer(ios), _rand(std::time(0))
{
}

dummy_driver::~dummy_driver()
{
}

void dummy_driver::start(pmip::mag& mag, const std::vector<std::string>& options)
{
	uint frequency = boost::lexical_cast<uint>(options.at(0));

	BOOST_ASSERT(0 < frequency || frequency <= 1000);

	_strand.dispatch(boost::bind(&dummy_driver::start_, this, frequency, boost::ref(mag)));
}

void dummy_driver::start_(uint frequency, pmip::mag& mag)
{
	pmip::node_db& db = mag.get_node_database();

	_timer.cancel();
	_clients.clear();
	_mag = boost::addressof(mag);
	_frequency = frequency;

	for (pmip::node_db::mobile_node_iterator i = db.mobile_node_begin(), e = db.mobile_node_end(); i != e; ++i)
		_clients.push_back(client_state(i->mac_address(), false));

	log_(0, "using ", _clients.size(), " mobile node(s), about to generate ", frequency, " message(s) per second");

	schedule();
}

void dummy_driver::stop()
{
	_strand.dispatch(boost::bind(&dummy_driver::stop_, this));
}

void dummy_driver::stop_()
{
	_timer.cancel();
	_clients.clear();
}

void dummy_driver::timer_handler(const boost::system::error_code& ec)
{
	typedef boost::variate_generator<rand_engine&, rand_distribution> rand_generator;

	_chrono.stop();
	if (ec)
		return;

	rand_generator dice(_rand, rand_distribution(0, _clients.size() - 1));
	uint n = dice();

	log_(0, "after ", _chrono.get(), " seconds we rolled the dice and got ", n);

	if (!_clients[n].second) {
		_mag->mobile_node_attach(pmip::mag::attach_info(1, ll::mac_address(), _clients[n].first), boost::bind(dummy));
		_clients[n].second = true;
	} else {
		_mag->mobile_node_detach(pmip::mag::attach_info(1, ll::mac_address(), _clients[n].first), boost::bind(dummy));
		_clients[n].second = false;
	}

	schedule();
}

void dummy_driver::schedule()
{
	_chrono.start();
	_timer.expires_from_now(boost::posix_time::milliseconds(1000 / _frequency));
	_timer.async_wait(_strand.wrap(boost::bind(&dummy_driver::timer_handler, this, _1)));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////////
