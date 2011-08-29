//=============================================================================
// Brief   : Local Mobility Anchor
// Authors : Bruno Santos <bsantos@av.it.pt>
// Authors : Filipe Manco <filipe.manco@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2011 Universidade de Aveiro
// Copyrigth (C) 2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#include <opmip/pmip/lma.hpp>
#include <opmip/pmip/mp_sender.hpp>
#include <opmip/exception.hpp>
#include <boost/bind.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
bool validate_sequence_number(uint16 prev, uint16 current)
{
	return prev < 32768 ?
		((current > prev) && (current < ((uint16)(prev + 32768))))
		:
		((current > prev) || (current < ((uint16)(prev + 32768))))
		;
}

///////////////////////////////////////////////////////////////////////////////
lma::lma(boost::asio::io_service& ios, node_db& ndb, size_t concurrency)
	: _service(ios), _node_db(ndb), _log("LMA", std::cout), _mp_sock(ios),
	  _tunnels(ios), _route_table(ios), _concurrency(concurrency)
{
}

void lma::start(const std::string& id)
{
	_service.dispatch(boost::bind(&lma::start_, this, id));
}

void lma::stop()
{
	_service.dispatch(boost::bind(&lma::stop_, this));
}

void lma::mp_send_handler(const boost::system::error_code& ec)
{
	if (ec && ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
		_log(0, "PBA sender error: ", ec.message());
}

void lma::mp_receive_handler(const boost::system::error_code& ec, const proxy_binding_info& pbinfo, pbu_receiver_ptr& pbur, chrono& delay)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "PBU receiver error: ", ec.message());
		return;
	}

	_service.dispatch(boost::bind(&lma::proxy_binding_update, this, pbinfo, delay));
	pbur->async_receive(_mp_sock, boost::bind(&lma::mp_receive_handler, this, _1, _2, _3, _4));
}

void lma::start_(const std::string& id)
{
	const router_node* node = _node_db.find_router(id);
	if (!node) {
		error_code ec(boost::system::errc::invalid_argument, boost::system::get_generic_category());

		throw_exception(exception(ec, "LMA id not found in node database"));
	}
	_log(0, "Started [id = ", id, ", address = ", node->address(), "]");

	_mp_sock.open(ip::mproto());
	_mp_sock.bind(ip::mproto::endpoint(node->address()));

	_identifier = id;

	_tunnels.open(ip::address_v6(node->address().to_bytes(), node->device_id()));

	for (size_t i = 0; i < _concurrency; ++i) {
		pbu_receiver_ptr pbur(new pbu_receiver());

		pbur->async_receive(_mp_sock, boost::bind(&lma::mp_receive_handler, this, _1, _2, _3, _4));
	}
}

void lma::stop_()
{
	_bcache.clear();
	_mp_sock.close();
	_route_table.clear();
	_tunnels.close();
}

void lma::proxy_binding_update(proxy_binding_info& pbinfo, chrono& delay)
{
	if (pbinfo.status != ip::mproto::pba::status_ok)
		return; //error

	pbu_process(pbinfo);

	pba_sender_ptr pbas(new pba_sender(pbinfo));

	pbas->async_send(_mp_sock, boost::bind(&lma::mp_send_handler, this, _1));

	delay.stop();
	_log(0, "PBU ", !pbinfo.lifetime ? "de-" : "", "register processing delay ", delay.get());
}

bcache_entry* lma::pbu_get_be(proxy_binding_info& pbinfo)
{
	BOOST_ASSERT((pbinfo.status == ip::mproto::pba::status_ok));

	bcache_entry* be = _bcache.find(pbinfo.id);
	if (be)
		return be;

	if (!_node_db.find_router(pbinfo.address)) {
		_log(0, "PBU registration error: MAG not authorized [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
		pbinfo.status = ip::mproto::pba::status_not_authorized_for_proxy_reg;
		return nullptr;
	}

	const mobile_node* mn = _node_db.find_mobile_node(pbinfo.id);
	if (!mn) {
		_log(0, "PBU registration error: unknown mobile node [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
		pbinfo.status = ip::mproto::pba::status_not_lma_for_this_mn;
		return nullptr;
	}

	if (mn->lma_id() != _identifier) {
		_log(0, "PBU registration error: not this LMA [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
		pbinfo.status = ip::mproto::pba::status_not_lma_for_this_mn;
		return nullptr;
	}

	if (!pbinfo.lifetime) {
		_log(0, "PBU de-registration error: binding cache entry not found [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
		return nullptr; //note: no error for this
	}

	be = new bcache_entry(_service.get_io_service(), pbinfo.id, mn->prefix_list());
	_bcache.insert(be);

	return be;
}

bool lma::pbu_mag_checkin(bcache_entry& be, proxy_binding_info& pbinfo)
{
	BOOST_ASSERT((pbinfo.status == ip::mproto::pba::status_ok));

	if (be.care_of_address != pbinfo.address) {
		const router_node* mag = _node_db.find_router(pbinfo.address);
		if (!mag) {
			_log(0, "PBU error: unknown MAG [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
			pbinfo.status = ip::mproto::pba::status_not_authorized_for_proxy_reg;
			return false;
		}
		if (!pbinfo.lifetime) {
			_log(0, "PBU de-registration error: not this MAG [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
			return false; //note: no error for this
		}

		if (!be.care_of_address.is_unspecified())
			del_route_entries(&be);
		be.care_of_address = pbinfo.address;
		be.lifetime = pbinfo.lifetime;
		be.sequence = pbinfo.sequence;
		be.link_type = pbinfo.link_type;
		be.bind_status = bcache_entry::k_bind_unknown;

	} else {
		if (!validate_sequence_number(be.sequence, pbinfo.sequence)) {
			_log(0, "PBU error: sequence not valid [id = ", pbinfo.id,
			                                     ", mag = ", pbinfo.address,
			                                     ", sequence = ", be.sequence, " <> ", pbinfo.sequence, "]");
			pbinfo.status = ip::mproto::pba::status_bad_sequence;
			pbinfo.sequence = be.sequence + 1;
			return false;
		}

		be.lifetime = pbinfo.lifetime;
		be.sequence = pbinfo.sequence;
	}

	return true;
}

void lma::pbu_process(proxy_binding_info& pbinfo)
{
	bcache_entry* be = pbu_get_be(pbinfo);
	if (!be)
		return;

	if (!pbu_mag_checkin(*be, pbinfo))
		return;

	if (pbinfo.lifetime) {
		if (be->care_of_address == pbinfo.address)
			if (be->bind_status == bcache_entry::k_bind_registered)
				_log(0, "PBU re-registration [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
			else
				_log(0, "PBU registration [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");
		else
			_log(0, "PBU handoff [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");

		be->timer.cancel();
		be->bind_status = bcache_entry::k_bind_registered;
		add_route_entries(be);

		be->timer.expires_from_now(boost::posix_time::seconds(pbinfo.lifetime));
		be->timer.async_wait(_service.wrap(boost::bind(&lma::expired_entry, this, _1, be->id())));
	}

	BOOST_ASSERT((be->bind_status != bcache_entry::k_bind_unknown));

	if (!pbinfo.lifetime && be->bind_status == bcache_entry::k_bind_registered) {
		_log(0, "PBU de-registration [id = ", pbinfo.id, ", mag = ", pbinfo.address, "]");

		be->timer.cancel();
		be->bind_status = bcache_entry::k_bind_deregistered;
		del_route_entries(be);
		be->care_of_address = ip::address_v6();

		be->timer.expires_from_now(boost::posix_time::milliseconds(_config.min_delay_before_BCE_delete));
		be->timer.async_wait(_service.wrap(boost::bind(&lma::remove_entry, this, _1, be->id())));
	}
}

void lma::expired_entry(const boost::system::error_code& ec, const std::string& mn_id)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "Binding cache expired entry timer error: ", ec.message());

		return;
	}

	bcache_entry* be = _bcache.find(mn_id);
	if (!be || be->bind_status != bcache_entry::k_bind_deregistered) {
		_log(0, "Binding cache expired entry error: not found [id = ", mn_id, "]");
		return;
	}
	_log(0, "Binding expired entry [id = ", mn_id, "]");

	be->timer.expires_from_now(boost::posix_time::milliseconds(_config.min_delay_before_BCE_delete));
	be->timer.async_wait(_service.wrap(boost::bind(&lma::remove_entry, this, _1, be->id())));
}

void lma::remove_entry(const boost::system::error_code& ec, const std::string& mn_id)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			_log(0, "Binding cache remove entry timer error: ", ec.message());

		return;
	}

	bcache_entry* be = _bcache.find(mn_id);
	if (!be || be->bind_status != bcache_entry::k_bind_deregistered) {
		_log(0, "Binding cache remove entry error: not found [id = ", mn_id, "]");
		return;
	}
	_log(0, "Binding cache remove entry [id = ", mn_id, "]");

	_bcache.remove(be);
}

void lma::add_route_entries(bcache_entry* be)
{
	chrono delay;

	delay.start();

	const bcache::net_prefix_list& npl = be->prefix_list();
	uint tdev = _tunnels.get(be->care_of_address);

	_log(0, "Add route entries [id = ", be->id(), ", tunnel = ", tdev, ", CoA = ", be->care_of_address, "]");

	for (bcache::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.add_by_dst(*i, tdev);

	delay.stop();
	_log(0, "Add route entries delay ", delay.get());
}

void lma::del_route_entries(bcache_entry* be)
{
	chrono delay;

	delay.start();

	const bcache::net_prefix_list& npl = be->prefix_list();

	_log(0, "Remove route entries [id = ", be->id(), ", CoA = ", be->care_of_address, "]");

	for (bcache::net_prefix_list::const_iterator i = npl.begin(), e = npl.end(); i != e; ++i)
		_route_table.remove_by_dst(*i);

	_tunnels.del(be->care_of_address);

	delay.stop();
	_log(0, "Remove route entries delay ", delay.get());
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
