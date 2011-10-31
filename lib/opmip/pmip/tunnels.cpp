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
	: _io_service(ios), _global_address(false)
{
}

ip6_tunnels::~ip6_tunnels()
{
}

void ip6_tunnels::open(const ip::address_v6& address, bool global_address)
{
	if (!_tunnels.empty()) {
		_gc.clear();
		_tunnels.clear();
	}
	_local = address;
	_global_address = global_address;
}

void ip6_tunnels::close()
{
	_local = ip::address_v6();
	_gc.clear();
	_tunnels.clear();
}

uint ip6_tunnels::get(const ip::address_v6& remote)
{
	map::iterator i = _tunnels.find(remote);
	if (i != _tunnels.end()) {
		if (++i->second->refcount == 1)
			_gc.erase(remote);

		return i->second->tunnel.get_device_id();
	}

	std::auto_ptr<entry> tun(new entry(_io_service));
	std::pair<map::iterator, bool> res = _tunnels.insert(remote, tun);

	try {
		res.first->second->tunnel.open("", _local.scope_id(), _local, remote);
		res.first->second->tunnel.set_enable(true);
		if (_global_address == true)	
			res.first->second->tunnel.add_address(_local, 64); // TODO make this prefix configurable
		else {}
	} catch (...) {
		_tunnels.erase(res.first);
		throw;
	}

	return res.first->second->tunnel.get_device_id();
}

void ip6_tunnels::del(const ip::address_v6& remote)
{
	map::iterator i = _tunnels.find(remote);
	if (i != _tunnels.end() && i->second->refcount) {
		if (!--i->second->refcount)
			_gc.insert(i->first);
	}

	if (_gc.size() >= k_gc_threshold) {
		for (map_gc::iterator i = _gc.begin(), e = _gc.end(); i!= e; ++i) {
			map::iterator j = _tunnels.find(*i);
			if (j != _tunnels.end() && !j->second->refcount)
				_tunnels.erase(*i);
		}
		_gc.clear();
	}
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
