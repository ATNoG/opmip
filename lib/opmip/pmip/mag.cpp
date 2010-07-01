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
#include <opmip/pmip/icmp_sender.hpp>
#include <opmip/ip/icmp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
mag::mag(boost::asio::io_service& ios, node_db& ndb, size_t concurrency)
	: _service(ios), _node_db(ndb), _log("MAG", &std::cout),
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

void mag::mp_send_handler(const boost::system::error_code& ec)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		_log(0, "PBU send error: ", ec.message());
}

void mag::mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pba_receiver_ptr& pbar)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "PBA receive error: ", ec.message());

	} else {
		_service.dispatch(boost::bind(&mag::iproxy_binding_ack, this, pbinfo));
		pbar->async_receive(_mp_sock, boost::bind(&mag::mp_receive_handler, this, _1, _2, _3));
	}
}

void mag::icmp_ra_timer_handler(const boost::system::error_code& ec, const std::string& mn_id)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "ICMPv6 router advertisement timer error: ", ec.message());

	} else {
		_service.dispatch(boost::bind(&mag::irouter_advertisement, this, mn_id));
	}
}

void mag::icmp_ra_send_handler(const boost::system::error_code& ec)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		_log(0, "ICMPv6 router advertisement send error: ", ec.message());
}

void mag::icmp_rs_receive_handler(const boost::system::error_code& ec, const ip_address& address, const mac_address& mac, icmp_rs_receiver_ptr& rsr)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "ICMPv6 router solicitation receive error: ", ec.message());

	} else {
		_service.dispatch(boost::bind(&mag::irouter_solicitation, this, address, mac));
		rsr->async_receive(_icmp_sock, boost::bind(&mag::icmp_rs_receive_handler, this, _1, _2, _3, _4));
	}
}

void mag::proxy_binding_retry(const boost::system::error_code& ec, const proxy_binding_info& pbinfo)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "PBU retry timer error: ", ec.message());

	} else {
		_service.dispatch(boost::bind(&mag::iproxy_binding_retry, this, pbinfo));
	}
}

void mag::istart(const char* id, const ip_address& mn_access_link)
{
	_log(0, "Starting... [id = ", id, ", mn_access_link = ", mn_access_link, "]");

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

	for (size_t i = 0; i < _concurrency; ++i) {
		pba_receiver_ptr pbar(new pba_receiver);

		pbar->async_receive(_mp_sock, boost::bind(&mag::mp_receive_handler, this, _1, _2, _3));
	}

	for (size_t i = 0; i < _concurrency; ++i) {
		icmp_rs_receiver_ptr rsr(new icmp_rs_receiver);

		rsr->async_receive(_icmp_sock, boost::bind(&mag::icmp_rs_receive_handler, this, _1, _2, _3, _4));
	}
}

void mag::istop()
{
	_log(0, "Stoping...");

	_bulist.clear();
	_mp_sock.close();
	_icmp_sock.close();
	_tunnel.close();
}

void mag::imobile_node_attach(const mac_address& mn_mac)
{
	bulist_entry* be = _bulist.find(mn_mac);
	if (!be) {
		const mobile_node* mn = _node_db.find_mobile_node(mn_mac);
		if (!mn) {
			_log(0, "Mobile Node attach error: not found [mac = ", mn_mac, "]");
			return;
		}

		const lma_node* lma = _node_db.find_lma(mn->lma_id());
		if (!lma) {
			_log(0, "Mobile Node attach error: unknown LMA [mac = ", mn_mac, ", id = ", mn->id(), ", lma_id = ", mn->lma_id(), "]");
			return;
		}

		be = new bulist_entry(_service.get_io_service(), mn->id(), mn->mac_address(), mn->prefix_list(), lma->address());

		_log(0, "Mobile Node attach [mac = ", mn_mac, ", id = ", be->mn_id(), ", lma_id = ", mn->lma_id(), ", lma_ip = ", be->lma_address(), "]");
	} else {
		_log(0, "Mobile Node re-attach [mac = ", mn_mac, ", id = ", be->mn_id(), ", lma_ip = ", be->lma_address(), "]");
	}

	if (be->bind_status == bulist_entry::k_bind_error) {
		_log(0, "Mobile Node attach bind error [mac =", mn_mac, ", id = ", be->mn_id(), ", lma_ip = ", be->lma_address(), "]");
		return;
	}

	proxy_binding_info pbinfo;

	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = be->sequence_number;
	pbinfo.lifetime = ~0;
	pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_requested;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::seconds(1.5)); //FIXME: set a proper timer
	be->timer.async_wait(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo));
}

void mag::imobile_node_detach(const mac_address& mn_mac)
{
	const mobile_node* mn = _node_db.find_mobile_node(mn_mac);
	if (!mn) {
		_log(0, "Mobile Node detach error: not authorized [mac = ", mn_mac, "]");
		return;
	}

	bulist_entry* be = _bulist.find(mn->id());
	if (!be || be->bind_status != bulist_entry::k_bind_requested || be->bind_status != bulist_entry::k_bind_ack) {
		_log(0, "Mobile Node detach error: not attached [mac = ", mn_mac, ", id = ", be->mn_id(), ", lma_ip = ", be->lma_address(), "]");
		return;
	}

	_log(0, "Mobile Node detach [mac = ", mn_mac, ", id = ", be->mn_id(), ", lma_ip = ", be->lma_address(), "]");

	if (be->bind_status == bulist_entry::k_bind_ack)
		del_route_entries(be);

	proxy_binding_info pbinfo;

	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = be->sequence_number;
	pbinfo.lifetime = 0;
	pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_detach;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::milliseconds(1500));
	be->timer.async_wait(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo));
}

void mag::irouter_solicitation(const ip_address& address, const mac_address& mac)
{
	bulist_entry* be = _bulist.find(mac);
	if (!be || be->bind_status != bulist_entry::k_bind_ack) {
		_log(0, "Router Solicitation error: unknown mobile node [mac = ", mac, "]");
		return;
	}

	_log(0, "Router solicitation [mac = ", mac, ", id = ", be->mn_id(), "]");

	icmp_ra_sender_ptr ras(new icmp_ra_sender(ll::mac_address::from_string("00:18:f3:90:6d:00"),
	                                                                       1460, be->mn_prefix_list(), address));

	ras->async_send(_icmp_sock, boost::bind(&mag::icmp_ra_send_handler, this, _1));
}

void mag::irouter_advertisement(const std::string& mn_id)
{
	bulist_entry* be = _bulist.find(mn_id);
	if (!be || be->bind_status != bulist_entry::k_bind_ack) {
		_log(0, "Router advertisement error: binding update list not found [id = ", be->mn_id(), "]");
		return;
	}

	icmp_ra_sender_ptr ras(new icmp_ra_sender(ll::mac_address::from_string("00:18:f3:90:6d:00"),
	                                                                       1460, be->mn_prefix_list(),
	                                                                       ip::address_v6::from_string("ff02::1")));

	ras->async_send(_icmp_sock, boost::bind(&mag::icmp_ra_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::seconds(3)); //FIXME: set a proper timer
	be->timer.async_wait(boost::bind(&mag::icmp_ra_timer_handler, this, _1, mn_id));
}

void mag::iproxy_binding_ack(const proxy_binding_info& pbinfo)
{
	bulist_entry* be = _bulist.find(pbinfo.id);
	if (!be) {
		_log(0, "PBU error: binding update list entry not found [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	if (be->lma_address() != pbinfo.address) {
		_log(0, "PBU error: not this LMA [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	if (be->sequence_number != pbinfo.sequence) {
		_log(0, "PBU error: sequence number invalid [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	++be->sequence_number;

	if (be->bind_status == bulist_entry::k_bind_requested) {
		_log(0, "PBA registration [id = ", pbinfo.id, ", lma = ", pbinfo.address, ", status = ", pbinfo.status,"]");

		if (pbinfo.status == ip::mproto::pba::status_ok) {
//			add_route_entries(be);
			be->bind_status = bulist_entry::k_bind_ack;
			irouter_advertisement(be->mn_id());

		} else {
			be->bind_status = bulist_entry::k_bind_error;
			be->timer.cancel();
		}

	} else 	if (be->bind_status == bulist_entry::k_bind_detach) {
		_log(0, "PBA de-registration [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");

		be->timer.cancel();
		_bulist.remove(be);

	} else {
		_log(0, "PBA ignored [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
	}
}

void mag::iproxy_binding_retry(proxy_binding_info& pbinfo)
{
	bulist_entry* be = _bulist.find(pbinfo.id);
	if (!be || (be->bind_status != bulist_entry::k_bind_requested && be->bind_status != bulist_entry::k_bind_detach)) {
		_log(0, "PBU retry error: binding update list entry not found [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	++be->retry_count;

	if (be->bind_status == bulist_entry::k_bind_detach && be->retry_count > 3) {
		_log(0, "PBU retry error: max retry count [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		_bulist.remove(be);
		return;
	}

	pbinfo.sequence = ++be->sequence_number;

	pbu_sender_ptr pbus(new pbu_sender(pbinfo));
	double         delay = std::min<double>(32, std::pow(1.5f, be->retry_count)); //FIXME: validate

	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::milliseconds(delay * 1000.f));
	be->timer.async_wait(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo));

	_log(0, "PBU retry [id = ", pbinfo.id,
	                 ", lma = ", pbinfo.address,
	                 ", sequence = ", pbinfo.sequence,
	                 ", retry_count = ", uint(be->retry_count),
	                 ", delay = ", delay, "]");
}

void mag::add_route_entries(bulist_entry* be)
{
	const bulist::net_prefix_list& npl = be->mn_prefix_list();

	_log(0, "Added route entries [id = ", be->mn_id(), "]");

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_src(*i, _tunnel_dev, be->lma_address());

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_dst(*i, _access_dev);
}

void mag::del_route_entries(bulist_entry* be)
{
	const bulist::net_prefix_list& npl = be->mn_prefix_list();

	_log(0, "Removed route entries [id = ", be->mn_id(), "]");

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_src(*i);

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_dst(*i);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
