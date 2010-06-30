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
#include <opmip/sys/error.hpp>
#include <opmip/sys/ip6_tunnel_service.hpp>
#include <boost/asio/basic_io_object.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
class ip6_tunnel : public boost::asio::basic_io_object<ip6_tunnel_service> {
public:
	explicit ip6_tunnel(boost::asio::io_service& ios)
		: boost::asio::basic_io_object<ip6_tunnel_service>(ios)
	{ }


	void open(const char* name);
	void open(const char* name, boost::system::error_code& ec);
	void open(const char* name, int device,
	                            const ip::address_v6& local_address,
	                            const ip::address_v6& remote_address);
	void open(const char* name, int device,
	                            const ip::address_v6& local_address,
	                            const ip::address_v6& remote_address,
	                            boost::system::error_code& ec);
	bool is_open() const;
	void close();
	void close(boost::system::error_code& ec);

	void set_address(const ip::address_v6& address, uint prefix_length);
	void set_address(const ip::address_v6& address, uint prefix_length, boost::system::error_code& ec);

	bool get_enable();
	bool get_enable(boost::system::error_code& ec);
	void set_enable(bool value);
	void set_enable(bool value, boost::system::error_code& ec);

	uint get_device_id();
	uint get_device_id(boost::system::error_code& ec);

	bool delete_on_close(bool value);
	bool delete_on_close() const;
};

inline void ip6_tunnel::open(const char* name)
{
	boost::system::error_code ec;
	service.open(implementation, name, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::open");
}

inline void ip6_tunnel::open(const char* name, boost::system::error_code& ec)
{
	service.open(implementation, name, ec);
}

inline void ip6_tunnel::open(const char* name,
                             int device,
                             const ip::address_v6& local_address,
                             const ip::address_v6& remote_address)
{
	boost::system::error_code ec;
	service.open(implementation, name, device, local_address, remote_address, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::open");
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

inline void ip6_tunnel::close()
{
	boost::system::error_code ec;
	service.close(implementation, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::close");
}

inline void ip6_tunnel::close(boost::system::error_code& ec)
{
	service.close(implementation, ec);
}

inline void ip6_tunnel::set_address(const ip::address_v6& address, uint prefix_length)
{
	boost::system::error_code ec;
	service.set_address(implementation, address, prefix_length, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::set_address");
}

inline void ip6_tunnel::set_address(const ip::address_v6& address, uint prefix_length, boost::system::error_code& ec)
{
	service.set_address(implementation, address, prefix_length, ec);
}

inline bool ip6_tunnel::get_enable()
{
	boost::system::error_code ec;
	bool res = service.get_enable(implementation, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::get_enable");
	return res;
}

inline bool ip6_tunnel::get_enable(boost::system::error_code& ec)
{
	return service.get_enable(implementation, ec);
}

inline void ip6_tunnel::set_enable(bool value)
{
	boost::system::error_code ec;
	service.set_enable(implementation, value, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::set_enable");
}

inline void ip6_tunnel::set_enable(bool value, boost::system::error_code& ec)
{
	service.set_enable(implementation, value, ec);
}

inline uint ip6_tunnel::get_device_id()
{
	boost::system::error_code ec;
	uint res = service.get_device_id(implementation, ec);
	throw_on_error(ec, "opmip::ip6_tunnel::get_device_id");
	return res;
}

inline uint ip6_tunnel::get_device_id(boost::system::error_code& ec)
{
	return service.get_device_id(implementation, ec);
}

inline bool ip6_tunnel::delete_on_close(bool value)
{
	return service.delete_on_close(implementation, value);
}

inline bool ip6_tunnel::delete_on_close() const
{
	return service.delete_on_close(implementation);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_IP6_TUNNEL__HPP_ */
