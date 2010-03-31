//=============================================================================
// Brief   : IPv6 Tunnel
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

#ifndef OPMIP_SYS_IP6_TUNNEL__HPP_
#define OPMIP_SYS_IP6_TUNNEL__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/ip6_tunnel_service.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/basic_io_object.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
class ip6_tunnel : public boost::asio::basic_io_object<ip6_tunnel_service> {
public:
	explicit ip6_tunnel(boost::asio::io_service& ios)
		: boost::asio::basic_io_object<ip6_tunnel_service>(ios)
	{ }


	void open(const char* name, boost::system::error_code& ec);
	void open(const char* name, int device,
	                            const ip::address_v6& local_address,
	                            const ip::address_v6& remote_address,
	                            boost::system::error_code& ec);
	bool is_open() const;
	void close(boost::system::error_code& ec);
};

inline void ip6_tunnel::open(const char* name, boost::system::error_code& ec)
{
	service.open(implementation, name, ec);
}

inline void ip6_tunnel::open(const char* name,
                             int device,
                             const ip::address_v6& local_address,
                             const ip::address_v6& remote_address,
                             boost::system::error_code& ec)
{
	service.open(implementation, name, device, local_address, remote_address, ec);
}

inline bool ip6_tunnel::is_open() const
{
	return service.is_open(implementation);
}

inline void ip6_tunnel::close(boost::system::error_code& ec)
{
	service.close(implementation, ec);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_IP6_TUNNEL__HPP_ */
