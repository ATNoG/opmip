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

#include <opmip/pmip/mag.hpp>
#include <opmip/ip/icmp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/bind.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
mag::mag(boost::asio::io_service& ios, node_db& ndb, size_t concurrency)
	: _service(ios), _node_db(ndb),
	_mp_sock(ios), _icmp_sock(ios),
	_tunnel_dev(0), _access_dev(0), _route_table(ios), _concurrency(concurrency),
	_tunnel(ios)
{
}

void mag::start(const char* id, const ip_address& mn_access_link)
{
	_service.dispatch(boost::bind(&mag::istart, this, id, mn_access_link));
}

void mag::stop()
{
	_service.dispatch(boost::bind(&mag::istop, this));
}

void mag::mobile_node_attach(const mac_address& mn_mac)
{
	_service.dispatch(boost::bind(&mag::imobile_node_attach, this, mn_mac));
}

void mag::mobile_node_detach(const mac_address& mn_mac)
{
	_service.dispatch(boost::bind(&mag::imobile_node_detach, this, mn_mac));
}

void mag::icmp_ra_timer_handler(const boost::system::error_code& ec, const std::string& mn_id)
{
}

void mag::icmp_ra_send_handler(const boost::system::error_code& ec)
{
}

void mag::istart(const char* id, const ip_address& mn_access_link)
{
	if (!mn_access_link.is_link_local()) {
		error_code ec(boost::system::errc::invalid_argument, boost::system::get_generic_category());

		throw_exception(exception(ec, "Access link must be a link local IPv6 address"));
	}

	const mag_node* node = _node_db.find_mag(id);
	if (!node) {
		error_code ec(boost::system::errc::invalid_argument, boost::system::get_generic_category());

		throw_exception(exception(ec, "MAG id not found in node database"));
	}

	_mp_sock.open(ip::mproto());
	_mp_sock.bind(ip::mproto::endpoint(node->address()));

	_icmp_sock.open(boost::asio::ip::icmp::v6());
	_icmp_sock.bind(boost::asio::ip::icmp::endpoint(mn_access_link, 0));
	_icmp_sock.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address_v6::from_string("ff02::2")));
	_icmp_sock.set_option(ip::icmp::filter(true, ip::icmp::router_solicitation::type_value));

	_access_dev = mn_access_link.scope_id();
	_identifier = id;

	_tunnel.open("mag0", node->device_id(), node->address(), ip_address::any());
	_tunnel.set_enable(true);
	_tunnel_dev = _tunnel.get_device_id();
}

void mag::istop()
{
	_bulist.clear();
	_mp_sock.close();
	_icmp_sock.close();
	_tunnel.close();
}

void mag::add_route_entries(bulist_entry* be)
{
	const bulist::net_prefix_list& npl = be->mn_prefix_list();

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_src(*i, _tunnel_dev, be->lma_address());

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_dst(*i, _access_dev);
}

void mag::del_route_entries(bulist_entry* be)
{
	const bulist::net_prefix_list& npl = be->mn_prefix_list();

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_src(*i);

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_dst(*i);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
