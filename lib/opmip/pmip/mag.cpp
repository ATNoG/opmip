//=============================================================================
// Brief   : Mobile Access Gateway Service
// Authors : Bruno Santos <bsantos@av.it.pt>
// Authors : Filipe Manco <filipe.manco@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#include <opmip/exception.hpp>
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
static const error_category      k_pba_error_category;
static const mag::error_category k_mag_error_category;

static const boost::system::error_category& pba_error_category()
{
	return k_pba_error_category;
}

static const boost::system::error_category& mag_error_category()
{
	return k_mag_error_category;
}

///////////////////////////////////////////////////////////////////////////////
inline void report_completion(boost::asio::io_service::strand& srv,
                              boost::function<void(const boost::system::error_code&)>& handler,
                              const boost::system::error_code& ec)
{
	if (handler) {
		srv.post(boost::bind(handler, ec));
		handler.clear();
	}
}

///////////////////////////////////////////////////////////////////////////////
const char* error_category::name() const
{
	return "PMIP status codes";
}

std::string error_category::message(int ev) const
{
	switch (ev) {
	case ip::mproto::pba::status_ok:                                 return "accepted"; break;
	case ip::mproto::pba::status_ok_needs_prefix:                    return "accepted but prefix discovery necessary"; break;
	case ip::mproto::pba::status_unspecified:                        return "reason unspecified"; break;
	case ip::mproto::pba::status_prohibited:                         return "administratively prohibited"; break;
	case ip::mproto::pba::status_insufficient_resources:             return "insufficient resources"; break;
	case ip::mproto::pba::status_hr_not_supported:                   return "home registration not supported"; break;
	case ip::mproto::pba::status_not_home_subnet:                    return "not home subnet"; break;
	case ip::mproto::pba::status_not_home_agent:                     return "not home agent for this mobile node"; break;
	case ip::mproto::pba::status_duplicate_address:                  return "duplicate Address Detection failed"; break;
	case ip::mproto::pba::status_bad_sequence:                       return "sequence number out of window"; break;
	case ip::mproto::pba::status_expired_home:                       return "expired home nonce index"; break;
	case ip::mproto::pba::status_expired_care_of:                    return "expired care-of nonce index"; break;
	case ip::mproto::pba::status_expired:                            return "expired nonces"; break;
	case ip::mproto::pba::status_invalid_registration:               return "registration type change disallowed"; break;
	case ip::mproto::pba::status_not_lma_for_this_mn:                return "not local mobility anchor for this mobile node"; break;
	case ip::mproto::pba::status_not_authorized_for_proxy_reg:       return "the mobile access gateway is not authorized to send proxy binding updates"; break;
	case ip::mproto::pba::status_not_authorized_for_net_prefix:      return "the mobile node is not authorized for one or more of the requesting home network prefixes"; break;
	case ip::mproto::pba::status_timestamp_missmatch:                return "invalid timestamp value (the clocks are out of sync)"; break;
	case ip::mproto::pba::status_timestamp_lower_than_prev_accepted: return "the timestamp value is lower than the previously accepted value"; break;
	case ip::mproto::pba::status_missing_home_network_prefix:        return "missing home network prefix option"; break;
	case ip::mproto::pba::status_bce_pbu_prefix_do_not_match:        return "the home network prefixes listed in the BCE do not match the prefixes in the received PBU"; break;
	case ip::mproto::pba::status_missing_mn_identifier_option:       return "missing mobile node identifier option"; break;
	case ip::mproto::pba::status_missing_handoff_indicator_option:   return "missing handoff indicator option"; break;
	case ip::mproto::pba::status_missing_access_type_tech_option:    return "missing access technology type"; break;
	}

	return std::string();
}

///////////////////////////////////////////////////////////////////////////////
const char* mag::error_category::name() const
{
	return "opmip::pmip::mag error codes";
}

std::string mag::error_category::message(int ev) const
{
	switch (ev) {
	case ec_success:        return "success"; break;
	case ec_not_authorized: return "not authorized"; break;
	case ec_unknown_lma:    return "unknown lma"; break;
	case ec_invalid_state:  return "invalid binding state"; break;
	case ec_canceled:       return "binding canceled"; break;
	case ec_timeout:        return "timeout"; break;
	}

	return std::string();
}

///////////////////////////////////////////////////////////////////////////////
mag::mag(boost::asio::io_service& ios, node_db& ndb, addrconf_server& asrv, size_t concurrency)
	: _service(ios), _node_db(ndb), _log("MAG", std::cout), _addrconf(asrv),
	  _mp_sock(ios), _tunnels(ios), _route_table(ios),
	  _concurrency(concurrency)
{
}

void mag::start(const std::string& id, const ip_address& mn_access_link, bool tunnel_global_address)
{
	_service.dispatch(boost::bind(&mag::start_, this, id, mn_access_link, tunnel_global_address));
}

void mag::stop()
{
	_service.dispatch(boost::bind(&mag::stop_, this));
}

void mag::mp_send_handler(const boost::system::error_code& ec)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		_log(0, "PBU send error: ", ec.message());
}

void mag::mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pba_receiver_ptr& pbar, chrono& delay)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "PBA receive error: ", ec.message());

	} else {
		_service.dispatch(boost::bind(&mag::proxy_binding_ack, this, pbinfo, delay));
		pbar->async_receive(_mp_sock, boost::bind(&mag::mp_receive_handler, this, _1, _2, _3, _4));
	}
}

void mag::start_(const std::string& id, const ip_address& link_local_ip, bool tunnel_global_address)
{
	const router_node* node = _node_db.find_router(id);
	if (!node)
		boost::throw_exception(exception(errc::make_error_code(errc::invalid_argument),
		                                 "MAG id not found in node database"));

	if (!link_local_ip.is_link_local())
		boost::throw_exception(exception(errc::make_error_code(errc::invalid_argument),
		                                 "Invalid Link local address"));

	_log(0, "Started [id = ", id, ", link_local_ip = ", link_local_ip, ", address = ", node->address(), "]");

	_mp_sock.open(ip::mproto());
	_mp_sock.bind(ip::mproto::endpoint(node->address()));

	_identifier = id;
	_link_local_ip = link_local_ip;

	_tunnels.open(ip::address_v6(node->address().to_bytes(), node->device_id()), tunnel_global_address);

	_addrconf.start();

	for (size_t i = 0; i < _concurrency; ++i) {
		pba_receiver_ptr pbar(new pba_receiver);

		pbar->async_receive(_mp_sock, boost::bind(&mag::mp_receive_handler, this, _1, _2, _3, _4));
	}
}

void mag::stop_()
{
	_bulist.clear();
	_addrconf.clear();
	_addrconf.stop();
	_mp_sock.close();
	_route_table.clear();
	_tunnels.close();
}

void mag::mobile_node_attach_(const attach_info& ai, completion_functor& completion_handler)
{
	chrono delay;

	delay.start();

	bulist_entry* be = _bulist.find(ai.mn_id);
	if (!be) {
		const mobile_node* mn = _node_db.find_mobile_node(ai.mn_id);
		if (!mn) {
			report_completion(_service, completion_handler, boost::system::error_code(ec_not_authorized, mag_error_category()));
			_log(0, "Mobile Node attach error: not authorized [id = ", ai.mn_id, " (", ai.mn_address, ")]");
			return;
		}

		const router_node* lma = _node_db.find_router(mn->lma_id());
		if (!lma) {
			report_completion(_service, completion_handler, boost::system::error_code(ec_unknown_lma, mag_error_category()));
			_log(0, "Mobile Node attach error: unknown LMA [id = ", mn->id(), " (", ai.mn_address, "), lma = ", mn->lma_id(), "]");
			return;
		}

		be = new bulist_entry(_service.get_io_service(), mn->id(),
		                      ai.mn_address, mn->prefix_list(), mn->home_address(),
		                      lma->address(), ai.poa_dev_id, ai.poa_address);

		_bulist.insert(be);
		_log(0, "Mobile Node attach [id = ", mn->id(), " (", ai.mn_address, ")"
		                          ", lma = ", mn->lma_id(), " (", be->lma_address(), ")]");
	} else {
		_log(0, "Mobile Node re-attach [id = ", be->mn_id(), " (", ai.mn_address, "), lma = ", be->lma_address(), "]");
	}

	if (be->bind_status == bulist_entry::k_bind_error) {
		report_completion(_service, completion_handler, boost::system::error_code(ec_invalid_state, mag_error_category()));
		_log(0, "Mobile Node attach error: previous bind failed [id = ", be->mn_id(), "(", ai.mn_address, ")",
				                                               " (previous = ", be->mn_id(), "), lma = ", be->lma_address(), "]");
		return;
	}

	proxy_binding_info pbinfo;

	be->handover_delay.start(); //begin chrono handover delay
	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = ++be->sequence_number;
	pbinfo.lifetime = be->lifetime;
	pbinfo.prefix_list = be->mn_prefix_list();
	pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_requested;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::seconds(1.5)); //FIXME: set a proper timer
	be->timer.async_wait(_service.wrap(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo)));

	report_completion(_service, be->completion, boost::system::error_code(ec_canceled, mag_error_category()));
	std::swap(be->completion, completion_handler);

	delay.stop();
	_log(0, "PBU register send process delay ", delay.get());
}

void mag::mobile_node_detach_(const attach_info& ai, completion_functor& completion_handler)
{
	chrono delay;

	delay.start();

	const mobile_node* mn = _node_db.find_mobile_node(ai.mn_id);
	if (!mn) {
		report_completion(_service, completion_handler, boost::system::error_code(ec_not_authorized, mag_error_category()));
		_log(0, "Mobile Node detach error: not authorized [id = ", ai.mn_id, "]");
		return;
	}

	bulist_entry* be = _bulist.find(mn->id());
	if (!be || (be->bind_status != bulist_entry::k_bind_requested && be->bind_status != bulist_entry::k_bind_ack)) {
		report_completion(_service, completion_handler, boost::system::error_code(ec_invalid_state, mag_error_category()));
		_log(0, "Mobile Node detach error: not attached [id = ", mn->id(), " (", ai.mn_address, ")", "]");
		return;
	}
	_log(0, "Mobile Node detach [id = ", mn->id(), " (", ai.mn_address, "), lma = ", be->lma_address(), "]");

	if (be->bind_status == bulist_entry::k_bind_ack)
		del_route_entries(*be);

	proxy_binding_info pbinfo;

	be->handover_delay.start(); //begin chrono handover delay
	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = ++be->sequence_number;
	pbinfo.lifetime = 0;
	pbinfo.prefix_list = be->mn_prefix_list();
	pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_detach;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::milliseconds(1500));
	be->timer.async_wait(_service.wrap(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo)));

	report_completion(_service, be->completion, boost::system::error_code(ec_canceled, mag_error_category()));
	std::swap(be->completion, completion_handler);

	delay.stop();
	_log(0, "PBU de-register send process delay ", delay.get());
}

void mag::proxy_binding_ack(const proxy_binding_info& pbinfo, chrono& delay)
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

	if (pbinfo.status == ip::mproto::pba::status_bad_sequence) {
		_log(0, "PBA error: bad sequence number [id = ", pbinfo.id,
		                                      ", lma = ", pbinfo.address,
		                                      ", sequence = ", be->sequence_number,
		                                      ", last accepted sequence = ", pbinfo.sequence, "]");

		be->sequence_number = pbinfo.sequence;

		proxy_binding_info pbinfo;

		pbinfo.id = be->mn_id();
		pbinfo.address = be->lma_address();
		pbinfo.handoff = ip::mproto::option::handoff::k_unknown;
		pbinfo.sequence = ++be->sequence_number;
		pbinfo.lifetime = (be->bind_status != bulist_entry::k_bind_detach) ? be->lifetime : 0;
		pbinfo.prefix_list = be->mn_prefix_list();

		pbu_sender_ptr pbus(new pbu_sender(pbinfo));

		pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
		be->timer.cancel();
		be->timer.expires_from_now(boost::posix_time::milliseconds(1500));
		be->timer.async_wait(_service.wrap(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo)));

		return;
	}

	if (pbinfo.sequence != be->sequence_number) {
		_log(0, "PBA error: sequence number invalid [id = ", pbinfo.id,
		                                          ", lma = ", pbinfo.address,
		                                          ", sequence = ", pbinfo.sequence,
		                                                   " != ", be->sequence_number, "]");
		return;
	}

	if (pbinfo.lifetime && (be->bind_status == bulist_entry::k_bind_requested
		                    || be->bind_status == bulist_entry::k_bind_renewing)) {

		boost::system::error_code ec;

		if (pbinfo.status == ip::mproto::pba::status_ok) {
			if (be->bind_status == bulist_entry::k_bind_requested)
				add_route_entries(*be);
		} else {
			ec = boost::system::error_code(pbinfo.status, pba_error_category());
		}

		be->timer.cancel();
		be->handover_delay.stop();

		if (be->bind_status == bulist_entry::k_bind_requested) {
			report_completion(_service, be->completion, ec);
			_log(0, "PBA registration [delay = ", be->handover_delay.get(),
			                        ", id = ", pbinfo.id,
			                        ", lma = ", pbinfo.address,
			                        ", status = ", pbinfo.status, "]");
		} else {
			_log(0, "PBA re-registration [delay = ", be->handover_delay.get(),
			                           ", id = ", pbinfo.id,
			                           ", lma = ", pbinfo.address,
			                           ", status = ", pbinfo.status, "]");
		}


		be->bind_status = bulist_entry::k_bind_ack;

		if (pbinfo.status == ip::mproto::pba::status_ok) {
			//Will try to renew 3 seconds before binding expires or 1 second if lifetime <= 6
			uint expire = (pbinfo.lifetime <= 6) ? pbinfo.lifetime - 1 : pbinfo.lifetime - 3; //FIXME Check used values

			be->timer.expires_from_now(boost::posix_time::seconds(expire));
			be->timer.async_wait(_service.wrap(boost::bind(&mag::proxy_binding_renew, this, _1, pbinfo.id)));
		} else {
			_bulist.remove(be);
		}

		delay.stop();
		_log(0, "PBA register process delay ", delay.get());

	} else 	if (!pbinfo.lifetime && be->bind_status == bulist_entry::k_bind_detach) {
		boost::system::error_code ec;

		if (pbinfo.status == ip::mproto::pba::status_ok)
			ec = boost::system::error_code(pbinfo.status, pba_error_category());

		be->timer.cancel();
		be->handover_delay.stop();

		report_completion(_service, be->completion, ec);
		_log(0, "PBA de-registration [delay = ", be->handover_delay.get(),
		                           ", id = ", pbinfo.id,
		                           ", lma = ", pbinfo.address, "]");

		_bulist.remove(be);

		delay.stop();
		_log(0, "PBA de-register process delay ", delay.get());

	} else {
		_log(0, "PBA ignored [id = ", pbinfo.id, ", lma = ", pbinfo.address, ", status = ", be->bind_status, "]");
	}
}

void mag::proxy_binding_retry(const boost::system::error_code& ec, proxy_binding_info& pbinfo)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "PBU retry timer error: ", ec.message());

		return;
	}

	bulist_entry* be = _bulist.find(pbinfo.id);
	if (!be || (be->bind_status != bulist_entry::k_bind_requested
		        && be->bind_status != bulist_entry::k_bind_renewing
			    && be->bind_status != bulist_entry::k_bind_detach)) {
		_log(0, "PBU retry error: binding update list entry not found [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		return;
	}

	++be->retry_count;

	if (be->bind_status == bulist_entry::k_bind_detach && be->retry_count > 3) {
		report_completion(_service, be->completion, boost::system::error_code(ec_timeout, mag_error_category()));
		_log(0, "PBU retry error: max retry count [id = ", pbinfo.id, ", lma = ", pbinfo.address, "]");
		_bulist.remove(be);
		return;
	}

	pbu_sender_ptr pbus(new pbu_sender(pbinfo));
	double         delay = std::min<double>(32, std::pow(1.5f, be->retry_count)); //FIXME: validate

	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::milliseconds(delay * 1000.f));
	be->timer.async_wait(_service.wrap(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo)));

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

void mag::proxy_binding_renew(const boost::system::error_code& ec, const std::string& id)
{
	if (ec) {
		 if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "PBU renew timer error: ", ec.message());

		return;
	}

	bulist_entry* be = _bulist.find(id);
	if (!be) {
		_log(0, "PBU renew timer error: binding update list entry not found [id = ", id, "]");
		return;
	}

	proxy_binding_info pbinfo;

	be->handover_delay.start(); //begin chrono handover delay
	pbinfo.id = be->mn_id();
	pbinfo.address = be->lma_address();
	pbinfo.sequence = ++be->sequence_number;
	pbinfo.lifetime = be->lifetime;
	pbinfo.prefix_list = be->mn_prefix_list();
	pbinfo.handoff = ip::mproto::option::handoff::k_not_changed;
	pbu_sender_ptr pbus(new pbu_sender(pbinfo));

	be->bind_status = bulist_entry::k_bind_renewing;
	be->retry_count = 0;
	pbus->async_send(_mp_sock, boost::bind(&mag::mp_send_handler, this, _1));
	be->timer.expires_from_now(boost::posix_time::milliseconds(1500));
	be->timer.async_wait(_service.wrap(boost::bind(&mag::proxy_binding_retry, this, _1, pbinfo)));
}

void mag::add_route_entries(bulist_entry& be)
{
	chrono delay;

	delay.start();

	const bulist::ip_prefix_list& npl = be.mn_prefix_list();
	uint adev = be.poa_dev_id();
	uint tdev = _tunnels.get(be.lma_address());

	_log(0, "Add route entries [id = ", be.mn_id(), ", tunnel = ", tdev, ", LMA = ", be.lma_address(), "]");

	for (bulist::ip_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_dst(*i, adev);

	for (bulist::ip_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_src(*i, tdev);

	router_advertisement_info rainfo;

	rainfo.device_id = be.poa_dev_id();
	rainfo.link_address = be.poa_address();
	rainfo.dst_link_address = be.mn_link_address();
	rainfo.mtu = be.mtu;
	rainfo.prefix_list = be.mn_prefix_list();
	rainfo.home_addr = be.home_address();
	rainfo.source = _link_local_ip;
	rainfo.destination = ip::address_v6::from_string("ff02::1");

	_addrconf.add(rainfo);

	delay.stop();
	_log(0, "Add route entries delay ", delay.get());
}

void mag::del_route_entries(bulist_entry& be)
{
	chrono delay;

	delay.start();

	const bulist::ip_prefix_list& npl = be.mn_prefix_list();

	_log(0, "Remove route entries [id = ", be.mn_id(), ", LMA = ", be.lma_address(), "]");

	for (bulist::ip_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_dst(*i);

	for (bulist::ip_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_src(*i);

	_tunnels.del(be.lma_address());
	_addrconf.del(be.mn_link_address());

	delay.stop();
	_log(0, "Remove route entries delay ", delay.get());
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
