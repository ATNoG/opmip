//=============================================================================
// Brief   : Tunnel Service for Linux
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

#include <opmip/sys/tunnel_service.hpp>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
tunnel_service::tunnel_service()
{
	boost::system::error_code ec;

	init(ec);
	detail::throw_on_error(ec, "opmip::sys::tunnel_service::tunnel_service()");
}

tunnel_service::tunnel_service(boost::system::error_code& ec)
{
	init(ec);
}

tunnel_service::~tunnel_service()
{
	int close_fd_result;

	close_fd_result = ::close(_fd);
	BOOST_ASSERT(close_fd_result == 0);
}

void tunnel_service::init(boost::system::error_code& ec)
{
	_fd = ::socket(AF_INET6, SOCK_DGRAM, 0);
	if (_fd < 0)
		ec = boost::system::error_code(errno,
		                               boost::system::get_system_category());
}

void tunnel_service::io_control(const char* name, int opcode, void* data, boost::system::error_code& ec)
{
	struct if_req {
		char  name[if_name_size];
		void* data;
	} req;
	int res;

	std::strncpy(req.name, name, sizeof(req.name));
	req.data = data;
	res = ::ioctl(_fd, opcode, &req);
	if (res < 0)
		ec = boost::system::error_code(errno,
		                               boost::system::get_system_category());
}

///////////////////////////////////////////////////////////////////////////////
tunnel_service::ip6_parameters::ip6_parameters()
{
	clear();
}

void tunnel_service::ip6_parameters::clear()
{
	std::fill(_name, _name + sizeof(_name), '\0');
	_link = 0;
	_proto = default_protocol;
	_encap_limit = default_encapsulation_limit;
	_hop_limit = default_hop_limit;
	_flowinfo = 0;
	_flags = 0;
	std::fill(_local_addr.begin(), _local_addr.end(), 0);
	std::fill(_remote_addr.begin(), _remote_addr.end(), 0);
}

bool operator!=(const tunnel_service::ip6_parameters& rhr, const tunnel_service::ip6_parameters& lhr)
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

std::ostream& operator<<(std::ostream& os, const tunnel_service::ip6_parameters& lhr)
{
	os << "{ name = "                << lhr.name()
	   << ", device = "              << lhr.device()
	   << ", protocol = "            << uint(lhr.protocol())
	   << ", encapsulation_limit = " << uint(lhr.encapsulation_limit())
	   << ", hop_limit = "           << uint(lhr.hop_limit())
	   << ", flowinfo = "            << std::hex << lhr.flowinfo() << std::dec
	   << ", flags = "               << std::hex << lhr.flags() << std::dec;

	if (lhr.flags() & (tunnel_service::ip6_parameters::ignore_encapsulation_limit
	                   | tunnel_service::ip6_parameters::use_original_traffic_class
	                   | tunnel_service::ip6_parameters::use_original_flowlabel
	                   | tunnel_service::ip6_parameters::use_original_dscp)) {
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

		if (lhr.flags() & tunnel_service::ip6_parameters::ignore_encapsulation_limit)
			pos = std::copy(flag0_str, flag0_str + sizeof(flag0_str) - 1, pos);

		if (lhr.flags() & tunnel_service::ip6_parameters::use_original_traffic_class)
			pos = std::copy(flag1_str, flag1_str + sizeof(flag1_str) - 1, pos);

		if (lhr.flags() & tunnel_service::ip6_parameters::use_original_flowlabel)
			pos = std::copy(flag2_str, flag2_str + sizeof(flag2_str) - 1, pos);

		if (lhr.flags() & tunnel_service::ip6_parameters::use_original_dscp)
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
