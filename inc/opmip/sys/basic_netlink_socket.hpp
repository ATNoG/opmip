//=============================================================================
// Brief   : Basic Netlink Socket
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

#ifndef OPMIP_SYS_BASIC_NETLINK_SOCKET__HPP_
#define OPMIP_SYS_BASIC_NETLINK_SOCKET__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/assert.hpp>
#include <boost/asio/basic_raw_socket.hpp>
#include <sys/socket.h>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
template<int NetlinkProtocol>
struct basic_netlink_endpoint;

template<int NetlinkProtocol>
struct basic_netlink_protocol {
	typedef basic_netlink_endpoint<NetlinkProtocol> endpoint;

	int family()   { return AF_NETLINK; }
	int type()     { return SOCK_RAW; }
	int protocol() { return NetlinkProtocol; }
};

template<int NetlinkProtocol>
class basic_netlink_endpoint {
public:
	typedef basic_netlink_protocol<NetlinkProtocol> protocol_type;

public:
	basic_netlink_endpoint()
		: _family(AF_NETLINK), _pad(0), _pid(0), _groups(0)
	{ }

	void resize(std::size_t sz)
	{
		BOOST_ASSERT(sz <= sizeof(*this));
	}

	protocol_type     protocol()   { return protocol_type(); }
	::sockaddr*       data()       { return reinterpret_cast< ::sockaddr*>(this); }
	const ::sockaddr* data() const { return reinterpret_cast<const ::sockaddr*>(this); }
	std::size_t       size()       { return sizeof(*this); }
	std::size_t       capacity()   { return sizeof(*this); }

private:
	::sa_family_t _family;
	ushort        _pad;
	uint32        _pid;    ///Port ID
	uint32        _groups; ///Multicast groups mask
};

///////////////////////////////////////////////////////////////////////////////
template<int NetlinkProtocol>
class basic_netlink_socket
	: boost::asio::basic_raw_socket<basic_netlink_protocol<NetlinkProtocol> > {

	typedef boost::asio::basic_raw_socket<
	            basic_netlink_protocol<NetlinkProtocol>
	            > base;

public:
	explicit basic_netlink_socket(io_service& ios)
		: base(ios)
	{ }

};

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_BASIC_NETLINK_SOCKET__HPP_ */
