//==============================================================================
// Brief   : IEEE 802.21 MIH User Driver
// Authors : Bruno Santos <bsantos@av.it.pt>
//           Carlos Guimarães <cguimaraes@av.it.pt>
// -----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//==============================================================================

#include "ieee802_21_driver.hpp"
#include <opmip/ptime.hpp>
#include <opmip/ll/mac_address.hpp>

#include <odtone/mih/request.hpp>
#include <odtone/mih/response.hpp>
#include <odtone/mih/indication.hpp>
#include <odtone/mih/confirm.hpp>
#include <odtone/mih/tlv_types.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include <string>
#include <iostream>

#include <netinet/if_ether.h>

////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace app {

static opmip::logger log_("802.21", std::cout);
////////////////////////////////////////////////////////////////////////////////
/**
 * Get the interface index.
 *
 * @param interface_name The interface name.
 * @return The interface index.
 */
int get_interface_index(const char *interface_name)
{
	int fd;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));

	// Setup IFR for ioclt
	strncpy (ifr.ifr_name, interface_name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

	// Create socket
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd == -1) {
		log_(0, "Could not create raw socket: ", strerror(errno));
		return -1;
	}

	// Get interface index
	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
		close(fd);
		return -1;
	}

	// Close socket
	close(fd);

	return ifr.ifr_ifindex;
}

/**
 * Get the interface physical address.
 *
 * @param interface_name The interface name.
 * @return The interface physical address.
 */
std::string get_interface_address(const char *interface_name)
{
	int fd;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));

	// Setup IFR for ioclt
	strncpy (ifr.ifr_name, interface_name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

	// Create socket
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd == -1) {
		log_(0, "Could not create raw socket: ", strerror(errno));
		return "";
	}

	// Get interface index
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
		close(fd);
		return "";
	}

	// Close socket
	close(fd);

	ll::mac_address addr(ifr.ifr_hwaddr.sa_data, 6);
	return addr.to_string();
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Node attachment callback.
 *
 * @param mn_address Mobile node address.
 * @param ec Error code.
 */
static void attach_result(const ll::mac_address& mn_address, const boost::system::error_code& ec)
{
	log_(0, "node ", mn_address, " attachment completed with code ", ec);
}

/**
 * Node detachment callback.
 *
 * @param mn_address Mobile node address.
 * @param ec Error code.
 */
static void detach_result(const ll::mac_address& mn_address, const boost::system::error_code& ec)
{
	log_(0, "node ", mn_address, " detachment completed with code ", ec);
}
////////////////////////////////////////////////////////////////////////////////

void ieee802_21_driver::decode_options(const std::vector<std::string>& options)
{
	if(options.size() < 2 || options.size() > 3) {
		log_(0, "IEEE 802.21 Usage: \"<local_mihf_id>\" \"<local_interface_mapping>\" \"<remote_mihf_mapping>\"");
		throw("Invalid options");
	}

	// Decode local MIHF
	_local_mihf = options[0];

	// Decode local interface mapping
	{
		boost::char_separator<char> sep1(" ");
		boost::tokenizer< boost::char_separator<char> > list_tokens(options[1], sep1);

		BOOST_FOREACH(std::string str, list_tokens) {
			std::string mac = get_interface_address(str.c_str());
			std::string interface = str;

			_local.insert(std::pair<std::string, std::string>(mac, interface));
		}
	}

	// Decode remote MIHF mapping
	if(options.size() == 3)
	{
		boost::char_separator<char> sep1(",");
		boost::tokenizer< boost::char_separator<char> > list_tokens(options[2], sep1);

		BOOST_FOREACH(std::string str, list_tokens)
		{
			boost::char_separator<char> sep2(" ");
			boost::tokenizer< boost::char_separator<char> > tokens(str, sep2);
			boost::tokenizer< boost::char_separator<char> >::iterator it = tokens.begin();
			std::vector<std::string> words(tokens.begin(), tokens.end());

			if(words.size() != 2) {
				log_(0, "Remote MIHF Mapping Usage: <MIHF 1> <interface 1>, <MIHF N> <interface N>");
				throw("Invalid options");
			}

			std::string mihf = words[0];
			std::string interface = words[1];

			_poa.insert(std::pair<std::string, std::string>(mihf, interface));
		}
	}
}

/**
 * User registration handler.
 *
 * @param msg Received message.
 * @param ec Error Code.
 */
void ieee802_21_driver::user_reg_handler(odtone::mih::message &msg, const boost::system::error_code& ec)
{
	log_(0, "MIH-User register result: ", ec.message());
	if (ec) {
		return;
	}

	// Discover capabilities from the local MIHF
	odtone::mih::message req;
	req << odtone::mih::request(odtone::mih::request::capability_discover);
	req.destination(odtone::mih::id(_local_mihf));
	_user_sap->async_send(req, boost::bind(&ieee802_21_driver::capability_discover_confirm, this, _1, _2));
	log_(0, "MIH-User has sent a Capability_Discover.request towards its local MIHF");

	// Discover capabilities from known remote MIHFs
	for(std::map<std::string, std::string>::iterator it = _poa.begin(); it != _poa.end(); ++it) {
		req.destination(odtone::mih::id(it->first));
		_user_sap->async_send(req, boost::bind(&ieee802_21_driver::capability_discover_confirm, this, _1, _2));
		log_(0, "MIH-User has sent a Capability_Discover.request towards ", it->first);
	}
}

/**
 * Capability Discover handler.
 *
 * @param msg Received message.
 * @param ec Error Code.
 */
void ieee802_21_driver::capability_discover_confirm(odtone::mih::message& msg, const boost::system::error_code& ec)
{
	if (ec) {
		log_(0, __FUNCTION__, " error: ", ec.message());
		return;
	}

	odtone::mih::status st;
	boost::optional<odtone::mih::net_type_addr_list> ntal;
	boost::optional<odtone::mih::event_list> evt;

	msg >> odtone::mih::confirm()
		& odtone::mih::tlv_status(st)
		& odtone::mih::tlv_net_type_addr_list(ntal)
		& odtone::mih::tlv_event_list(evt);

	log_(0, "MIH-User has received a Capability_Discover.response with status ",
			st.get(), " and the following capabilities:");

	if(st != odtone::mih::status_success)
		return;

	if (ntal) {
		for (odtone::mih::net_type_addr_list::iterator i = ntal->begin(); i != ntal->end(); ++i)
			log_(0,  *i);

	} else {
		log_(0,  "none");
	}

	//
	// event subscription
	//
	// For every interface the MIHF sent in the
	// Capability_Discover.response send an Event_Subscribe.request
	// for all availabe events
	//
	if (ntal && evt) {
		for (odtone::mih::net_type_addr_list::iterator i = ntal->begin(); i != ntal->end(); ++i) {
			odtone::mih::message req;
			odtone::mih::link_tuple_id li;

			if (i->nettype.link.which() == 1)
			{
				li.addr = i->addr;
				li.type = boost::get<odtone::mih::link_type>(i->nettype.link);

				req << odtone::mih::request(odtone::mih::request::event_subscribe, msg.source())
					& odtone::mih::tlv_link_identifier(li)
					& odtone::mih::tlv_event_list(evt);

				_user_sap->async_send(req, [this](odtone::mih::message& msg, const boost::system::error_code& ec) {
					if (ec) {
						log_(0, __FUNCTION__, " error: ", ec.message());
						return;
					}

					odtone::mih::status st;
					msg >> odtone::mih::response()
						& odtone::mih::tlv_status(st);

					log_(0, "status: ", st.get());
				});

				log_(0, "MIH-User has sent Event_Subscribe.request");
			}
		}
	}
}

/**
 * Default MIH event handler.
 *
 * @param msg Received message.
 * @param ec Error code.
 */
void ieee802_21_driver::event_handler(odtone::mih::message& msg, const boost::system::error_code& ec)
{
	if (ec) {
		log_(0, __FUNCTION__, " error: ", ec.message());
		return;
	}

	switch (msg.mid()) {
	case odtone::mih::indication::link_up: {
		log_(0, "MIH-User has received a local event \"link_up\"");

		odtone::mih::link_tuple_id li;

		// Decode only the required TLV
		msg >> odtone::mih::indication(odtone::mih::indication::link_up)
			& odtone::mih::tlv_link_identifier(li);

		// Get MN MAC address
		ll::mac_address mn_address;
		if(odtone::mih::link_addr* poa_addr = boost::get<odtone::mih::link_addr>(&li.poa_addr)) {
			if(odtone::mih::mac_addr* mn_mac = boost::get<odtone::mih::mac_addr>(poa_addr)) {
				mn_address = ll::mac_address::from_string(mn_mac->address());
			}
		} else {
			return;
		}

		const opmip::pmip::mobile_node* mn = _mag.get_node_database().find_mobile_node(mn_address);
		if(!mn) {
			log_(0, "node ", mn_address, " not authorized");
			return;
		}

		ll::mac_address if_address;
		uint if_index;
		if(_local_mihf.compare(msg.source().to_string()) == 0) {
			// Get MAG interface address
			odtone::mih::mac_addr if_mac = boost::get<odtone::mih::mac_addr>(li.addr);
			if_address = ll::mac_address::from_string(if_mac.address());

			// Get MAG interface index
			std::map<std::string, std::string>::iterator it;
			it = _local.find(if_address.to_string());
			if(it != _local.end()) {
				if_index = get_interface_index(it->second.c_str());
			} else {
				return;
			}
		} else {
			std::map<std::string, std::string>::iterator it;
			it = _poa.find(msg.source().to_string());
			if(it != _poa.end()) {
				// Get MAG interface address
				std::string if_mac = get_interface_address(it->second.c_str());
				if_address = ll::mac_address::from_string(if_mac);

				// Get MAG interface index
				if_index = get_interface_index(it->second.c_str());
			} else {
				return;
			}
		}

		if((if_index < 0) || (if_address.to_string().compare("") == 0))
			return;

		opmip::pmip::mag::attach_info ai(if_index,
										 if_address,
										 mn->id(),
										 mn_address);

		_mag.mobile_node_attach(ai, boost::bind(attach_result, mn_address, _1));

		} break;

	default: {
		log_(0, "Not supported message.");
	} break;
	} // End switch
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Construct the IEEE 802.21 driver.
 *
 * @param ios The io_service object that the IEEE 802.21 driver will use to
 * dispatch handlers for any asynchronous operations performed on the socket.
 * @param mag The associated MAG module.
 */
ieee802_21_driver::ieee802_21_driver(boost::asio::io_service& ios, pmip::mag& mag)
	: _service(ios),
	  _mag(mag)
{
}

/**
 * Destruct the IEEE 802.21 driver.
 */
ieee802_21_driver::~ieee802_21_driver()
{
}

/**
 * Start the IEEE 802.21 driver.
 *
 * @param options The input driver options.
 */
void ieee802_21_driver::start(const std::vector<std::string>& options)
{
	decode_options(options);

	_user_sap.reset(new odtone::sap::user(_service, boost::bind(&ieee802_21_driver::event_handler, this, _1, _2)));

	odtone::mih::message msg;
	msg << odtone::mih::indication(odtone::mih::indication::user_register, odtone::mih::id(_local_mihf))
	    & odtone::mih::tlv_mbb_handover_support(true);

	_user_sap->async_send(msg, boost::bind(&ieee802_21_driver::user_reg_handler, this, _1, _2));
}

/**
 * Stop the IEEE 802.21 driver.
 */
void ieee802_21_driver::stop()
{
	_user_sap.reset();
}

////////////////////////////////////////////////////////////////////////////////
} /* namespace app */ } /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////
