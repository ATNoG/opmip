//=============================================================================
// Brief   : IPv6 Options
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

#ifndef OPMIP_IP_ICMP_OPTIONS__HPP_
#define OPMIP_IP_ICMP_OPTIONS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/options.hpp>
#include <opmip/ip/prefix.hpp>
#include <boost/array.hpp>
#include <algorithm>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
template<uint8 TypeValue>
class opt_link_layer : public option<TypeValue> {
	typedef option<TypeValue> base;

public:
	typedef boost::array<uint8, 14> bytes_type;

	static const size_t static_size = 2 + bytes_type::static_size;

public:
	opt_link_layer()
		: base(static_size)
	{
		_addr.assign(0);
	}

	void set(const std::pair<uint8*, size_t>& addr)
	{
		BOOST_ASSERT(addr.second <= bytes_type::static_size);
		std::copy(addr.first, addr.first + base::_length, _addr.begin());
	}

	std::pair<const uint8*, size_t> get() const
	{
		return std::pair<const uint8*, size_t>(_addr.begin(), base::_length);
	}

private:
	bytes_type _addr;
};

class opt_source_link_layer : opt_link_layer<1> { };
class opt_target_link_layer : opt_link_layer<2> { };

///////////////////////////////////////////////////////////////////////////////
class opt_prefix_info : public option<3> {
	typedef option<3> base;

public:
	typedef prefix_v6::bytes_type bytes_type;

	static const size_t static_size = 16 + bytes_type::static_size;

public:
	opt_prefix_info()
		: base(static_size),
		_plength(0),_flags(0), _valid_lifetime(0),
		_prefered_lifetime(0), _reserved(0)
	{
		_prefix.assign(0);
	}

	prefix_v6 prefix() const  { return prefix_v6(_prefix, _plength); }
	uint8     plength() const { return _plength; }
	bool      L() const       { return _flags & 0x80; }
	bool      A() const       { return _flags & 0x40; }

	void prefix(const prefix_v6& val)
	{
		_plength = val.length();
		_prefix = val.bytes();
	}

	void L(bool val)
	{
		if (val)
			_flags |= 0x80;
		else
			_flags &= ~(0x80);
	}

	void A(bool val)
	{
		if (val)
			_flags |= 0x40;
		else
			_flags &= ~(0x40);
	}

private:
	uint8      _plength;
	uint8      _flags;
	uint32     _valid_lifetime;
	uint32     _prefered_lifetime;
	uint32     _reserved;
	bytes_type _prefix;
};

///////////////////////////////////////////////////////////////////////////////
class opt_mtu : public option<5> {
	typedef option<5> base;

public:
	static const size_t static_size = 8;

public:
	opt_mtu()
		: base(static_size), _reserved(0)
	{ }

	void   set(uint32 val) { _mtu = val; }
	uint32 get() const     { return _mtu; }

public:
	uint16 _reserved;
	uint32 _mtu;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_ICMP_OPTIONS__HPP_ */
