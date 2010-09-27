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

	struct attach_info {
		attach_info(const ll::mac_address& mn_ll_address_,
		            const ip::address_v6& poa_ip_address_,
		            const ll::mac_address& poa_ll_address_,
		            uint poa_dev_id_)

			: mn_ll_address(mn_ll_address_), poa_ip_address(poa_ip_address_),
			  poa_ll_address(poa_ll_address_), poa_dev_id(poa_dev_id_)
		{ }


		ll::mac_address  mn_ll_address;
		ip::address_v6   poa_ip_address;
		ll::mac_address  poa_ll_address;
		uint             poa_dev_id;
//		mobility_options mob_options;
	};

public:
	mag(boost::asio::io_service& ios, node_db& ndb, size_t concurrency);

	void start(const char* id, const ip_address& mn_access_link);
	void stop();

	void mobile_node_attach(const attach_info& ai);
	void mobile_node_detach(const attach_info& ai);

private:
	void mp_send_handler(const boost::system::error_code& ec);
	void mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pba_receiver_ptr& pbar);

	void icmp_ra_send_handler(const boost::system::error_code& ec);

private:
	void istart(const char* id, const ip_address& mn_access_link);
	void istop();

	void imobile_node_attach(const attach_info& ai);
	void imobile_node_detach(const attach_info& ai);

	void irouter_solicitation(const boost::system::error_code& ec, const ip_address& address, const mac_address& mac, icmp_rs_receiver_ptr& rsr);
	void irouter_advertisement(const boost::system::error_code& ec, const std::string& mn_id);

	void iproxy_binding_ack(const proxy_binding_info& pbinfo);
	void iproxy_binding_retry(const boost::system::error_code& ec, proxy_binding_info& pbinfo);

	void add_route_entries(bulist_entry& be);
	void del_route_entries(bulist_entry& be);

	void setup_icmp_socket(bulist_entry& be);

private:
	strand   _service;
	bulist   _bulist;
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
#endif /* OPMIP_PMIP_MAG__HPP_ */
