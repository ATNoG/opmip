//=============================================================================
// Brief   : RTNetlink
// Authors : Bruno Santos <bsantos@av.it.pt>
//
//
// Copyright (C) 2009 Universidade Aveiro - Instituto de Telecomunicacoes Polo Aveiro
//
// This file is part of ODTONE - Open Dot Twenty One.
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef LINK_SAP_LINUX_RTNETLINK__HPP
#define LINK_SAP_LINUX_RTNETLINK__HPP

///////////////////////////////////////////////////////////////////////////////
#include "netlink.hpp"
#include <opmip/ll/mac_address.hpp>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
class rtnetlink : public netlink {
public:
	template<class T>
	class attr {
		struct safe_bool_t {
			void true_() {}
		};
		typedef void (safe_bool_t::*safe_bool)();

	public:
		attr() : _data(opmip::nullptr), _len(0)
		{ }
		attr(const void* data, opmip::size_t len) : _data((const T*) data), _len(len)
		{ }

		const T* get() const  { return _data; }
		opmip::size_t   size() const { return _len; }

		operator safe_bool() const { return _data != opmip::nullptr ? &safe_bool_t::true_ : 0; }
		bool operator!() const     { return _data == opmip::nullptr; }

	private:
		const T* _data;
		opmip::size_t   _len;
	};

	class if_link : data {
	public:
		enum type {
			ethernet   = 1,
			ieee802_11 = 801,
		};

		enum wevent {
			we_unknown,
			we_attach,
			we_detach,
		};

		enum flags {
			up             = 0x00001,
			broadcast      = 0x00002,
			debug          = 0x00004,
			loopback       = 0x00008,
			point_to_point = 0x00010,
			no_trailers    = 0x00020,
			running        = 0x00040,
			no_arp         = 0x00080,
			promiscuous    = 0x00100,
			all_multicast  = 0x00200,
			master         = 0x00400,
			slave          = 0x00800,
			multicast      = 0x01000,
			portsel        = 0x02000,
			automedia      = 0x04000,
			dynamic        = 0x08000,
			lower_up       = 0x10000,
			dormant        = 0x20000,
			echo           = 0x40000,
		};

		static inline bool is(const message& msg)
		{
			opmip::uint m = msg.type();

			return m >= 16 && m <= 19;
		}

		if_link();
		if_link(message& msg);

		if_link& operator=(message& msg);

		opmip::ushort type() const  { return _type; }
		opmip::sint   index() const { return _index; }
		opmip::uint   flags() const { return _flags; }

		bool has_name() const       { return _name; }
		bool has_mtu() const        { return _mtu; }
		bool has_lnk_type() const   { return _link_type; }

		wevent                        wireless_event() const   { return _wevent; }
		const opmip::ll::mac_address& wireless_address() const { return _waddress; }

		std::string       name() const       { return std::string(_name.get(), _name.size()); }
		opmip::uint       mtu() const        { return *_mtu; }
		opmip::sint       link_type() const  { return *_link_type; }

	private:
		opmip::ushort _type;
		opmip::sint   _index;
		opmip::uint   _flags;
		attr<char>    _name;
		opmip::uint*  _mtu;
		opmip::sint*  _link_type;

		wevent                 _wevent;
		opmip::ll::mac_address _waddress;
	};

	enum subscription {
		link   = 1,
		notify = 2,
		neigh  = 4,
		tc     = 8,

		ipv4_ifaddr = 0x10,
		ipv4_mroute = 0x20,
		ipv4_route  = 0x40,
		ipv4_rule   = 0x80,

		ipv6_ifaddr = 0x100,
		ipv6_mroute = 0x200,
		ipv6_route  = 0x400,
		ipv6_ifinfo = 0x800,

		DECnet_ifaddr = 0x1000,
		DECnet_route  = 0x4000,

		ipv6_prefix = 0x20000,
	};

	rtnetlink(opmip::uint subscriptions = 0) : netlink(netlink::route, subscriptions)
	{ }

};

template<class T>
inline std::ostream& operator<<(std::ostream& os, const rtnetlink::attr<T>& a)
{
	const opmip::uchar* c = reinterpret_cast<const opmip::uchar*>(a.get());

	os << std::hex;
	for (opmip::size_t i = 0; i < a.size(); ++i)
		os << opmip::uint(c[i]);
	os << std::dec;

	return os;
}

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* LINK_SAP_LINUX_RTNETLINK__HPP */
