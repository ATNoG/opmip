//=============================================================================
// Brief   : Netlink 802.11 Interface
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

#ifndef OPMIP_LINUX_NL80211_NL80211__HPP_
#define OPMIP_LINUX_NL80211_NL80211__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/linux/genetlink.hpp>
#include <opmip/ll/mac_address.hpp>
#include <boost/function.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux {

///////////////////////////////////////////////////////////////////////////////
class nl80211 {
public:
	struct event {
		enum which_t {
			unknown,
			new_sta,
			associate,
			disassociate
		};

		event()
			: which(unknown), phy_id(-1), if_index(0)
		{ }

		which_t         which;
		uint            phy_id;
		uint            if_index;
		uint            generation;
		ll::mac_address mac;
		ll::mac_address src_addr;
		ll::mac_address dst_addr;
		ll::mac_address bssid;
	};

	typedef boost::function<void(const boost::system::error_code&, const event&)> event_handler_t;

public:
	nl80211(boost::asio::io_service& ios);

	template<class EventHandler>
	void open(EventHandler event_handler)
	{
		_event_handler = event_handler;
		open_();
	}

	void close();

private:
	void open_();

	void setup_family(boost::system::error_code& ec);
	void setup_frame(boost::system::error_code& ec);
	void receive_handler(const boost::system::error_code& ec, size_t rbytes);
	void message_handler(genetlink::message& msg);

private:
	genetlink::socket _gnl;
	uint16            _family;
	uint32            _version;
	uint32            _hdrsize;
	uint32            _mlne_mcast_id;
	event_handler_t   _event_handler;
	uchar             _buffer[4096];
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace linux */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LINUX_NL80211_NL80211__HPP_ */
