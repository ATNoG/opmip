//=============================================================================
// Brief   : Tunnel Management
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

#include <opmip/pmip/tunnels.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
ip6_tunnels::ip6_tunnels(boost::asio::io_service& ios)
	: _io_service(ios)
{
}

ip6_tunnels::~ip6_tunnels()
{
}

uint ip6_tunnels::get(const ip::address_v6& remote)
{
	auto i = _tunnels.find(remote);
	if (i != _tunnels.end()) {
		++i->second->refcount;
		return i->second->tunnel.get_device_id();
	}

	std::auto_ptr<entry> tun(new entry(_io_service));
	auto res = _tunnels.insert(remote, tun);

	try {
		res.first->second->tunnel.open("", _local.scope_id(), _local, remote);
		res.first->second->tunnel.set_enable(true);

	} catch (...) {
		_tunnels.erase(res.first);
		throw;
	}

	return res.first->second->tunnel.get_device_id();
}

void ip6_tunnels::del(const ip::address_v6& remote)
{
	auto i = _tunnels.find(remote);
	if (i != _tunnels.end()) {
		if (!(--i->second->refcount))
			_tunnels.erase(i);
	}
}

void ip6_tunnels::clear()
{
	_tunnels.clear();
}

void ip6_tunnels::set_local_address(const ip::address_v6& address)
{
	BOOST_ASSERT(_tunnels.empty());

	_local = address;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
