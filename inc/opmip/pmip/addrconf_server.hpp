//=============================================================================
// Brief   : Address Configuration Server
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

#ifndef OPMIP_PMIP_ADDRCONF_SERVER__HPP_
#define OPMIP_PMIP_ADDRCONF_SERVER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/net/link/ethernet.hpp>
#include <opmip/pmip/types.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <map>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class addrconf_server {
public:
	typedef ll::mac_address                                link_address;
	typedef boost::shared_ptr<net::link::ethernet::socket> socket_ptr;
	typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

	typedef std::map<link_address, timer_ptr> client_map;

public:
	addrconf_server(boost::asio::io_service& ios);

	void add(uint device_id, const router_advertisement_info& ai);
	void del(const link_address& addr);
	void clear();

private:
	boost::asio::io_service& _io_service;
	client_map               _clients;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_ADDRCONF_SERVER__HPP_ */
