//=============================================================================
// Brief   : Protocol Independent Multicast
// Authors : Bruno Santos <bsantos@av.it.pt>
// 		   : Sérgio Figueiredo <sfigueiredo@av.it.pt>
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

#ifndef OPMIP_NET_IP_PIM__HPP_
#define OPMIP_NET_IP_PIM__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/ip/address.hpp>
#include <boost/asio/basic_raw_socket.hpp>
#include <boost/type_traits/is_base_of.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {	namespace net {	namespace ip {

///////////////////////////////////////////////////////////////////////////////
struct pim {
	typedef boost::asio::basic_raw_socket<pim> socket;
	class endpoint;
	struct header;
	struct hello;
	struct register_;
	struct register_stop;
	struct join_prune;
//	struct bootstrap;
//	struct assert;
//	struct graft;		// PIM-DM only
//	struct graft_ack;	// PIM-DM only

	int family() const   { return AF_INET6; }
	int type() const     { return SOCK_RAW; }
	int protocol() const { return 103; }
};

///////////////////////////////////////////////////////////////////////////////
class pim::endpoint {
public:
	typedef pim protocol_type;

public:
	endpoint();
	endpoint(const address_v6& addr);

	protocol_type     protocol() const { return protocol_type(); }
	::sockaddr*       data()           { return reinterpret_cast< ::sockaddr*>(&_addr); }
	const ::sockaddr* data() const     { return reinterpret_cast<const ::sockaddr*>(&_addr); }
	size_t            size() const     { return sizeof(_addr); }
	size_t            capacity() const { return sizeof(_addr); }
	address_v6        address() const;

	void resize(size_t len) { BOOST_ASSERT(len == sizeof(_addr)); }
	void address(const address_v6& addr);

private:
	::sockaddr_in6 _addr;
};

inline pim::endpoint::endpoint()
{
	::in6_addr tmp = IN6ADDR_ANY_INIT;

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = 0;
	_addr.sin6_flowinfo = 0;
	_addr.sin6_addr = tmp;
	_addr.sin6_scope_id = 0;
}

inline pim::endpoint::endpoint(const address_v6& addr)
{
	const address_v6::bytes_type& src = addr.to_bytes();

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = 0;
	_addr.sin6_flowinfo = 0;
	std::copy(src.begin(), src.end(), _addr.sin6_addr.s6_addr);
	_addr.sin6_scope_id = addr.scope_id();
}

inline address_v6 pim::endpoint::address() const
{
	address_v6::bytes_type tmp;

	std::copy(_addr.sin6_addr.s6_addr, _addr.sin6_addr.s6_addr + address_v6::bytes_type::static_size,
	          tmp.elems);

	return address_v6(tmp, _addr.sin6_scope_id);
}

inline void pim::endpoint::address(const address_v6& addr)
{
	const address_v6::bytes_type& src = addr.to_bytes();

	std::copy(src.begin(), src.end(), _addr.sin6_addr.s6_addr);
	_addr.sin6_scope_id = addr.scope_id();
}

///////////////////////////////////////////////////////////////////////////////
// TODO template for casting header, not packet

struct pim::header {
	static header* cast(void* buffer, size_t length)
	{
		header* hdr = reinterpret_cast<header*>(buffer);

		if (length >= sizeof(header) && hdr->version == 2)
			return hdr;

		return nullptr;
	}

	template<class T>
	static T* cast(void* buffer, size_t length)
	{
		OPMIP_STATIC_ASSERT((boost::is_base_of<header, T>::value), "T must be a base of opmip::net::ip::pim::header");
		header* hdr = reinterpret_cast<header*>(buffer);

		if (length >= sizeof(T) && hdr->version == 2 && hdr->type == T::type_value)
			return static_cast<T*>(hdr);

		return nullptr;
	}

	header(uint8 tp)
		: type(tp), version(2), reserved(0), checksum(0)
	{ }

	uint8  type : 4;
	uint8  version : 4;
	uint8  reserved;
	uint16 checksum;
};

///////////////////////////////////////////////////////////////////////////////
struct enc_unicast {
	uint8 family;
	uint8 type;
	address_v6::bytes_type address;
};

///////////////////////////////////////////////////////////////////////////////
struct enc_group {
	void B(bool val) {	// Bidirectional PIM
		if (val)
			flags |= 0x80;
		else
			flags &= ~(0x80);
	}

	void Z(bool val) {	// Admin Scope Zone
		if (val)
			flags |= 0x1;
		else
			flags &= ~(0x1);
	}

	uint8  family;
	uint8  type;
	uint8  flags;
	uint8  mask_len;
	address_v6::bytes_type address;
};

///////////////////////////////////////////////////////////////////////////////
struct enc_source {
	void S(bool val) {
		if (val)
			flags |= 0x4;
		else
			flags &= ~(0x4);
	}
	void W(bool val) {
		if (val)
			flags |= 0x2;
		else
			flags &= ~(0x2);
	}
	void R(bool val) {
		if (val)
			flags |= 0x1;
		else
			flags &= ~(0x1);
	}

	uint8 family;
	uint8 type;
	uint8 flags;
	uint8 len;
	address_v6::bytes_type address;
};

///////////////////////////////////////////////////////////////////////////////
struct pim::hello : header	{
	static const uint8 type_value = 0;

	hello()
		: header(type_value)
	{}

	struct option {
		uint16 type;
		uint16 length;

		option(uint16 tp, uint16 len)
			: type(htons(tp)), length(htons(len))
		{}
	};

	static option* option_next(option* opt, size_t& rem_len)
	{
		const size_t off = align_to<2>(opt->length);

		opt = offset_cast<option*>(opt, off);
		if (sizeof(option) > rem_len || (sizeof(option) + opt->length) > rem_len)
			return nullptr;

		rem_len -= sizeof(option) + opt->length;
		return opt;
	};

	template<class T>
	static T* option_cast(option* opt)
	{
		OPMIP_STATIC_ASSERT((boost::is_base_of<option, T>::value), "T must be a base of opmip::net::ip::pim::hello::option");
		if (opt->type != T::type_value || opt->length < sizeof(T))
			return nullptr;
		return static_cast<T*>(opt);
	}

	struct holdtime : option {
		static const uint16 type_value = 1;

		holdtime()
			: option(type_value, 2)
		{}

		uint16 value;
	};

	struct dr_priority : option {
		static const uint16 type_value = 19;

		dr_priority()
			: option(type_value, 4)
		{}

		uint32 value;
	};

	struct generation_id : option {
		static const uint16 type_value = 20;

		generation_id()
			: option(type_value, 4)
		{}

		uint32 value;
	};

	struct address_list : option {
		static const uint8 type_value = 24;

		address_list(uint count)
			: option(type_value, count * sizeof(enc_unicast))		 // TODO what value to use for length?
		{}
		enc_unicast entries[];
	};

	option options[];
};

///////////////////////////////////////////////////////////////////////////////
/*
struct pim::register_ : header	{
	static const uint8 type_value = 1;

	register_()
		: header(type_value), flags(0)
	{}


	void B(bool val) {
		if (val)
			flags |= 0x8000;
		else
			flags &= 0x8000;
	}

	void N(bool val) {
		if (val)
			flags |= 0x4000;
		else
			flags &= 0x4000;
	}

	uint32 	flags;
	uint8	packet[];
};

///////////////////////////////////////////////////////////////////////////////
struct pim::register_stop : header	{
public:
	static const uint8 type_value = 2;

	register_stop()
		: header(type_value)
	{}

	enc_group group;
	enc_unicast source;

};
*/
///////////////////////////////////////////////////////////////////////////////
struct pim::join_prune : header	{
public:
	static const uint8 type_value = 3;

	join_prune()
		: header(type_value)
	{}

	struct mcast_group {
		enc_group  group;
		uint16     count_joins;
		uint16     count_prunes;
		enc_source sources[];
	};

	enc_unicast uplink_neigh;
	uint8       reserved;
	uint8       num_groups;
	uint16      holdtime;
	mcast_group mcast_groups[];
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_NET_IP_PIM__HPP_ */
