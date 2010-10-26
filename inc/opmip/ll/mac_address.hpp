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

#ifndef OPMIP_LL_MAC_ADDRESS__HPP_
#define OPMIP_LL_MAC_ADDRESS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/array.hpp>
#include <utility>
#include <string>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ll {

///////////////////////////////////////////////////////////////////////////////
class mac_address {
public:
	typedef boost::array<uint8, 6> bytes_type;

public:
	static mac_address from_string(const char* str);
	static mac_address from_string(const std::string& str);

public:
	mac_address()
	{
		_address.assign(0);
	}

	explicit mac_address(const uint8 address[6])
	{
		std::copy(address, address + 6, _address.begin());
	}

	explicit mac_address(const bytes_type& address)
	{
		std::copy(address.begin(), address.end(), _address.begin());
	}

	bool operator<(const mac_address& lhs) const
	{
		return _address < lhs._address;
	}

	std::string to_string() const;
	bytes_type  to_bytes() const;

	friend std::ostream& operator<<(std::ostream& out, const mac_address& mac)
	{
		return out << mac.to_string();
	}

public:
	bytes_type _address;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ll */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LL_MAC_ADDRESS__HPP_ */
