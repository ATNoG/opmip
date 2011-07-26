//=============================================================================
// Brief   : Local Mobility Anchor
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
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
//=============================================================================

#ifndef OPMIP_PMIP_LMA__HPP_
#define OPMIP_PMIP_LMA__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/chrono.hpp>
#include <opmip/logger.hpp>
#include <opmip/ip/mproto.hpp>
#include <opmip/pmip/bcache.hpp>
#include <opmip/pmip/node_db.hpp>
#include <opmip/pmip/mp_receiver.hpp>
#include <opmip/pmip/tunnels.hpp>
#include <opmip/sys/route_table.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class lma {
	typedef boost::asio::io_service::strand strand;

public:
	typedef	ip::address_v6 ip_address;

	struct config {
		config()
			: min_delay_before_BCE_delete(10000),
			  max_delay_before_BCE_assign(1500)
		{ }

		uint min_delay_before_BCE_delete; //MinDelayBeforeBCEDelete (ms)
		uint max_delay_before_BCE_assign; //MaxDelayBeforeNewBCEAssign (ms)
	};

public:
	lma(boost::asio::io_service& ios, node_db& ndb, size_t concurrency);

	void start(const std::string& id);
	void stop();

private:
	void mp_send_handler(const boost::system::error_code& ec);
	void mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pbu_receiver_ptr& pbur, chrono& delay);

private:
	void start_(const std::string& id);
	void stop_();

	void          proxy_binding_update(proxy_binding_info& pbinfo, chrono& delay);
	bcache_entry* pbu_get_be(proxy_binding_info& pbinfo);
	bool          pbu_mag_checkin(bcache_entry& be, proxy_binding_info& pbinfo);
	void          pbu_process(proxy_binding_info& pbinfo);

	void bcache_remove_entry(const boost::system::error_code& ec, const std::string& mn_id);

	void add_route_entries(bcache_entry* be);
	void del_route_entries(bcache_entry* be);

private:
	strand   _service;
	bcache   _bcache;
	config   _config;
	node_db& _node_db;
	logger   _log;

	ip::mproto::socket _mp_sock;

	std::string       _identifier;
	pmip::ip6_tunnels _tunnels;
	sys::route_table  _route_table;
	size_t            _concurrency;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_LMA__HPP_ */
