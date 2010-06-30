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

#include <opmip/pmip/lma.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
lma::lma(boost::asio::io_service& ios, node_db& ndb, size_t concurrency)
	: _service(ios), _node_db(ndb),
	  _mp_sock(ios), _icmp_sock(ios),
	  _route_table(ios), _tunnel_dev(0), _home_net_dev(0),
	  _concurrency(concurrency), _tunnel(ios)
{
}

void lma::start(const char* id, const ip_address& home_network_link)
{
	_service.dispatch(boost::bind(&lma::istart, this, id, home_network_link));
}

void lma::stop()
{
	_service.dispatch(boost::bind(&lma::istop, this));
}

void lma::mp_send_handler(const boost::system::error_code& ec)
{
	if (ec)
		std::cerr << "lma::mp_send_handler error: " << ec.message() << std::endl;
}

void lma::icmp_ra_timer_handler(const boost::system::error_code& ec, const std::string& mn_id)
{
	if (ec) {
		std::cerr << "lma::icmp_ra_timer_handler error: " << ec.message() << std::endl;
		return;
	}

	_service.dispatch(boost::bind(&lma::irouter_advertisement, this, mn_id));
}

void lma::icmp_ra_send_handler(const boost::system::error_code& ec)
{
	if (ec)
		std::cerr << "lma::icmp_ra_send_handler error: " << ec.message() << std::endl;
}

void lma::bcache_remove_entry(const boost::system::error_code& ec, const std::string& mn_id)
{
	if (!ec)
		_service.dispatch(boost::bind(&lma::ibcache_remove_entry, this, mn_id));
}

void lma::istart(const char* id, const ip_address& home_network_link)
{
	if (!home_network_link.is_link_local()) {
		error_code ec(boost::system::errc::invalid_argument, boost::system::get_generic_category());

		throw_exception(exception(ec, "Home network link must be a link local IPv6 address"));
	}

	const lma_node* node = _node_db.find_lma(id);
	if (!node) {
		error_code ec(boost::system::errc::invalid_argument, boost::system::get_generic_category());

		throw_exception(exception(ec, "LMA id not found in node database"));
	}

	_mp_sock.open(ip::mproto());
	_mp_sock.bind(ip::mproto::endpoint(node->address()));

	_icmp_sock.open(boost::asio::ip::icmp::v6());
	_icmp_sock.bind(boost::asio::ip::icmp::endpoint(home_network_link, 0));
//	_icmp_sock.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address_v6::from_string("ff02::2")));
//	_icmp_sock.set_option(ip::icmp::filter(true, ip::icmp::router_solicitation::type_value));

	_home_net_dev = home_network_link.scope_id();
	_identifier   = id;

	_tunnel.open("lma0", node->device_id(), node->address(), ip_address::any());
	_tunnel.set_enable(true);
	_tunnel_dev = _tunnel.get_device_id();
}

void lma::istop()
{
	_bcache.clear();
	_tunnel.close();
	_mp_sock.close();
	_icmp_sock.close();
}

void lma::ibcache_remove_entry(const std::string& mn_id)
{
	auto be = _bcache.find(mn_id);
	if (!be || be->bind_status != bcache_entry::k_bind_deregistered)
		return;

	_bcache.remove(be);
}

void lma::add_route_entries(bcache_entry* be)
{
	const bcache::net_prefix_list& npl = be->prefix_list();

	for (bcache::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_src(*i, _tunnel_dev, be->care_of_address);

	for (bcache::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_dst(*i, _home_net_dev);
}

void lma::del_route_entries(bcache_entry* be)
{
	const bcache::net_prefix_list& npl = be->prefix_list();

	for (bcache::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_src(*i);

	for (bcache::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_dst(*i);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
