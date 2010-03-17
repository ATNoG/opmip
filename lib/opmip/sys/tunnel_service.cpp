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

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys {

///////////////////////////////////////////////////////////////////////////////
tunnel_service::tunnel_service()
{
	boost::system::error_code ec;

	init(ec);
	if (ec)
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
	std::memset(this, 0, sizeof(*this));
	_proto = IPPROTO_IPV6;
	_encap_limit = default_encapsulation_limit;
	_hop_limit = 64;
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
	os << "{ name = "        << lhr.name()
	   << ", device = "      << lhr.device()
	   << ", protocol = "    << uint(lhr._proto)
	   << ", encap_limit = " << uint(lhr._encap_limit)
	   << ", hop_limit = "   << uint(lhr.hop_limit())
	   << ", flowinfo = "    << std::hex << lhr._flowinfo << std::dec
	   << ", flags = "       << lhr._flags;

	if (lhr._flags) {
		bool prefix = false;
		os << "(";

		if (lhr._flags & tunnel_service::ip6_parameters::ignore_encapsulation_limit) {
			os << "ignore_encapsulation_limit";
			prefix = true;
		}
		if (lhr._flags & tunnel_service::ip6_parameters::use_original_traffic_class) {
			if (prefix)
				os << "| ";
			else
				prefix = true;
			os << "use_original_traffic_class";
		}
		if (lhr._flags & tunnel_service::ip6_parameters::use_original_flowlabel) {
			if (prefix)
				os << "| ";
			else
				prefix = true;
			os << "use_original_flowlabel";
		}
		if (lhr._flags & tunnel_service::ip6_parameters::use_original_dscp) {
			if (prefix)
				os << "| ";
//			else
//				prefix = true;
			os << "use_original_dscp";
		}

		os << ")";
	}

	os << ", local_address = "  << lhr.local_address()
	   << ", remote_address = " << lhr.remote_address()
	   << " }";

	return os;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
