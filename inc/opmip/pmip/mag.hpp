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
#include <opmip/pmip/tunnels.hpp>
#include <opmip/pmip/addrconf_server.hpp>
#include <opmip/sys/route_table.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/bind.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class error_category : public boost::system::error_category {
public:
	error_category()
	{ }

	const char* name() const;
	std::string message(int ev) const;
};

///////////////////////////////////////////////////////////////////////////////
class mag {
	typedef boost::asio::io_service::strand                         strand;
	typedef boost::function<void(const boost::system::error_code&)> completion_functor;

public:
	typedef ip::address_v6  ip_address;
	typedef ll::mac_address mac_address;

	class error_category : public boost::system::error_category {
	public:
		error_category()
		{ }

		const char* name() const;
		std::string message(int ev) const;
	};

	enum error_code {
		ec_success,
		ec_not_authorized,
		ec_unknown_lma,
		ec_invalid_state,
		ec_canceled,
		ec_timeout,
	};

	struct attach_info {
		attach_info(uint poa_dev_id_,
		            const ll::mac_address& poa_address_,
		            const std::string& mn_id_,
		            const ll::mac_address& mn_address_)

			: poa_dev_id(poa_dev_id_), poa_address(poa_address_),
			  mn_id(mn_id_), mn_address(mn_address_)
		{ }

		uint            poa_dev_id;
		ll::mac_address poa_address;
		std::string     mn_id;
		ll::mac_address mn_address;
//		mobility_options mob_options;
	};


public:
	mag(boost::asio::io_service& ios, node_db& ndb, addrconf_server& asrv, size_t concurrency);

	void start(const std::string& id, const ip_address& link_local_ip);
	void stop();

	template<class CompletionHandler>
	void mobile_node_attach(const attach_info& ai, CompletionHandler handler);

	template<class CompletionHandler>
	void mobile_node_detach(const attach_info& ai, CompletionHandler handler);

	node_db& get_node_database() { return _node_db; }

private:
	void mp_send_handler(const boost::system::error_code& ec);
	void mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pba_receiver_ptr& pbar, chrono& delay);

private:
	void start_(const std::string& id, const ip_address& mn_access_link);
	void stop_();

	void mobile_node_attach_(const attach_info& ai, completion_functor& completion_handler);
	void mobile_node_detach_(const attach_info& ai, completion_functor& completion_handler);

	void proxy_binding_ack(const proxy_binding_info& pbinfo, chrono& delay);
	void proxy_binding_retry(const boost::system::error_code& ec, proxy_binding_info& pbinfo);
	void proxy_binding_renew(const boost::system::error_code& ec, const std::string& id);

	void add_route_entries(bulist_entry& be);
	void del_route_entries(bulist_entry& be);

private:
	strand   _service;
	bulist   _bulist;
	node_db& _node_db;
	logger   _log;

	addrconf_server&   _addrconf;
	ip::mproto::socket _mp_sock;

	std::string       _identifier;
	ip_address        _link_local_ip;
	pmip::ip6_tunnels _tunnels;
	sys::route_table  _route_table;
	size_t            _concurrency;
};

template<class CompletionHandler>
inline void mag::mobile_node_attach(const attach_info& ai, CompletionHandler handler)
{
	completion_functor ch(handler);

	_service.dispatch(boost::bind(&mag::mobile_node_attach_, this, ai, ch));
}

template<class CompletionHandler>
inline void mag::mobile_node_detach(const attach_info& ai, CompletionHandler handler)
{
	completion_functor ch(handler);

	_service.dispatch(boost::bind(&mag::mobile_node_detach_, this, ai, ch));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_MAG__HPP_ */
