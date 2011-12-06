//===========================================================================================================
// Brief   : ICMP Driver
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------------------------------------
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
//===========================================================================================================

#include "icmp.hpp"
#include <opmip/ip/icmp.hpp>
#include <opmip/net/ip/icmp_parser.hpp>

#include <boost/make_shared.hpp>
#include <boost/asio/ip/multicast.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static opmip::logger log_("icmp-rs", std::cout);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct setup_poa_dev_map : boost::noncopyable {
	static const uint if_name_size  = 16;
	static const uint if_haddr_size = 6;

	static const int ioctl_get_index = 0x8933;
	static const int ioctl_get_haddr = 0x8927;

	struct ioctl {
		char name[if_name_size];

		union {
			uint       index;
			::sockaddr haddr;
		};
	};

	setup_poa_dev_map(std::map<uint, net::link::address_mac>& m)
		: _fd(::socket(AF_INET6, SOCK_DGRAM, 0)), _map(m)
	{
		if (_fd < 0) {
			boost::system::system_error se(errno, boost::system::get_system_category(),
			                               "setup_poa_dev_map_ -> ::socket");
			boost::throw_exception(se);
		}
	}

	~setup_poa_dev_map()
	{
		::close(_fd);
	}

	void insert(const std::string& name)
	{
		if (name.length() > if_name_size) {
			log_(0, "interface name \"", name, "\" exceeds system maximum length");
			return;
		}

		ioctl ioc;
		uint idx;

		std::fill(boost::begin(ioc.name), boost::end(ioc.name), '\0');
		std::copy(boost::begin(name), boost::end(name), ioc.name);

		if (::ioctl(_fd, ioctl_get_index, &ioc) != 0) {
			log_(0, "\'", name, "\' ", boost::system::get_system_category().message(errno));
			return;
		}

		idx = ioc.index;

		if (::ioctl(_fd, ioctl_get_haddr, &ioc) != 0) {
			log_(0, "\'", name, "\' ", boost::system::get_system_category().message(errno));
			return;
		}

		_map[idx] = net::link::address_mac(ioc.haddr.sa_data, if_haddr_size);

		log_(0, "interface \'", name, "\' with index ", idx, " and hw address ",
		        _map[idx], " added to device map");
	}

	int                                     _fd;
	std::map<uint, net::link::address_mac>& _map;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct icmp_drv::msg_buffer {
	icmp_endpoint endpoint;
	uchar         buffer[1480];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
icmp_drv::icmp_drv(boost::asio::io_service& ios, pmip::mag& mag)
	: _mag(mag), _icmp_sock(ios), _strand(ios)
{
}

icmp_drv::~icmp_drv()
{
}

void icmp_drv::start(const std::vector<std::string>& options)
{
	setup_poa_dev_map spdm(_poa_dev_map);
	ip::icmp::filter fo(true, ND_ROUTER_SOLICIT);
	boost::asio::ip::multicast::join_group mo(net::ip::address_v6::from_string("ff01::2"));

	std::for_each(boost::begin(options), boost::end(options),
	              boost::bind(&setup_poa_dev_map::insert, &spdm, _1));

	_icmp_sock.open(icmp_sock::protocol_type::v6());
	_icmp_sock.set_option(fo);
	_icmp_sock.set_option(mo);

	recv();
}

void icmp_drv::stop()
{
	_strand.dispatch(boost::bind(&icmp_sock::close, &_icmp_sock));
}

void icmp_drv::recv()
{
	boost::shared_ptr<msg_buffer> msg = boost::make_shared<msg_buffer>();

	_icmp_sock.async_receive_from(boost::asio::buffer(msg->buffer),
	                              msg->endpoint,
	                              boost::bind(&icmp_drv::handle_recv, this,
	                                          _1, _2, msg));
}

void icmp_drv::handle_recv(const boost::system::error_code& ec, size_t rbytes, boost::shared_ptr<msg_buffer> msg)
{
	if (ec) {
		if (ec != boost::system::errc::make_error_condition(boost::system::errc::operation_canceled))
			log_(0, "the driver must be restarted. receive error: ", ec.message());
		return;
	}
	_strand.dispatch(boost::bind(&icmp_drv::recv, this));

	net::link::address_mac laddr;

	if (net::ip::icmp_rs_parse(msg->buffer, rbytes, laddr)) {
		_strand.dispatch(boost::bind(&icmp_drv::handle_rs, this, msg->endpoint.address().to_v6(), laddr));

		log_(0, "router solicitation ", msg->endpoint, ", ", laddr);
	} else {
		log_(0, "invalid router solicitation from ", msg->endpoint);
	}
}

void icmp_drv::handle_rs(net::ip::address_v6& ep, net::link::address_mac& laddr)
{
	const pmip::mobile_node* mn = _mag.get_node_database().find_mobile_node(laddr);
	if (!mn) {
		log_(0, "router advertisement from ", ep, " - ", laddr, " ignored. not authorized");
		return;
	}

	uint dev_id = ep.scope_id();
	poa_dev_map::iterator poa = _poa_dev_map.find(dev_id);

	if (poa == boost::end(_poa_dev_map)) {
		log_(0, "router advertisement from ", ep, " - ", laddr, " ignored. device not listed");
		return;
	}

	_mag.mobile_node_attach(pmip::mag::attach_info(dev_id, poa->second, mn->id(), laddr),
	                        boost::bind(&icmp_drv::handle_attach, this, _1, ep));
}

void icmp_drv::handle_attach(const boost::system::error_code& ec, net::ip::address_v6& ep)
{
	if (ec)
		log_(0, "mobile node ", ep, " attach error: ", ec.message());
	else
		log_(0, "mobile node ", ep, " attach successfully");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////////
