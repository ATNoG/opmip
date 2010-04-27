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

#include <opmip/sys/ip6_tunnel_service.hpp>
#include <boost/throw_exception.hpp>
#include <algorithm>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
namespace detail {

///////////////////////////////////////////////////////////////////////////////
inline void throw_on_error(boost::system::error_code& ec, const char* what)
{
	if (ec)
		boost::throw_exception(boost::system::system_error(ec, what));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace detail */

///////////////////////////////////////////////////////////////////////////////
boost::asio::io_service::id ip6_tunnel_service::id;

ip6_tunnel_service::ip6_tunnel_service(boost::asio::io_service& ios)
	: boost::asio::io_service::service(ios)
{
	boost::system::error_code ec;

	_fd = ::socket(AF_INET6, SOCK_DGRAM, 0);
	if (_fd < 0) {
		boost::throw_exception(
			boost::system::system_error(errno,
			                            boost::system::get_system_category(),
			                            "::socket"));
	}
	_tunnels.init();
}

ip6_tunnel_service::~ip6_tunnel_service()
{
	int close_fd_result;

	close_fd_result = ::close(_fd);
	BOOST_ASSERT(close_fd_result == 0);
}

void ip6_tunnel_service::construct(implementation_type& impl)
{
	boost::mutex::scoped_lock(_mutex);
	_tunnels.push_back(&impl.node);
}

void ip6_tunnel_service::destroy(implementation_type& impl)
{
	boost::mutex::scoped_lock(_mutex);
	boost::system::error_code ignore;
	close(impl, ignore);
	impl.node.remove();
}

void ip6_tunnel_service::open(implementation_type& impl, const char* name,
                                                         boost::system::error_code& ec)
{
	if (is_open(impl))
		close(impl, ec);

	if (!ec) {
		impl.data.name(name);
		impl.delete_on_close = false;
		get(impl.data, ec);
	}

	if (ec)
		impl.data.clear();
}

void ip6_tunnel_service::open(implementation_type& impl, const char* name,
                                                         int device,
                                                         const ip::address_v6& local_address,
                                                         const ip::address_v6& remote_address,
                                                         boost::system::error_code& ec)
{
	if (is_open(impl))
		close(impl, ec);

	if (!ec) {
		impl.data.name(name);
		impl.data.device(device);
		impl.data.local_address(local_address);
		impl.data.remote_address(remote_address);
		impl.delete_on_close = true;
		add(impl.data, ec);
	}

	if (ec)
		impl.data.clear();
}

bool ip6_tunnel_service::is_open(const implementation_type& impl) const
{
	return impl.data.name()[0] != '\0';
}

void ip6_tunnel_service::close(implementation_type& impl, boost::system::error_code& ec)
{
	if (is_open(impl)) {
		if (impl.delete_on_close)
			remove(impl.data, ec);
		impl.data.clear();
	} else {
		ec = boost::system::error_code(boost::system::errc::bad_file_descriptor,
		                               boost::system::get_generic_category());
	}
}

void ip6_tunnel_service::get_enable(implementation_type& impl, bool& value,
                                                               boost::system::error_code& ec)
{
	if (!is_open(impl)) {
		ec = boost::system::error_code(boost::system::errc::bad_file_descriptor,
		                               boost::system::get_generic_category());
		return;
	}

	struct if_req {
		char  name[if_name_size];
		short flags;
	} req;

	std::strncpy(req.name, impl.data.name(), sizeof(req.name));
	io_control(ioctl_get_flags, &req, ec);
	if (ec)
		return;

	value = req.flags & IFF_UP;
}

void ip6_tunnel_service::set_enable(implementation_type& impl, bool value,
                                                               boost::system::error_code& ec)
{
	if (!is_open(impl)) {
		ec = boost::system::error_code(boost::system::errc::bad_file_descriptor,
		                               boost::system::get_generic_category());
		return;
	}

	struct if_req {
		char  name[if_name_size];
		union {
			short  flags;
			size_t pad;
		};
	} req;

	std::strncpy(req.name, impl.data.name(), sizeof(req.name));
	io_control(ioctl_get_flags, &req, ec);
	if (ec) 
		return;

	if (value)
		req.flags |= IFF_UP;
	else
		req.flags &= ~IFF_UP;

	io_control(ioctl_set_flags, &req, ec);
	BOOST_ASSERT((ec || (req.flags & IFF_UP)));
}

void ip6_tunnel_service::shutdown_service()
{
	boost::mutex::scoped_lock(_mutex);
	boost::system::error_code ignore;
	implementation_type* impl;

	for (list_hook* i = _tunnels.next; !_tunnels.empty(); i = i->next) {
		impl = parent_of<implementation_type>(i, &implementation_type::node);
		close(*impl, ignore);
		i->remove();
	}
}

void ip6_tunnel_service::get(parameters& op, boost::system::error_code& ec)
{
	io_control(op.name(), ioctl_get, op.data(), ec);
}

void ip6_tunnel_service::add(parameters& op, boost::system::error_code& ec)
{
	parameters tmp = op;

	io_control("ip6tnl0", ioctl_add, op.data(), ec);
	if (!ec && (op != tmp))
		ec = boost::system::error_code(boost::system::errc::invalid_argument,
		                               boost::system::get_system_category());
}

void ip6_tunnel_service::remove(parameters& op, boost::system::error_code& ec)
{
	io_control(op.name(), ioctl_remove, op.data(), ec);
}

void ip6_tunnel_service::change(parameters& op, boost::system::error_code& ec)
{
	io_control(op.name(), ioctl_change, op.data(), ec);
}

void ip6_tunnel_service::io_control(const char* name, int opcode, void* data, boost::system::error_code& ec)
{
	struct if_req {
		char  name[if_name_size];
		void* data;
	} req;

	std::strncpy(req.name, name, sizeof(req.name));
	req.data = data;
	io_control(opcode, &req, ec);
}

void ip6_tunnel_service::io_control(int opcode, void* data, boost::system::error_code& ec)
{
	int res;

	res = ::ioctl(_fd, opcode, data);
	if (res < 0)
		ec = boost::system::error_code(errno,
		                               boost::system::get_system_category());
	else
		ec = boost::system::error_code();
}

///////////////////////////////////////////////////////////////////////////////
ip6_tunnel_service::parameters::parameters()
{
	clear();
}

void ip6_tunnel_service::parameters::clear()
{
	_name[0] = '\0';
	_link = 0;
	_proto = default_protocol;
	_encap_limit = default_encapsulation_limit;
	_hop_limit = default_hop_limit;
	_flowinfo = 0;
	_flags = 0;
	std::fill(_local_addr.begin(), _local_addr.end(), 0);
	std::fill(_remote_addr.begin(), _remote_addr.end(), 0);
}

bool operator!=(const ip6_tunnel_service::parameters& rhr, const ip6_tunnel_service::parameters& lhr)
{
	if (std::strncmp(rhr._name, lhr._name, sizeof(rhr._name)) != 0)
		return true;
	if (rhr._link != lhr._link)
		return true;
	if (rhr._proto != lhr._proto)
		return true;
	if (rhr._encap_limit != lhr._encap_limit)
		return true;
	if (rhr._hop_limit != lhr._hop_limit)
		return true;
	if (rhr._flags != lhr._flags)
		return true;

	return false;
}

std::ostream& operator<<(std::ostream& os, const ip6_tunnel_service::parameters& lhr)
{
	os << "{ name = "                << lhr.name()
	   << ", device = "              << lhr.device()
	   << ", protocol = "            << uint(lhr.protocol())
	   << ", encapsulation_limit = " << uint(lhr.encapsulation_limit())
	   << ", hop_limit = "           << uint(lhr.hop_limit())
	   << ", flowinfo = "            << std::hex << lhr.flowinfo() << std::dec
	   << ", flags = "               << std::hex << lhr.flags() << std::dec;

	if (lhr.flags() & (ip6_tunnel_service::parameters::ignore_encapsulation_limit
	                   | ip6_tunnel_service::parameters::use_original_traffic_class
	                   | ip6_tunnel_service::parameters::use_original_flowlabel
	                   | ip6_tunnel_service::parameters::use_original_dscp)) {
		static const char scope_str[] = " ()";
		static const char flag0_str[] = "ignore_encapsulation_limit | ";
		static const char flag1_str[] = "use_original_traffic_class | ";
		static const char flag2_str[] = "use_original_flowlabel | ";
		static const char flag3_str[] = "use_original_dscp";
		static const std::size_t slen = sizeof(scope_str)
		                                + sizeof(flag0_str)
		                                + sizeof(flag1_str)
		                                + sizeof(flag2_str)
		                                + sizeof(flag3_str)
		                                - 4;
		boost::array<char, slen>::iterator pos;
		boost::array<char, slen> tmp;


		pos = std::copy(scope_str, scope_str + 2, tmp.begin());

		if (lhr.flags() & ip6_tunnel_service::parameters::ignore_encapsulation_limit)
			pos = std::copy(flag0_str, flag0_str + sizeof(flag0_str) - 1, pos);

		if (lhr.flags() & ip6_tunnel_service::parameters::use_original_traffic_class)
			pos = std::copy(flag1_str, flag1_str + sizeof(flag1_str) - 1, pos);

		if (lhr.flags() & ip6_tunnel_service::parameters::use_original_flowlabel)
			pos = std::copy(flag2_str, flag2_str + sizeof(flag2_str) - 1, pos);

		if (lhr.flags() & ip6_tunnel_service::parameters::use_original_dscp)
			pos = std::copy(flag3_str, flag3_str + sizeof(flag3_str) - 1, pos) + 3;

		pos = std::copy(scope_str + 2, scope_str + sizeof(scope_str), pos - 3);
		os << tmp.elems;
	}

	os << ", local_address = "  << lhr.local_address()
	   << ", remote_address = " << lhr.remote_address()
	   << " }";

	return os;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
