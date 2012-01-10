//=============================================================================
// Brief   : IPv6 ICMP Data Structures
// Authors : Bruno Santos <bsantos@av.it.pt>
//           Sérgio Figueiredo <sfigueiredo@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2012 Universidade de Aveiro
// Copyrigth (C) 2010-2012 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_IP_ICMP__HPP_
#define OPMIP_IP_ICMP__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <netinet/icmp6.h>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
struct icmp {
	class header;
	class router_solicitation;
	class router_advertisement;
	class neighbor_solicitation;
	class neighbor_advertisement;
	struct mld_report;
	struct mld_query;
	class filter;
};

///////////////////////////////////////////////////////////////////////////////
class icmp::header {
public:
	template<class T>
	static T* cast(void* buffer, size_t length)
	{
		OPMIP_STATIC_ASSERT((boost::is_base_of<header, T>::value), "T must be a base of opmip::ip::icmp::header");
		header* hdr = static_cast<header*>(buffer);

		if (length >= sizeof(T) && hdr->type() == T::type_value)
			return static_cast<T*>(hdr);

		return nullptr;
	}

public:
	header(uint8 type, uint8 code)
		: _type(type), _code(code), _checksum(0)
	{ }

	uint8 type() const { return _type; }
	uint8 code() const { return _code; }

	void checksum(uint16 csum) { _checksum = csum; }

protected:
	uint8  _type;
	uint8  _code;
	uint16 _checksum;
};

///////////////////////////////////////////////////////////////////////////////
class icmp::router_solicitation : public icmp::header {
public:
	static const uint8 type_value = 133;
	static const uint8 code_value = 0;

public:
	router_solicitation()
		: header(type_value, code_value), _reserved(0)
	{ }

private:
	uint32 _reserved;
};

///////////////////////////////////////////////////////////////////////////////
class icmp::router_advertisement : public icmp::header {
public:
	static const uint8 type_value = 134;
	static const uint8 code_value = 0;

public:
	router_advertisement()
		: header(type_value, code_value), _current_hop_limit(0),
		_flags(0), _lifetime(0), _reachable_time(0), _retrans_timer(0)
	{ }

	uint8  current_hop_limit() const { return _current_hop_limit; }
	bool   M() const                 { return _flags & 0x80; }
	bool   O() const                 { return _flags & 0x40; }
	uint16 lifetime() const          { return ntohs(_lifetime); }
	uint32 reachable_time() const    { return ntohl(_reachable_time); }
	uint32 retrans_timer() const     { return ntohl(_retrans_timer); }

	void current_hop_limit(uint8 val) { _current_hop_limit = val; }

	void M(bool val)
    {
		if (val)
			_flags |= 0x80;
		else
			_flags &= ~(0x80);
	}

	void O(bool val)
	{
		if (val)
			_flags |= 0x40;
		else
			_flags &= ~(0x40);
	}

	void lifetime(uint16 val)       { _lifetime = htons(val); }
	void reachable_time(uint32 val) { _reachable_time = htonl(val); }
	void retrans_timer(uint32 val)  { _retrans_timer = htonl(val); }

private:
	uint8  _current_hop_limit;
	uint8  _flags;
	uint16 _lifetime;
	uint32 _reachable_time;
	uint32 _retrans_timer;
};

///////////////////////////////////////////////////////////////////////////////
class icmp::neighbor_solicitation : public icmp::header {
public:
	static const uint8 type_value = 135;
	static const uint8 code_value = 0;

public:
	neighbor_solicitation()
		: header(type_value, code_value), _reserved(0)
	{
		_target_addr.fill(0);
	}

	void target(const address_v6::bytes_type& addr) { _target_addr = addr; }

	const address_v6::bytes_type& target() const { return _target_addr; }

private:
	uint32                 _reserved;
	address_v6::bytes_type _target_addr;
};

///////////////////////////////////////////////////////////////////////////////
class icmp::neighbor_advertisement : public icmp::header {
public:
	static const uint8 type_value = 136;
	static const uint8 code_value = 0;

public:
	neighbor_advertisement()
		: header(type_value, code_value), _flags(0), _reserved1(0),
		_reserved2(0)
	{
		_target_addr.fill(0);
	}

	void target(const address_v6::bytes_type& addr) { _target_addr = addr; }
	bool R() const                                  { return _flags & 0x80; }
	bool S() const                                  { return _flags & 0x40; }
	bool O() const                                  { return _flags & 0x20; }

	const address_v6::bytes_type& target() const { return _target_addr; }

	void R(bool val)
    {
		if (val)
			_flags |= 0x80;
		else
			_flags &= ~(0x80);
	}

	void S(bool val)
	{
		if (val)
			_flags |= 0x40;
		else
			_flags &= ~(0x40);
	}

	void O(bool val)
	{
		if (val)
			_flags |= 0x20;
		else
			_flags &= ~(0x20);
	}

private:
	uint8                  _flags;
	uint8                  _reserved1;
	uint16                 _reserved2;
	address_v6::bytes_type _target_addr;
};

///////////////////////////////////////////////////////////////////////////////
struct icmp::mld_report : icmp::header {
	static const uint8 type_value = 143;

	struct mcast_address {
		mcast_address* next()
		{
			size_t offset = align_to<4>(sizeof(*this) + aux_data_len + sizeof(address_v6::bytes_type) * source_count);

			return offset_cast<mcast_address*>(this, offset);
		}

		uint8                  type;
		uint8                  aux_data_len;
		uint16                 source_count;
		address_v6::bytes_type group;
		address_v6::bytes_type sources[];
	};

	uint16        reserved;
	uint16        count;
	mcast_address mcast_addresses[];
};

///////////////////////////////////////////////////////////////////////////////
struct icmp::mld_query : icmp::header {
	static const uint8 type_value = 130;
	static const uint8 code_value = 0;

	mld_query()
		: header(type_value, code_value)
	{ }

	uint16 max_response_code()
	{
		uint mat = max_resp_code & 0xfff;
		uint exp = (max_resp_code >> 12) & 0x7;
		return max_resp_code < 0x8000 ? max_resp_code : (mat | 0x1000) >> (exp + 3);
	}

	void S(bool val) // Suppress Routing-side processing
	{
		if (val)
			flags |= 1 << 3;
		else
			flags &= ~(1 << 3);
	}

	void qrv(uint8 val)
	{
		if (val < 0x7)
			flags |= 0x7;
		else
			flags &= ~(0x7);
	}

	uint8 qqi()
	{
		uint mat = qqic & 0xf;
		uint exp = (qqic >> 4) & 0x7;
		return qqic < 0x80 ? qqic : (mat | 0x10) >> (exp + 3);
	}


	uint16					max_resp_code;
	uint16					reserved;
	address_v6::bytes_type 	group;
	uint8					flags;
	uint8					qqic;
	uint16            		source_count;
	address_v6::bytes_type 	sources[];
} __attribute__((packed));

///////////////////////////////////////////////////////////////////////////////
class icmp::filter {
public:
	filter(bool block)
	{
		if (block) {
			ICMP6_FILTER_SETBLOCKALL(&_filter);
		} else {
			ICMP6_FILTER_SETPASSALL(&_filter);
		}
	}

	filter(bool block, uint except_value_type)
	{
		if (block) {
			ICMP6_FILTER_SETBLOCKALL(&_filter);
			ICMP6_FILTER_SETPASS(except_value_type, &_filter);
		} else {
			ICMP6_FILTER_SETPASSALL(&_filter);
			ICMP6_FILTER_SETBLOCK(except_value_type, &_filter);
		}
	}

	void pass()
	{
		ICMP6_FILTER_SETPASSALL(&_filter);
	}

	void pass(uint type_value)
	{
		ICMP6_FILTER_SETPASS(type_value, &_filter);
	}

	void block()
	{
		ICMP6_FILTER_SETBLOCKALL(&_filter);
	}

	void block(uint type_value)
	{
		ICMP6_FILTER_SETBLOCK(type_value, &_filter);
	}

	int level(const boost::asio::ip::icmp&) const        { return IPPROTO_ICMPV6; }
	int name(const boost::asio::ip::icmp&) const         { return ICMP6_FILTER; }
	const void* data(const boost::asio::ip::icmp&) const { return &_filter; }
	size_t size(const boost::asio::ip::icmp&) const      { return sizeof(_filter); }

private:
	::icmp6_filter _filter;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_ICMP__HPP_ */
