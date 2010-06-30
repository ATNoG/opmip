//=============================================================================
// Brief   : Mobility Protocol
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

#ifndef OPMIP_IP_MPROTO__HPP_
#define OPMIP_IP_MPROTO__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/error.hpp>
#include <opmip/ip/address.hpp>
#include <boost/asio/basic_raw_socket.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
class mproto {
public:
	typedef boost::asio::basic_raw_socket<mproto> socket;

	class endpoint;
	class header;
	class pbu;
	class pba;
	class option;

	enum mh_types {
		mh_pbu = 5,
		mh_pba = 6,
	};

public:
	int family() const   { return AF_INET6; }
	int type() const     { return SOCK_RAW; }
	int protocol() const { return 135; }
};

///////////////////////////////////////////////////////////////////////////////
class mproto::endpoint {
public:
	typedef mproto protocol_type;

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

inline mproto::endpoint::endpoint()
{
	::in6_addr tmp = IN6ADDR_ANY_INIT;

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = 0;
	_addr.sin6_flowinfo = 0;
	_addr.sin6_addr = tmp;
	_addr.sin6_scope_id = 0;
}

inline mproto::endpoint::endpoint(const address_v6& addr)
{
	address_v6::bytes_type& tmp = reinterpret_cast<address_v6::bytes_type&>(_addr.sin6_addr.s6_addr);

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = 0;
	_addr.sin6_flowinfo = 0;
	tmp = addr.to_bytes();
	_addr.sin6_scope_id = addr.scope_id();
}

inline address_v6 mproto::endpoint::address() const
{
	return address_v6(reinterpret_cast<const address_v6::bytes_type&>(_addr.sin6_addr.s6_addr), _addr.sin6_scope_id);
}

inline void mproto::endpoint::address(const address_v6& addr)
{
	address_v6::bytes_type& tmp = reinterpret_cast<address_v6::bytes_type&>(_addr.sin6_addr.s6_addr);

	tmp = addr.to_bytes();
	_addr.sin6_scope_id = addr.scope_id();
}

///////////////////////////////////////////////////////////////////////////////
struct mproto::header {
	static header* cast(void* buff, size_t len)
	{
		header* hdr = reinterpret_cast<header*>(buff);

		if (len < sizeof(header) || len < hdr->length())
			return nullptr;

		return hdr;
	}

	void init(uint8 type, size_t len)
	{
		next = 59;
		length(len);
		mh_type = type;
		reserved = 0;
		checksum = 0;
	}

	size_t length() const { return (_length + 1) * 8; }

	void length(size_t len)
	{
		BOOST_ASSERT(!(len % 8));
		_length = (len / 8) - 1;
	}

	uint8  next;
	uint8  _length;
	uint8  mh_type;
	uint8  reserved;
	uint16 checksum;
};

///////////////////////////////////////////////////////////////////////////////
class mproto::pbu : public header {
public:
	static const size_t mh_type = 5;
	static const size_t mh_size = 12;

public:
	static pbu* cast(header* hdr)
	{
		if ((hdr->mh_type != mh_type) || (hdr->length() < sizeof(pbu)))
			return nullptr;

		return static_cast<pbu*>(hdr);
	}

public:
	pbu()
		: _sequence(0), _flags1(0), _flags2(0), _lifetime(0)
	{ }

	uint16 sequence() const { return ::ntohs(_sequence); }
	bool   a() const;
	bool   h() const;
	bool   l() const;
	bool   k() const;
	bool   m() const;
	bool   r() const;
	bool   p() const;
	uint16 lifetime() const { return ::ntohs(_lifetime); }

	void sequence(uint16 value) { _sequence = ::htons(value); }
	void a(bool value);
	void h(bool value);
	void l(bool value);
	void k(bool value);
	void m(bool value);
	void r(bool value);
	void p(bool value);
	void lifetime(uint16 value) { _lifetime = ::htons(value); }

	const void* data() const
	{
		return this;
	}

private:
	uint16 _sequence;
	uint8  _flags1;
	uint8  _flags2;
	uint16 _lifetime;
};

bool mproto::pbu::a() const
{
	const uint8 v = 1u << 7;

	return _flags1 & v;
}

bool mproto::pbu::h() const
{
	const uint8 v = 1u << 6;

	return _flags1 & v;
}

bool mproto::pbu::l() const
{
	const uint8 v = 1u << 5;

	return _flags1 & v;
}

bool mproto::pbu::k() const
{
	const uint8 v = 1u << 4;

	return _flags1 & v;
}

bool mproto::pbu::m() const
{
	const uint8 v = 1u << 3;

	return _flags1 & v;
}

bool mproto::pbu::r() const
{
	const uint8 v = 1u << 2;

	return _flags1 & v;
}

bool mproto::pbu::p() const
{
	const uint8 v = 1u << 1;

	return _flags1 & v;
}

void mproto::pbu::a(bool value)
{
	const uint8 v = 1u << 7;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::h(bool value)
{
	const uint8 v = 1u << 6;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::l(bool value)
{
	const uint8 v = 1u << 5;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::k(bool value)
{
	const uint8 v = 1u << 4;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::m(bool value)
{
	const uint8 v = 1u << 3;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::r(bool value)
{
	const uint8 v = 1u << 2;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::p(bool value)
{
	const uint8 v = 1u << 1;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

///////////////////////////////////////////////////////////////////////////////
class mproto::pba : public header {
public:
	static const size_t mh_type = 6;
	static const size_t mh_size = 12;

	enum status_type {
		status_ok                     = 0,   ///Accepted
		status_ok_needs_prefix        = 1,   ///Accepted but prefix discovery necessary
		status_unspecified            = 128, ///Reason unspecified
		status_prohibited             = 129, ///Administratively prohibited
		status_insufficient_resources = 130, ///Insufficient resources
		status_hr_not_supported       = 131, ///Home registration not supported
		status_not_home_subnet        = 132, ///Not home subnet
		status_not_home_agent         = 133, ///Not home agent for this mobile node
		status_duplicate_address      = 134, ///Duplicate Address Detection failed
		status_bad_sequence           = 135, ///Sequence number out of window
		status_expired_home           = 136, //Expired home nonce index
		status_expired_care_of        = 137, ///Expired care-of nonce index
		status_expired                = 138, ///Expired nonces
		status_invalid_registration   = 139, ///Registration type change disallowed
	};

public:
	static pba* cast(header* hdr)
	{
		if ((hdr->mh_type != mh_type) || (hdr->length() < sizeof(pba)))
			return nullptr;

		return static_cast<pba*>(hdr);
	}

public:
	pba()
		: _status(0), _flags(0), _sequence(0), _lifetime(0)
	{ }

	status_type status() const   { return status_type(_status); }
	bool        k() const;
	bool        r() const;
	bool        p() const;
	uint16      sequence() const { return ::ntohs(_sequence); }
	uint16      lifetime() const { return ::ntohs(_lifetime); }

	void status(status_type value) { _status = value; }
	void k(bool value);
	void r(bool value);
	void p(bool value);
	void sequence(uint16 value)  { _sequence = ::htons(value); }
	void lifetime(uint16 value)  { _lifetime = ::htons(value); }

	const void* data() const
	{
		return this;
	}

private:
	uint8  _status;
	uint8  _flags;
	uint16 _sequence;
	uint16 _lifetime;
};

bool mproto::pba::k() const
{
	const uint8 v = 1u << 7;

	return _flags & v;
}

bool mproto::pba::r() const
{
	const uint8 v = 1u << 6;

	return _flags & v;
}

bool mproto::pba::p() const
{
	const uint8 v = 1u << 6;

	return _flags & v;
}

void mproto::pba::k(bool value)
{
	const uint8 v = 1u << 7;

	if (value)
		_flags |= v;
	else
		_flags &= ~v;
}

void mproto::pba::r(bool value)
{
	const uint8 v = 1u << 6;

	if (value)
		_flags |= v;
	else
		_flags &= ~v;
}

void mproto::pba::p(bool value)
{
	const uint8 v = 1u << 5;

	if (value)
		_flags |= v;
	else
		_flags &= ~v;
}

///////////////////////////////////////////////////////////////////////////////
class mproto::option {
public:
	static option* cast(void* buff, size_t length)
	{
		option* opt = reinterpret_cast<option*>(buff);

		if ((length < 2) || length < (opt->length + 2u))
			return nullptr;

		return opt;
	}

	static size_t size(const option* opt)
	{
		if (!opt->type)
			return 1;

		return opt->length + 2;
	}

	enum types {
		nai_type     = 8,
		handoff_type = 23,
		att_type     = 24,
	};

	struct nai {
		static const uint8 type_value = 8;

		uint8 subtype;
		char  id[0];
	};

	struct handoff {
		static const uint8 type_value = 23;

		enum type {
			k_reserved       = 0,
			k_new_interface  = 1,
			k_diff_interface = 2,
			k_same_interface = 3,
			k_unknown        = 4,
			k_not_changed    = 5,
		};

		uint8 reserved;
		uint8 indicator;
	};

	struct att {
		static const uint8 type_value = 24;

		enum {
			virtua        = 1,
			ppp           = 2,
			ieee802_3     = 3,
			ieee802_11abg = 4,
			ieee802_16e   = 5,
		};

		uint8 reserved;
		uint8 tech_type;
	};

public:
	template<class OptionT>
	option(OptionT, size_t xlength = 0)
	{
		BOOST_ASSERT((sizeof(OptionT) < 0xff));
		BOOST_ASSERT((xlength < 0xff));
		BOOST_ASSERT(((sizeof(OptionT) + xlength) < 0xff));

		type = OptionT::type_value;
		length = sizeof(OptionT) + xlength;
	}

	template<class T>
	T* get()
	{
		uint8* tmp = reinterpret_cast<uint8*>(this) + 2;

		return reinterpret_cast<T*>(tmp);
	}

public:
	uint8 type;
	uint8 length;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_MIP__HPP_ */
