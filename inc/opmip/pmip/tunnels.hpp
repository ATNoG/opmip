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

#ifndef OPMIP_PMIP_TUNNELS__HPP_
#define OPMIP_PMIP_TUNNELS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/sys/ip6_tunnel.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <set>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class ip6_tunnels {
	struct entry {
		entry(boost::asio::io_service& ios)
			: tunnel(ios), refcount(1)
		{ }

		sys::ip6_tunnel tunnel;
		uint            refcount;
	};

	typedef boost::ptr_map<ip::address_v6, entry> map;
	typedef std::set<ip::address_v6>              map_gc;

	static const uint k_gc_threshold = 128;

public:
	ip6_tunnels(boost::asio::io_service& ios);
	~ip6_tunnels();

	void open(const ip::address_v6& address);
	void close();

	uint get(const ip::address_v6& remote);
	void del(const ip::address_v6& remote);

	const ip::address_v6& get_local_address() const { return _local; }

private:
	boost::asio::io_service& _io_service;
	ip::address_v6           _local;
	map                      _tunnels;
	map_gc                   _gc;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_TUNNELS__HPP_ */
