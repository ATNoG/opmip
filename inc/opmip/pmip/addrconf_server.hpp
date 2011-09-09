//=============================================================================
// Brief   : Address Configuration Server
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

#ifndef OPMIP_PMIP_ADDRCONF_SERVER__HPP_
#define OPMIP_PMIP_ADDRCONF_SERVER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/ip/prefix.hpp>
#include <opmip/net/ip/address.hpp>
#include <opmip/net/link/address_mac.hpp>
#include <opmip/net/link/ethernet.hpp>
#include <opmip/pmip/types.hpp>
#include <opmip/pmip/icmp_sender.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <map>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class addrconf_server {
	typedef boost::shared_ptr<boost::asio::deadline_timer>   timer_ptr;
	typedef boost::shared_ptr<boost::asio::ip::icmp::socket> icmp_sock_ptr;
	typedef boost::shared_ptr<boost::asio::ip::udp::socket>  udp_sock_ptr;

public:
	typedef net::ip::prefix_v6     ip6_prefix;
	typedef net::ip::address_v6    ip6_address;
	typedef net::link::address_mac link_address;

	struct device {
		device(uint idx, const link_address& laddr, const ip6_address& llip)
			: id(idx), address(laddr), local_ip(llip)
		{ }

		uint         id;
		link_address address;
		ip6_address  local_ip;
		udp_sock_ptr udp_sock;
	};

	struct client {
		client(const link_address& laddr, const link_address& poa_laddr)
			: link_addr(laddr), poa_link_addr(poa_laddr)
		{ }

		link_address            link_addr;
		link_address            poa_link_addr;
		timer_ptr               ra_timer;
		ip6_address             home_addr;
		std::vector<ip6_prefix> prefixes;
	};

private:
	typedef boost::multi_index_container<
				device,
				boost::multi_index::indexed_by<
					boost::multi_index::ordered_unique<
						boost::multi_index::member<device, uint, &device::id>
					>,
					boost::multi_index::ordered_unique<
						boost::multi_index::member<device, link_address, &device::address>
					>
				>
			>
		devices;

	typedef boost::multi_index_container<
				client,
				boost::multi_index::indexed_by<
					boost::multi_index::ordered_unique<
						boost::multi_index::member<client, link_address, &client::link_addr>
					>
				>
			>
		clients;


public:
	addrconf_server(boost::asio::io_service& ios);
	~addrconf_server();

	bool dev_add(uint id, const link_address& link_addr, const ip6_address& link_local);
	bool dev_rem(uint id);
	void dev_clear();

	bool add(const router_advertisement_info& ai);
	bool del(const link_address& addr);
	void clear();

private:
	void router_advertisement(const boost::system::error_code& ec,
	                          icmp_ra_sender_ptr& ras,
	                          net::link::ethernet::endpoint& ep,
	                          timer_ptr& timer);

private:
	boost::asio::io_service&    _io_service;
	clients                     _clients;
	net::link::ethernet::socket _link_sock;
	devices                     _devices;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_ADDRCONF_SERVER__HPP_ */
