//=============================================================================
// Brief   : Link SAP Linux
// Authors : Bruno Santos <bsantos@av.it.pt>
//
//
// Copyright (C) 2009 Universidade Aveiro - Instituto de Telecomunicacoes Polo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/logger.hpp>
#include <opmip/pmip/mag.hpp>
#include "rtnetlink.hpp"
#include <boost/thread/thread.hpp>
#include <iostream>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
void link_sap(opmip::pmip::mag& mag)
{
	rtnetlink nl(rtnetlink::link);
	rtnetlink::message msg;
	opmip::logger log("LINK_SAP", &std::cout);

	while (!boost::this_thread::interruption_requested()) {
		nl.recv(msg);

		do {
			if (rtnetlink::if_link::is(msg)) {
				rtnetlink::if_link lnk(msg);

				std::stringstream info;
				bool do_log = false;

				if (lnk.has_name())
					info << ", name = " << lnk.name();
				if (lnk.has_mtu())
					info << ", mtu = " << lnk.mtu();
				if (lnk.has_lnk_type())
					info << ", link_type = " << lnk.link_type();

				switch (lnk.wireless_event()) {
				case rtnetlink::if_link::we_attach:
					mag.mobile_node_attach(lnk.wireless_address());
					info << ", wireless = { event = attach, address = " << lnk.wireless_address() << "}";
					do_log = true;
					break;

				case rtnetlink::if_link::we_detach:
					mag.mobile_node_detach(lnk.wireless_address());
					info << ", wireless = { event = detach, address = " << lnk.wireless_address() << "}";
					do_log = true;
					break;
				}

				if (do_log)
					log(0, "Link event [type = ", lnk.type(),
						             ", id = ", lnk.index(),
						             ", flags = ", lnk.flags(), info.str(), "]");
			}

		} while (msg.next());
	}
}

// EOF ////////////////////////////////////////////////////////////////////////
