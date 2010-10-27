//=============================================================================
// Brief   : Route Table
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

#include <opmip/sys/route_table.hpp>
#include <opmip/sys/netlink/message.hpp>
#include <opmip/sys/netlink/utils.hpp>
#include <opmip/sys/rtnetlink/route.hpp>
#include <opmip/sys/error.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
route_table::route_table(boost::asio::io_service& ios)
	: _rtnl(ios), _rtnl_seq(0)
{
	_rtnl.open(netlink<0>());
	_rtnl.bind(netlink<0>::endpoint());
}

route_table::~route_table()
{
	clear();
}

std::pair<route_table::const_iterator, bool> route_table::add_by_src(const ip::prefix_v6& prefix, uint device, const ip::address_v6& gateway)
{
	std::pair<iterator, bool> res = _map_by_src.insert(map::value_type(prefix, entry(device, gateway)));

	if (!res.second)
		return res;


	boost::system::error_code ec;

	add_by_src(res.first, ec);
	if (ec) {
		_map_by_src.erase(res.first);
		boost::throw_exception(boost::system::system_error(ec, "opmip::sys::route_table::add_by_src"));

		return std::pair<const_iterator, bool>(_map_by_src.end(), false);
	}

	return res;
}

std::pair<route_table::const_iterator, bool> route_table::find_by_src(const ip::prefix_v6& prefix) const
{
	std::pair<const_iterator, bool> res(_map_by_src.find(prefix), true);

	if (res.first == _map_by_src.cend())
		res.second = false;

	return res;
}

bool route_table::remove_by_src(const ip::prefix_v6& prefix)
{
	iterator res = _map_by_src.find(prefix);

	if (res == _map_by_src.end())
		return false;


	boost::system::error_code ec;

	remove_by_src(res, ec);
	_map_by_src.erase(res);
	throw_on_error(ec, "opmip::sys::route_table::remove_by_src");

	return true;
}

std::pair<route_table::const_iterator, bool> route_table::add_by_dst(const ip::prefix_v6& prefix, uint device, const ip::address_v6& gateway)
{
	std::pair<iterator, bool> res = _map_by_dst.insert(map::value_type(prefix, entry(device, gateway)));

	if (!res.second)
		return res;


	boost::system::error_code ec;

	add_by_dst(res.first, ec);
	if (ec) {
		_map_by_dst.erase(res.first);
		boost::throw_exception(boost::system::system_error(ec, "opmip::sys::route_table::add_by_dst"));

		return std::pair<const_iterator, bool>(_map_by_dst.end(), false);
	}

	return res;
}

std::pair<route_table::const_iterator, bool> route_table::find_by_dst(const ip::prefix_v6& prefix) const
{
	std::pair<const_iterator, bool> res(_map_by_dst.find(prefix), true);

	if (res.first == _map_by_dst.cend())
		res.second = false;

	return res;
}

bool route_table::remove_by_dst(const ip::prefix_v6& prefix)
{
	iterator res = _map_by_dst.find(prefix);

	if (res == _map_by_dst.end())
		return false;


	boost::system::error_code ec;

	remove_by_dst(res, ec);
	_map_by_dst.erase(res);
	throw_on_error(ec, "opmip::sys::route_table::remove_by_dst");

	return true;
}

void route_table::clear()
{
	boost::system::error_code ec;

	for (iterator i = _map_by_src.begin(), e = _map_by_src.end(); i != e; ++i)
		remove_by_src(i, ec);

	for (iterator i = _map_by_dst.begin(), e = _map_by_dst.end(); i != e; ++i)
		remove_by_dst(i, ec);
}

void route_table::add_by_src(iterator& entry, boost::system::error_code& ec)
{
	BOOST_ASSERT(entry->first.length() <= 128);

	nl::message<rtnl::route> rtmsg;

	rtmsg.mtype(rtnl::route::m_new);
	rtmsg.flags(nl::header::request | nl::header::ack | nl::header::create | nl::header::replace);
	rtmsg.sequence(++_rtnl_seq);
	rtmsg->family = AF_INET6;
	rtmsg->src_len = entry->first.length();
	rtmsg->table = rtnl::route::table_main;
	rtmsg->protocol = rtnl::route::proto_static;
	rtmsg->type = rtnl::route::r_unicast;

	if (!entry->second.gateway.is_unspecified()) {
		ip::prefix_v6::bytes_type gw = entry->second.gateway.to_bytes();
		rtmsg.push_attribute(rtnl::route::attr_gateway, gw.elems, gw.size());
	}

	ip::prefix_v6::bytes_type src = entry->first.bytes();
	rtmsg.push_attribute(rtnl::route::attr_source, src.elems, src.size());

	uint32 dev = entry->second.device;
	rtmsg.push_attribute(rtnl::route::attr_output_device, &dev, sizeof(dev));

	nl::checked_send(_rtnl, rtmsg.cbuffer(), ec);
	if (ec == boost::system::errc::make_error_condition(boost::system::errc::file_exists))
		ec = boost::system::error_code();
}

void route_table::remove_by_src(iterator& entry, boost::system::error_code& ec)
{
	BOOST_ASSERT(entry->first.length() <= 128);

	nl::message<rtnl::route> rtmsg;

	rtmsg.mtype(rtnl::route::m_del);
	rtmsg.flags(nl::header::request | nl::header::ack);
	rtmsg.sequence(++_rtnl_seq);
	rtmsg->family = AF_INET6;
	rtmsg->src_len = entry->first.length();
	rtmsg->table = rtnl::route::table_main;

	if (!entry->second.gateway.is_unspecified()) {
		ip::prefix_v6::bytes_type gw = entry->second.gateway.to_bytes();
		rtmsg.push_attribute(rtnl::route::attr_gateway, gw.elems, gw.size());
	}

	ip::prefix_v6::bytes_type src = entry->first.bytes();
	rtmsg.push_attribute(rtnl::route::attr_source, src.elems, src.size());

	uint32 dev = entry->second.device;
	rtmsg.push_attribute(rtnl::route::attr_output_device, &dev, sizeof(dev));

	nl::checked_send(_rtnl, rtmsg.cbuffer(), ec);
}

void route_table::add_by_dst(iterator& entry, boost::system::error_code& ec)
{
	BOOST_ASSERT(entry->first.length() <= 128);

	nl::message<rtnl::route> rtmsg;

	rtmsg.mtype(rtnl::route::m_new);
	rtmsg.flags(nl::header::request | nl::header::ack | nl::header::create | nl::header::replace);
	rtmsg.sequence(++_rtnl_seq);
	rtmsg->family = AF_INET6;
	rtmsg->dst_len = entry->first.length();
	rtmsg->table = rtnl::route::table_main;
	rtmsg->protocol = rtnl::route::proto_static;
	rtmsg->type = rtnl::route::r_unicast;

	if (!entry->second.gateway.is_unspecified()) {
		ip::prefix_v6::bytes_type gw = entry->second.gateway.to_bytes();
		rtmsg.push_attribute(rtnl::route::attr_gateway, gw.elems, gw.size());
	}

	ip::prefix_v6::bytes_type dst = entry->first.bytes();
	rtmsg.push_attribute(rtnl::route::attr_destination, dst.elems, dst.size());

	uint32 dev = entry->second.device;
	rtmsg.push_attribute(rtnl::route::attr_output_device, &dev, sizeof(dev));

	nl::checked_send(_rtnl, rtmsg.cbuffer(), ec);
	if (ec == boost::system::errc::make_error_condition(boost::system::errc::file_exists))
		ec = boost::system::error_code();
}

void route_table::remove_by_dst(iterator& entry, boost::system::error_code& ec)
{
	BOOST_ASSERT(entry->first.length() <= 128);

	nl::message<rtnl::route> rtmsg;

	rtmsg.mtype(rtnl::route::m_del);
	rtmsg.flags(nl::header::request | nl::header::ack);
	rtmsg.sequence(++_rtnl_seq);
	rtmsg->family = AF_INET6;
	rtmsg->dst_len = entry->first.length();
	rtmsg->table = rtnl::route::table_main;

	if (!entry->second.gateway.is_unspecified()) {
		ip::prefix_v6::bytes_type gw = entry->second.gateway.to_bytes();
		rtmsg.push_attribute(rtnl::route::attr_gateway, gw.elems, gw.size());
	}

	ip::prefix_v6::bytes_type dst = entry->first.bytes();
	rtmsg.push_attribute(rtnl::route::attr_destination, dst.elems, dst.size());

	uint32 dev = entry->second.device;
	rtmsg.push_attribute(rtnl::route::attr_output_device, &dev, sizeof(dev));

	nl::checked_send(_rtnl, rtmsg.cbuffer(), ec);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
