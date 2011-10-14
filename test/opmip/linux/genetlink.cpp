#include <opmip/linux/netlink.hpp>
#include <opmip/linux/netlink/message_iterator.hpp>
#include <opmip/linux/netlink/attribute_iterator.hpp>
#include <opmip/linux/genetlink.hpp>
#include <opmip/linux/genetlink/message.hpp>
#include <opmip/linux/nl80211.hpp>
#include <iostream>
#include <cstdio>

void event_handler(const boost::system::error_code& ec, const opmip::linux::nl80211::event& ev)
{
	std::cout << "event = {\n"
				 "    .phy = " << ev.phy_id << "\n"
				 "    .if_index = " << ev.if_index << "\n"
				 "    .generation = " << ev.generation << "\n"
				 "    .mac = " << ev.mac << "\n"
				 "    .frame = {\n"
				 "        .da       = " << ev.dst_addr << "\n"
				 "        .sa       = " << ev.src_addr << "\n"
				 "        .bssid    = " << ev.bssid << "\n"
				 "     }\n"
				 "}\n";
}

int main()
{
	boost::asio::io_service ios;
	opmip::linux::nl80211 nl80211(ios);

	nl80211.open(event_handler);
	ios.run();
}
