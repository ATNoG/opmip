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

#ifndef OPMIP_ROUTE_TABLE__HPP_
#define OPMIP_ROUTE_TABLE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/prefix.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/sys/netlink.hpp>
#include <boost/system/error_code.hpp>
#include <map>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
class route_table : boost::noncopyable {
public:
	struct entry {
		entry()
			: device(0)
		{ }

		entry(uint device_, ip::address_v6 gateway_)
			: device(device_), gateway(gateway_)
		{ }


		uint           device;
		ip::address_v6 gateway;
	};

private:
	typedef std::map<ip::prefix_v6, entry> map;
	typedef map::iterator                  iterator;

public:
	typedef map::const_iterator const_iterator;

public:
	route_table(boost::asio::io_service& ios);
	~route_table();

	std::pair<const_iterator, bool> add_by_src(const ip::prefix_v6& prefix, uint device, const ip::address_v6& gateway = ip::address_v6());
	std::pair<const_iterator, bool> find_by_src(const ip::prefix_v6& prefix) const;
	bool                            remove_by_src(const ip::prefix_v6& prefix);

	std::pair<const_iterator, bool> add_by_dst(const ip::prefix_v6& prefix, uint device, const ip::address_v6& gateway = ip::address_v6());
	std::pair<const_iterator, bool> find_by_dst(const ip::prefix_v6& prefix) const;
	bool                            remove_by_dst(const ip::prefix_v6& prefix);


private:
	void add_by_src(iterator& entry, boost::system::error_code& ec);
	void remove_by_src(iterator& entry, boost::system::error_code& ec);

	void add_by_dst(iterator& entry, boost::system::error_code& ec);
	void remove_by_dst(iterator& entry, boost::system::error_code& ec);

private:
	std::map<ip::prefix_v6, entry> _map_by_src;
	std::map<ip::prefix_v6, entry> _map_by_dst;

	netlink<0>::socket _rtnl;
	uint               _rtnl_seq;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_ROUTE_TABLE__HPP_ */
