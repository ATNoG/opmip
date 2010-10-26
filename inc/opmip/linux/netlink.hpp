//=============================================================================
// Brief   : Netlink
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

#ifndef OPMIP_LINUX_NETLINK__HPP_
#define OPMIP_LINUX_NETLINK__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/asio/basic_raw_socket.hpp>
#include <sys/socket.h>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux {

///////////////////////////////////////////////////////////////////////////////
template<int Protocol>
class netlink_ {
	typedef netlink_<Protocol> this_type;

public:
	typedef boost::asio::basic_raw_socket<this_type> socket;

	class endpoint {
	public:
		typedef netlink_<Protocol> protocol_type;

	public:
		endpoint()
			: _family(AF_NETLINK), _pad(0), _pid(0), _groups(0)
		{ }

		endpoint(uint32 subscriptions)
			: _family(AF_NETLINK), _pad(0), _pid(0), _groups(subscriptions)
		{ }


		protocol_type     protocol() const       { return protocol_type(); }
		::sockaddr*       data()                 { return reinterpret_cast< ::sockaddr*>(this); }
		const ::sockaddr* data() const           { return reinterpret_cast<const ::sockaddr*>(this); }
		std::size_t       size() const           { return sizeof(*this); }
		std::size_t       capacity() const       { return sizeof(*this); }
		void              resize(std::size_t sz) { BOOST_ASSERT(sz <= sizeof(*this)); }

		void   subscriptions(uint32 val)       { _groups = val; }
		uint32 subscriptions(uint32 val) const { return _groups; }

	private:
		uint16 _family;
		uint16 _pad;
		uint32 _pid;    ///Port ID
		uint32 _groups; ///Multicast groups mask
	};

	class multicast {
	public:
		enum opcode {
			add  = 1,
			drop = 2,
		};

		multicast(opcode op, uint32 group)
			: _opcode(op), _group(group)
		{ }

		int level(const this_type&) const        { return 270; /*SOL_NETLINK*/ }
		int name(const this_type&) const         { return _opcode; }
		const void* data(const this_type&) const { return &_group; }
		size_t size(const this_type&) const      { return sizeof(_group); }

	private:
		int    _opcode;
		uint32 _group;
	};

public:
	int family() const   { return AF_NETLINK; }
	int type() const     { return SOCK_RAW; }
	int protocol() const { return Protocol; }
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace linux */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LINUX_NETLINK__HPP_ */
