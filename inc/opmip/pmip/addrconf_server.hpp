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
#include <opmip/net/ip/dhcp_v6.hpp>
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
public:
	typedef net::ip::prefix_v6     ip6_prefix;
	typedef net::ip::address_v6    ip6_address;
	typedef net::link::address_mac link_address;

private:
	typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

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

	typedef boost::multi_index_container<
				client,
				boost::multi_index::indexed_by<
					boost::multi_index::ordered_unique<
						boost::multi_index::member<client, link_address, &client::link_addr>
					>
				>
			>
		clients;

	struct dhcp_receive_data;

	typedef boost::shared_ptr<dhcp_receive_data> dhcp_receive_data_ptr;

public:
	addrconf_server(boost::asio::io_service& ios);
	~addrconf_server();

	void start();
	void stop();

	bool add(const router_advertisement_info& ai);
	bool del(const link_address& addr);
	void clear();

private:
	void router_advertisement(const boost::system::error_code& ec,
	                          icmp_ra_sender_ptr& ras,
	                          net::link::ethernet::endpoint& ep,
	                          timer_ptr& timer);

	void dhcp6_receive_handler(const boost::system::error_code& ec,
	                           size_t blen,
	                           dhcp_receive_data_ptr& rd);

	void dhcp6_handle_message(net::ip::dhcp_v6::buffer_type buff,
	                          const boost::asio::ip::udp::endpoint& ep);

	void dhcp6_reply_message(const boost::asio::ip::udp::endpoint& ep,
	                         net::ip::dhcp_v6::opcode op,
	                         uint tid,
	                         const link_address& poa_addr,
	                         net::ip::dhcp_v6::buffer_type& cid,
	                         uint ia_id,
	                         const ip6_address& home_addr,
	                         bool hdonly);

private:
	clients                       _clients;
	net::link::ethernet::socket   _link_sock;
//	boost::asio::ip::icmp::socket _icmp_sock;
	boost::asio::ip::udp::socket  _udp_sock;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_ADDRCONF_SERVER__HPP_ */
