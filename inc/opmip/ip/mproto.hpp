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
#include <opmip/ip/address.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
class mproto {
public:
	struct header;
	class  endpoint;
	class  socket;

public:
	int family() const   { return AF_INET6; }
	int type() const     { return SOCK_RAW; }
	int protocol() const { return 0; }
};

struct mproto::header {
	uint8  proto;
	uint8  length;
	uint8  mh_type;
	uint8  reserved;
	uint16 checksum;
};

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
	_addr.sin6_port = ::htons(135);
	_addr.sin6_flowinfo = 0;
	_addr.sin6_addr = tmp;
	_addr.sin6_scope_id = 0;
}

inline mproto::endpoint::endpoint(const address_v6& addr)
{
	address_v6::bytes_type& tmp = reinterpret_cast<address_v6::bytes_type&>(_addr.sin6_addr.s6_addr);

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = ::htons(135); //for raw sockets, this is the protocol
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
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_MIP__HPP_ */
