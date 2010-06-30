//=============================================================================
// Brief   : Link Layer MAC Address
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

#include <opmip/ll/mac_address.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ll {

///////////////////////////////////////////////////////////////////////////////
//FIXME: move this somewhere else
static uchar hex_to_int(uchar c)
{
	if (c >= 'a')
		return c - 'a' + 0xA;

	if (c >= 'A')
		return c - 'A' + 0xA;

	return c - '0';
}

///////////////////////////////////////////////////////////////////////////////
mac_address mac_address::from_string(const char* str)
{
	mac_address mac;

	for (uint i = 0; i < bytes_type::static_size; ++i) {
		uint8 tmp[2];

		if (!std::isxdigit(*str))
			break;
		tmp[0] = hex_to_int(*(str++));

		if (!std::isxdigit(*str))
			break;
		tmp[1] = hex_to_int(*(str++));

		if (*str != ':')
			break;
		++str;

		mac._address[i] = (tmp[0] << 4) | tmp[1];
	}

	if (*str != '\0')
		return mac_address(); //FIXME

	return mac;
}

mac_address mac_address::from_string(const std::string& str)
{
	return from_string(str.c_str());
}

mac_address::bytes_type mac_address::to_bytes() const
{
	return _address;
}

std::string mac_address::to_string() const
{
	std::string str;

	for (uint i = 0; i < bytes_type::static_size; ++i) {
		char tmp[4];

		tmp[0] = _address[i] >> 4;
		tmp[1] = _address[i] & 0x0f;
		tmp[2] = ':';
		tmp[3] = '\0';

		BOOST_ASSERT(!(tmp[0] & 0xf0) && !(tmp[1] & 0xf0));

		if (tmp[0] < 10)
			tmp[0] += '0';
		else
			tmp[0] += 'a' - 10;

		if (tmp[1] < 10)
			tmp[1] += '0';
		else
			tmp[1] += 'a' - 10;

		str.append(tmp);
	}
	str.resize(str.length() - 1);

	return str;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ll */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
