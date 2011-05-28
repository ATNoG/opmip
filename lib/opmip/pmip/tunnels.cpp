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
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
ip6_tunnels::ip6_tunnels(boost::asio::io_service& ios)
	: _io_service(ios), _gc(0)
{
}

ip6_tunnels::~ip6_tunnels()
{
}

void ip6_tunnels::open(const ip::address_v6& address)
{
	if (!_tunnels.empty()) {
		_gc.clear();
		_tunnels.clear();
	}
	_local = address;
}

void ip6_tunnels::close()
{
	_local = ip::address_v6();
	_gc.clear();
	_tunnels.clear();
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
	if (i != _tunnels.end() && i->second->refcount) {
		if (!--i->second->refcount)
			_gc.push_back(i);
	}

	if (_gc.size() >= k_gc_threshold) {
		std::for_each(_gc.begin(), _gc.end(), [this](map::iterator& i) {
			if (!i->second->refcount)
				_tunnels.erase(i);
		});
		_gc.clear();
	}
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
