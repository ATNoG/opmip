//=============================================================================
// Brief   : IPv6 Tunnel Service for Linux
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

#ifndef OPMIP_SYS_IP6_TUNNEL_SERVICE__HPP_
#define OPMIP_SYS_IP6_TUNNEL_SERVICE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/list_hook.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <cstring>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

namespace ip = boost::asio::ip;

///////////////////////////////////////////////////////////////////////////////
class ip6_tunnel_service : public boost::asio::io_service::service {
	static const std::size_t if_name_size = 16;
	static const int         ioctl_begin  = 0x89F0;
	static const int         ioctl_get    = ioctl_begin + 0;
	static const int         ioctl_add    = ioctl_begin + 1;
	static const int         ioctl_remove = ioctl_begin + 2;
	static const int         ioctl_change = ioctl_begin + 3;

public:
	static boost::asio::io_service::id id;

	class parameters;
	struct implementation_type;

public:
	explicit ip6_tunnel_service(boost::asio::io_service& ios);
	~ip6_tunnel_service();

	void construct(implementation_type& impl);
	void destroy(implementation_type& impl);

	void open(implementation_type& impl, const char* name,
	                                     boost::system::error_code& ec);
	void open(implementation_type& impl, const char* name,
	                                     int device,
	                                     const ip::address_v6& local_address,
	                                     const ip::address_v6& remote_address,
	                                     boost::system::error_code& ec);
	bool is_open(const implementation_type& impl) const;
	void close(implementation_type& impl, boost::system::error_code& ec);


	void get(parameters& op, boost::system::error_code& ec);
	void add(parameters& op, boost::system::error_code& ec);
	void remove(parameters& op, boost::system::error_code& ec);
	void change(parameters& op, boost::system::error_code& ec);

private:
	void shutdown_service();
	void io_control(const char* name, int opcode, void* data, boost::system::error_code& ec);

private:
	int          _fd;
	boost::mutex _mutex;
	list_hook    _tunnels;
};

///////////////////////////////////////////////////////////////////////////////
class ip6_tunnel_service::parameters {
	typedef ip::address_v6::bytes_type raw_address;

public:
	static const uint8 default_protocol            = IPPROTO_IPV6;
	static const uint8 default_encapsulation_limit = 4;
	static const uint8 default_hop_limit           = 64;

	enum flags {
		ignore_encapsulation_limit = 0x01, ///Don't add encapsulation limit if one isn't present in inner packet
		use_original_traffic_class = 0x02, ///Copy the traffic class field from the inner packet
		use_original_flowlabel     = 0x04, ///Copy the flowlabel from the inner packet
		use_original_dscp          = 0x10, ///Copy DSCP from the outer packet
	};

	friend bool          operator!=(const parameters& rhr, const parameters& lhr);
	friend std::ostream& operator<<(std::ostream& os, const parameters& lhr);

public:
	parameters();

	void clear();

	void name(const char* str)                      { std::strncpy(_name, str, sizeof(_name)); }
	void device(int index)                          { _link = index; }
	void protocol(uint8 val)                        { _proto = val; }
	void encapsulation_limit(uint8 val)             { _encap_limit = val; }
	void hop_limit(uint8 val)                       { _hop_limit = val; }
	void flowinfo(uint32 val)                       { _flowinfo = val; }
	void flags(uint32 val)                          { _flags = val; }
	void local_address(const ip::address_v6& addr)  { _local_addr = addr.to_bytes(); }
	void remote_address(const ip::address_v6& addr) { _remote_addr = addr.to_bytes(); }

	const char*    name() const                { return _name; }
	int            device() const              { return _link; }
	uint8          protocol() const            { return _proto; }
	uint8          encapsulation_limit() const { return _encap_limit; }
	uint8          hop_limit() const           { return _hop_limit; }
	uint32         flowinfo() const            { return _flowinfo; }
	uint32         flags() const               { return _flags; }
	ip::address_v6 local_address() const       { return ip::address_v6(_local_addr); }
	ip::address_v6 remote_address() const      { return ip::address_v6(_remote_addr); }

	const char* clone() { return "ip6tnl0"; }
	static int  proto() { return AF_INET6; }

	void* data() { return this; }

private:
	char        _name[if_name_size];
	int         _link;
	uint8       _proto;
	uint8       _encap_limit;
	uint8       _hop_limit;
	uint32      _flowinfo;
	uint32      _flags;
	raw_address _local_addr;
	raw_address _remote_addr;
};

bool          operator!=(const ip6_tunnel_service::parameters& rhr, const ip6_tunnel_service::parameters& lhr);
std::ostream& operator<<(std::ostream& os, const ip6_tunnel_service::parameters& lhr);

///////////////////////////////////////////////////////////////////////////////
struct ip6_tunnel_service::implementation_type {
	parameters data;
	list_hook  node;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_IP6_TUNNEL_SERVICE__HPP_ */
