#include <opmip/conf.hpp>
#include <fstream>
#include <string>

void test(uint value, opmip::conf::pset_type const& pset)
{
	opmip::conf::pset_type::const_iterator end = pset.end();
	opmip::conf::pset_type::const_iterator itr;

	std::cout << "test("
	          << value
	          << ") ";

	itr = pset.find("ip");
	if (itr != end)
		std::cout << '{' << boost::get<opmip::net::ip::address_v6>(itr->second[0]) << "\n}";

	std::cout << std::endl;
}

static const opmip::conf::property_class test_pset[] = {
	{ "ip", opmip::conf::type_id_<opmip::net::ip::address_v6>::value, 1 }
};

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cout << "usage: " << argv[0] << " <input-file>\n";
		return 1;
	}

	std::ifstream in(argv[1]);

	if (!in) {
		std::cout << "could not open: " << argv[1] << std::endl;
		return 1;
	}

	std::string buf((std::istreambuf_iterator<char>(in)),
	                 std::istreambuf_iterator<char>());

	std::string::const_iterator it(buf.begin());

	try {
		opmip::conf::functions cm;

		cm["test"] = opmip::conf::function(&test, test_pset);

		return !opmip::conf::exec(it, buf.end(), cm);
	} catch (std::exception& e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
