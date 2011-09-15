#include <opmip/pmip/addrconf_server.hpp>

int main(int argc, char* argv[])
{
	using namespace opmip;

	if (argc != 2)
		return -1;

	boost::asio::io_service ios;
	pmip::addrconf_server acs(ios);
	pmip::router_advertisement_info ai;

	ai.device_id = 2;
	ai.link_address = net::link::address_mac::from_string("08:00:27:c3:63:a9");
	ai.dst_link_address = net::link::address_mac::from_string(argv[1]);
	ai.home_addr = net::ip::address_v6::from_string("2001:abcd:ef01:1234::1");

	acs.start();
	acs.add(ai);

	ios.run();
}
