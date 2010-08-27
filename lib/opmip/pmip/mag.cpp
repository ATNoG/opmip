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
#include <opmip/pmip/mp_sender.hpp>
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
	  _mp_sock(ios), _tunnels(ios), _route_table(ios),
	  _concurrency(concurrency)
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

void mag::mobile_node_attach(const attach_info& ai)
{
	_service.dispatch(boost::bind(&mag::imobile_node_attach, this, ai));
}

void mag::mobile_node_detach(const attach_info& ai)
{
	_service.dispatch(boost::bind(&mag::imobile_node_detach, this, ai));
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
	const mag_node* node = _node_db.find_mag(id);
	if (!node) {
		error_code ec(boost::system::errc::invalid_argument, boost::system::get_generic_category());

		throw_exception(exception(ec, "MAG id not found in node database"));
	}
	_log(0, "Starting... [id = ", id, ", mn_access_link = ", mn_access_link, ", address = ", node->address(), "]");

	_mp_sock.open(ip::mproto());
	_mp_sock.bind(ip::mproto::endpoint(node->address()));

	_identifier = id;

	_tunnels.set_local_address(ip::address_v6(node->address().to_bytes(), node->device_id()));

	for (size_t i = 0; i < _concurrency; ++i) {
		pba_receiver_ptr pbar(new pba_receiver);

		pbar->async_receive(_mp_sock, boost::bind(&mag::mp_receive_handler, this, _1, _2, _3));
	}
}

void mag::istop()
{
	_log(0, "Stoping...");

	_bulist.clear();
	_mp_sock.close();
	_route_table.clear();
	_tunnels.clear();
}

void mag::imobile_node_attach(const attach_info& ai)
{
	bulist_entry* be = _bulist.find(ai.mn_ll_address);
	if (!be) {
		const mobile_node* mn = _node_db.find_mobile_node(ai.mn_ll_address);
		if (!mn) {
			_log(0, "Mobile Node attach error: not authorized [mac = ", ai.mn_ll_address, "]");
			return;
		}

		const lma_node* lma = _node_db.find_lma(mn->lma_id());
		if (!lma) {
			_log(0, "Mobile Node attach error: unknown LMA [id = ", mn->id(), " (", ai.mn_ll_address, "), lma = ", mn->lma_id(), "]");
			return;
		}

		be = new bulist_entry(_service.get_io_service(), mn->id(), ai.mn_ll_address, mn->prefix_list(), lma->address(),
		                      ai.poa_ip_address, ai.poa_ll_address, ai.poa_dev_id);

		_bulist.insert(be);
		setup_icmp_socket(*be);

		_log(0, "Mobile Node attach [id = ", mn->id(), " (", ai.mn_ll_address, ")"
		                          ", lma = ", mn->lma_id(), " (", be->lma_address(), ")]");
	} else {
		_log(0, "Mobile Node re-attach [id = ", be->mn_id(), " (", ai.mn_ll_address, "), lma = ", be->lma_address(), "]");
	}

	if (be->bind_status == bulist_entry::k_bind_error) {
		_log(0, "Mobile Node attach error: previous bind failed [id = ", be->mn_id(), " (", ai.mn_ll_address, "), lma = ", be->lma_address(), "]");
		return;
	}

	proxy_binding_info pbinfo;

	be->last_ack_sequence = be->sequence_number;
	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = ++be->sequence_number;
	pbinfo.lifetime = ~0;
	pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_requested;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::seconds(1.5)); //FIXME: set a proper timer
	be->timer.async_wait(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo));
}

void mag::imobile_node_detach(const attach_info& ai)
{
	const mobile_node* mn = _node_db.find_mobile_node(ai.mn_ll_address);
	if (!mn) {
		_log(0, "Mobile Node detach error: not authorized [mac = ", ai.mn_ll_address, "]");
		return;
	}

	bulist_entry* be = _bulist.find(mn->id());
	if (!be || (be->bind_status != bulist_entry::k_bind_requested && be->bind_status != bulist_entry::k_bind_ack)) {
		_log(0, "Mobile Node detach error: not attached [id = ", mn->id(), " (", ai.mn_ll_address, ")", "]");
		return;
	}
	_log(0, "Mobile Node detach [id = ", mn->id(), " (", ai.mn_ll_address, "), lma = ", be->lma_address(), "]");

	if (be->bind_status == bulist_entry::k_bind_ack)
		del_route_entries(*be);

	proxy_binding_info pbinfo;

	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = ++be->sequence_number;
	pbinfo.lifetime = 0;
	pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_detach;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::milliseconds(1500));
	be->timer.async_wait(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo));
}

void mag::irouter_solicitation(const boost::system::error_code& ec, const ip_address& address, const mac_address& mac, icmp_rs_receiver_ptr& rsr)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "ICMPv6 router solicitation receive error: ", ec.message());

		return;
	}

	bulist_entry* be = _bulist.find(mac);
	if (!be || be->bind_status != bulist_entry::k_bind_ack) {
		_log(0, "Router Solicitation error: unknown mobile node [mac = ", mac, "]");
		return;
	}
	_log(0, "Router solicitation [mac = ", mac, ", id = ", be->mn_id(), "]");

	rsr->async_receive(be->icmp_sock, _service.wrap(boost::bind(&mag::irouter_solicitation, this, _1, _2, _3, _4)));

	router_advertisement_info rainfo;

	rainfo.link_address = be->poa_ll_address();
	rainfo.mtu = be->mtu;
	rainfo.prefix_list = be->mn_prefix_list();
	rainfo.destination = address;

	icmp_ra_sender_ptr ras(new icmp_ra_sender(rainfo));

	ras->async_send(be->icmp_sock, boost::bind(&mag::icmp_ra_send_handler, this, _1));
}

void mag::irouter_advertisement(const std::string& mn_id)
{
	bulist_entry* be = _bulist.find(mn_id);
	if (!be || be->bind_status != bulist_entry::k_bind_ack) {
		_log(0, "Router advertisement error: binding update list not found [id = ", be->mn_id(), "]");
		return;
	}

	router_advertisement_info rainfo;

	rainfo.link_address = be->poa_ll_address();
	rainfo.mtu = be->mtu;
	rainfo.prefix_list = be->mn_prefix_list();
	rainfo.destination = ip::address_v6::from_string("ff02::1");

	icmp_ra_sender_ptr ras(new icmp_ra_sender(rainfo));

	ras->async_send(be->icmp_sock, boost::bind(&mag::icmp_ra_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::seconds(3)); //FIXME: set a proper timer
	be->timer.async_wait(boost::bind(&mag::icmp_ra_timer_handler, this, _1, mn_id));
}

void mag::iproxy_binding_ack(const proxy_binding_info& pbinfo)
{
	bulist_entry* be = _bulist.find(pbinfo.id);
	if (!be) {
		_log(0, "PBA error: binding update list entry not found [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	if (be->lma_address() != pbinfo.address) {
		_log(0, "PBA error: not this LMA [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	if ((pbinfo.sequence <= be->last_ack_sequence) || (pbinfo.sequence > be->sequence_number)) {
		_log(0, "PBA error: sequence number invalid [id = ", pbinfo.id,
		                                          ", lma = ", pbinfo.address,
		                                          ", sequence = ", be->last_ack_sequence,
		                                                    " < ", pbinfo.sequence,
		                                                   " <= ", be->sequence_number, "]");
		return;
	}

	be->last_ack_sequence = pbinfo.sequence;

	if (pbinfo.lifetime && be->bind_status == bulist_entry::k_bind_requested) {
		_log(0, "PBA registration [id = ", pbinfo.id, ", lma = ", pbinfo.address, ", status = ", pbinfo.status,"]");

		if (pbinfo.status == ip::mproto::pba::status_ok) {
			add_route_entries(*be);
			be->bind_status = bulist_entry::k_bind_ack;
			irouter_advertisement(be->mn_id());

		} else {
			be->bind_status = bulist_entry::k_bind_error;
			be->timer.cancel();
		}

	} else 	if (!pbinfo.lifetime && be->bind_status == bulist_entry::k_bind_detach) {
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

	if (pbinfo.lifetime)
		_log(0, "PBU register retry [id = ", pbinfo.id,
			                      ", lma = ", pbinfo.address,
			                      ", sequence = ", pbinfo.sequence,
			                      ", retry_count = ", uint(be->retry_count),
			                      ", delay = ", delay, "]");
	else
		_log(0, "PBU de-register retry [id = ", pbinfo.id,
			                         ", lma = ", pbinfo.address,
			                         ", sequence = ", pbinfo.sequence,
			                         ", retry_count = ", uint(be->retry_count),
			                         ", delay = ", delay, "]");
}

void mag::add_route_entries(bulist_entry& be)
{
	const bulist::net_prefix_list& npl = be.mn_prefix_list();
	uint adev = be.poa_dev_id();
	uint tdev = _tunnels.get(be.lma_address());

	_log(0, "Add route entries [id = ", be.mn_id(), ", tunnel = ", tdev, ", LMA = ", be.lma_address(), "]");

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_dst(*i, adev);

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_src(*i, tdev);
}

void mag::del_route_entries(bulist_entry& be)
{
	const bulist::net_prefix_list& npl = be.mn_prefix_list();

	_log(0, "Remove route entries [id = ", be.mn_id(), ", LMA = ", be.lma_address(), "]");

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_dst(*i);

	for (bulist::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_src(*i);

	_tunnels.del(be.lma_address());
}

void mag::setup_icmp_socket(bulist_entry& be)
{
	be.icmp_sock.open(boost::asio::ip::icmp::v6());
	be.icmp_sock.bind(boost::asio::ip::icmp::endpoint(be.poa_ip_address(), 0));
	be.icmp_sock.set_option(boost::asio::ip::multicast::hops(255));
	be.icmp_sock.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address_v6::from_string("ff02::2")));
	be.icmp_sock.set_option(ip::icmp::filter(true, ip::icmp::router_solicitation::type_value));

	for (size_t i = 0; i < _concurrency; ++i) {
		icmp_rs_receiver_ptr rsr(new icmp_rs_receiver);

		rsr->async_receive(be.icmp_sock, _service.wrap(boost::bind(&mag::irouter_solicitation, this, _1, _2, _3, _4)));
	}
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
