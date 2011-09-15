//=============================================================================
// Brief   : DHCPv6
// Authors : Bruno Santos <bsantos@av.it.pt>
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

#include <opmip/base.hpp>
#include <opmip/net/ip/dhcp_v6.hpp>
#include <utility>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip { namespace dhcp_v6 {

///////////////////////////////////////////////////////////////////////////////
class uint24 {
	static const uint k_mask = 0x00ffffff;

	typedef uint value_type;

public:
	uint24()
		: _val(0)
	{ }

	uint24(uint val)
		: _val(val & k_mask)
	{ }

	uint24& operator=(uint val)
	{
		_val = val & k_mask;
		return *this;
	}

	uint get() const { return _val; }

private:
	uint _val;
};

template<class T>
struct put_be_int_impl;

template<>
struct put_be_int_impl<uint8> {
	bool operator()(buffer_type& buff, uint8 v) const
	{
		if (buffer_size(buff) < 1)
			return false;

		*buff.first++ = v;
		return true;
	}
};

template<>
struct put_be_int_impl<uint16> {
	bool operator()(buffer_type& buff, uint16 v) const
	{
		if (buffer_size(buff) < 2)
			return false;

		*buff.first++ = v >> 8;
		*buff.first++ = v & 0xff;
		return true;
	}
};

template<>
struct put_be_int_impl<uint24> {
	bool operator()(buffer_type& buff, uint24 v) const
	{
		if (buffer_size(buff) < 3)
			return false;

		uint val = v.get();

		*buff.first++ = val >> 16;
		*buff.first++ = (val >> 8) & 0xff;
		*buff.first++ = val & 0xff;
		return true;
	}
};

template<>
struct put_be_int_impl<uint32> {
	bool operator()(buffer_type& buff, uint32 v) const
	{
		if (buffer_size(buff) < 4)
			return false;

		*buff.first++ = (v >> 24);
		*buff.first++ = (v >> 16) & 0xff;
		*buff.first++ = (v >> 8) & 0xff;
		*buff.first++ = v & 0xff;
		return true;
	}
};

template<>
struct put_be_int_impl<uint64> {
	bool operator()(buffer_type& buff, uint64 v) const
	{
		if (buffer_size(buff) < 8)
			return false;

		*buff.first++ = (v >> 56);
		*buff.first++ = (v >> 48) & 0xff;
		*buff.first++ = (v >> 40) & 0xff;
		*buff.first++ = (v >> 32) & 0xff;
		*buff.first++ = (v >> 24) & 0xff;
		*buff.first++ = (v >> 16) & 0xff;
		*buff.first++ = (v >> 8) & 0xff;
		*buff.first++ = v & 0xff;
		return true;
	}
};

template<class T>
struct get_be_int_impl;

template<>
struct get_be_int_impl<uint8> {
	bool operator()(buffer_type& buff, uint8& v) const
	{
		if (buffer_size(buff) < 1)
			return false;

		v = *buff.first++;
		return true;
	}
};

template<>
struct get_be_int_impl<uint16> {
	bool operator()(buffer_type& buff, uint16& v) const
	{
		if (buffer_size(buff) < 2)
			return false;

		v  = *buff.first++ << 8;
		v |= *buff.first++;
		return true;
	}
};

template<>
struct get_be_int_impl<uint24> {
	bool operator()(buffer_type& buff, uint24& v) const
	{
		if (buffer_size(buff) < 3)
			return false;

		uint val;

		val  = *buff.first++ << 16;
		val |= *buff.first++ << 8;
		val |= *buff.first++;

		v = val;

		return true;
	}
};

template<>
struct get_be_int_impl<uint32> {
	bool operator()(buffer_type& buff, uint32& v) const
	{
		if (buffer_size(buff) < 4)
			return false;

		v  = *buff.first++ << 24;
		v |= *buff.first++ << 16;
		v |= *buff.first++ << 8;
		v |= *buff.first++;
		return true;
	}
};

template<>
struct get_be_int_impl<uint64> {
	bool operator()(buffer_type& buff, uint64& v) const
	{
		if (buffer_size(buff) < 8)
			return false;

		v  = uint64(*buff.first++) << 56;
		v |= uint64(*buff.first++) << 48;
		v |= uint64(*buff.first++) << 40;
		v |= uint64(*buff.first++) << 32;
		v |= *buff.first++ << 24;
		v |= *buff.first++ << 16;
		v |= *buff.first++ << 8;
		v |= *buff.first++;
		return true;
	}
};

template<class T>
bool put_be_int(buffer_type& buff, T v)
{
	put_be_int_impl<T> impl;

	return impl(buff, v);
}

template<class T>
bool get_be_int(buffer_type& buff, T& v)
{
	get_be_int_impl<T> impl;

	return impl(buff, v);
}

// Generators /////////////////////////////////////////////////////////////////
bool gen_header(buffer_type& buff, opcode op, uint tid)
{
	return put_be_int(buff, uint8(op))
		&& put_be_int(buff, uint24(tid));
}

bool gen_option_begin(buffer_type& buff, option opt, buffer_type& state)
{
	if (!put_be_int(buff, uint16(opt)) || !put_be_int(buff, uint16(0)))
		return false;

	state = buff;
	return true;
}

bool gen_option_end(buffer_type& buff, buffer_type state)
{
	size_t len = buff.first - state.first;

	BOOST_ASSERT(buff.second == state.second);
	BOOST_ASSERT(len <= 0xffff);

	state.first -= 2;
	return put_be_int(state, uint16(len));
}

bool gen_option_server_id(buffer_type& buff, const link::address_mac& link_addr)
{
	buffer_type state;

	if (!gen_option_begin(buff, server_id, state)
		|| !put_be_int(buff, uint16(3))
		|| !put_be_int(buff, uint16(1))
		|| buffer_size(buff) < link::address_mac::bytes_type::static_size)
		return false;

	const link::address_mac::bytes_type& raw = link_addr.to_bytes();
	buff.first = std::copy(raw.begin(), raw.end(), buff.first);
	return gen_option_end(buff, state);
}

bool gen_option_client_id(buffer_type& buff, const buffer_type& cid)
{
	buffer_type state;

	if (!gen_option_begin(buff, client_id, state)
		|| buffer_size(buff) < buffer_size(cid))
		return false;

	buff.first = std::copy(cid.first, cid.second, buff.first);
	return gen_option_end(buff, state);
}

bool gen_option_ia(buffer_type& buff, uint32 id, uint32 t1, uint32 t2, buffer_type* state)
{
	buffer_type st;

	if (!gen_option_begin(buff, ia_na, st)
		|| !put_be_int(buff, id) || !put_be_int(buff, t1) || !put_be_int(buff, t2)
		|| !gen_option_end(buff, st))
		return false;

	if (state)
		*state = st;

	return true;
}

bool gen_option_addr(buffer_type& buff, const address_v6& addr,
                     uint32 pref_lifetime, uint32 val_lifetime,
                     buffer_type state)
{
	buffer_type st;

	if (!gen_option_begin(buff, ia_addr, st)
		|| buffer_size(buff) < address_v6::bytes_type::static_size)
		return false;

	const address_v6::bytes_type& raw = addr.to_bytes();
	buff.first = std::copy(raw.begin(), raw.end(), buff.first);

	if (!put_be_int(buff, pref_lifetime) || !put_be_int(buff, val_lifetime))
		return false;

	return gen_option_end(buff, st) && gen_option_end(buff, state);
}

bool gen_option_status(buffer_type& buff, status st, const char* msg)
{
	buffer_type state;
	size_t len = std::strlen(msg);

	if (!gen_option_begin(buff, status_code, state)
		|| !put_be_int(buff, uint16(st))
		|| buffer_size(buff) < len)
		return false;

	buff.first = std::copy(reinterpret_cast<const uint8*>(msg),
	                       reinterpret_cast<const uint8*>(msg) + len,
	                       buff.first);
	return gen_option_end(buff, state);
}

bool gen_message(buffer_type& buff, opcode op, uint32 tid,
                 const link::address_mac& link_addr,
                 const buffer_type& client_id)
{
	return gen_header(buff, op, tid)
		&& gen_option_server_id(buff, link_addr)
		&& gen_option_client_id(buff, client_id);
}

// Parsers ////////////////////////////////////////////////////////////////////
bool parse_header(buffer_type& buff, opcode& op, uint& tid)
{
	uint8 tmp1;
	uint24 tmp2;

	if (!get_be_int(buff, tmp1)
		|| !get_be_int(buff, tmp2))
		return false;

	op = static_cast<opcode>(tmp1);
	tid = tmp2.get();

	return true;
}

bool parse_option(buffer_type& buff, option& opt, buffer_type& data)
{
	uint16 tp;
	uint16 len;

	if (!get_be_int(buff, tp)
		|| !get_be_int(buff, len)
		|| buffer_size(buff) < len)
		return false;

	opt = option(tp);
	data.first = buff.first;
	data.second = buff.first + len;
	buff.first = data.second;

	return true;
}

bool parse_option_duid(buffer_type buff, link::address_mac& link_addr)
{
	uint16 tp;

	if (!get_be_int(buff, tp))
		return false;

	if (tp < 1 || tp > 3)
		return false;

	if (tp == 1 || tp == 3) {
		uint16 hw;

		if (!get_be_int(buff, hw))
			return false;

		if (hw != 1) //TODO: we don't support any other hw type besides ethernet
			return false;
	}

	if (tp == 1 || tp == 2) {
		if (buffer_size(buff) < 4)
			return false;

		buff.first += 4;
	}

	if (buffer_size(buff) != 6)
		return false;

	link_addr = link::address_mac(buff.first, 6);
	buff.first += 6;

	return buff.first == buff.second;
}

bool parse_option_ia(buffer_type& buff, uint32& id, uint32& t1, uint32& t2)
{
	return get_be_int(buff, id) && get_be_int(buff, t1) && get_be_int(buff, t2);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace dhcp_v6 */ } /* namespace ip */ } /* namespace net */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////