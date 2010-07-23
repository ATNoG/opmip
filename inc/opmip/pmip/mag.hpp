//=============================================================================
// Brief   : Mobile Access Gateway Service
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

#ifndef OPMIP_PMIP_MAG__HPP_
#define OPMIP_PMIP_MAG__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/logger.hpp>
#include <opmip/pmip/bulist.hpp>
#include <opmip/pmip/node_db.hpp>
#include <opmip/pmip/mp_receiver.hpp>
#include <opmip/pmip/icmp_receiver.hpp>
#include <opmip/pmip/tunnels.hpp>
#include <opmip/sys/route_table.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/icmp.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class mag {
	typedef boost::asio::io_service::strand strand;

public:
	typedef ip::address_v6  ip_address;
	typedef ll::mac_address mac_address;

public:
	mag(boost::asio::io_service& ios, node_db& ndb, size_t concurrency);

	void start(const char* id, const ip_address& mn_access_link);
	void stop();

	void mobile_node_attach(const mac_address& mn_mac);
	void mobile_node_detach(const mac_address& mn_mac);

private:
	void mp_send_handler(const boost::system::error_code& ec);
	void mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pba_receiver_ptr& pbar);

	void icmp_ra_timer_handler(const boost::system::error_code& ec, const std::string& mn_id);
	void icmp_ra_send_handler(const boost::system::error_code& ec);
	void icmp_rs_receive_handler(const boost::system::error_code& ec, const ip_address& address, const mac_address& mac, icmp_rs_receiver_ptr& rsr);

	void proxy_binding_retry(const boost::system::error_code& ec, const proxy_binding_info& pbinfo);

private:
	void istart(const char* id, const ip_address& mn_access_link);
	void istop();

	void imobile_node_attach(const mac_address& mn_mac);
	void imobile_node_detach(const mac_address& mn_mac);

	void irouter_solicitation(const ip_address& address, const mac_address& mac);
	void irouter_advertisement(const std::string& mn_id);

	void iproxy_binding_ack(const proxy_binding_info& pbinfo);
	void iproxy_binding_retry(proxy_binding_info& pbinfo);

	void add_route_entries(bulist_entry* be);
	void del_route_entries(bulist_entry* be);

private:
	strand   _service;
	bulist   _bulist;
	node_db& _node_db;
	logger   _log;

	ip::mproto::socket            _mp_sock;
	boost::asio::ip::icmp::socket _icmp_sock;

	std::string       _identifier;
	uint              _access_dev;
	pmip::ip6_tunnels _tunnels;
	sys::route_table  _route_table;
	size_t            _concurrency;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_MAG__HPP_ */
