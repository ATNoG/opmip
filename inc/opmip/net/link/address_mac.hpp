//=============================================================================
// Brief   : Link Layer MAC Address
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
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
//=============================================================================

#ifndef OPMIP_NET_LINK_ADDRESS_MAC__HPP_
#define OPMIP_NET_LINK_ADDRESS_MAC__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/array.hpp>
#include <utility>
#include <string>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace link {

///////////////////////////////////////////////////////////////////////////////
class address_mac {
public:
	typedef boost::array<uint8, 6> bytes_type;

public:
	static address_mac from_string(const char* str);
	static address_mac from_string(const std::string& str);

public:
	address_mac()
	{
		_address.assign(0);
	}

	explicit address_mac(const uint8 address[6])
	{
		std::copy(address, address + 6, _address.begin());
	}

	address_mac(const void* address, size_t len)
	{
		const uint8* src = reinterpret_cast<const uint8*>(address);

		std::copy(src, src + std::min<size_t>(len, bytes_type::static_size), _address.begin());
	}

	explicit address_mac(const bytes_type& address)
	{
		std::copy(address.begin(), address.end(), _address.begin());
	}

	bool operator!()
	{
		for (bytes_type::iterator i = _address.begin(), e = _address.end(); i != e; ++i) {
			if (*i)
				return false;
		}
		return true;
	}

	bool operator<(const address_mac& lhs) const
	{
		return _address < lhs._address;
	}

	std::string       to_string() const;
	const bytes_type& to_bytes() const { return _address; }

	friend std::ostream& operator<<(std::ostream& out, const address_mac& mac)
	{
		return out << mac.to_string();
	}

public:
	bytes_type _address;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace link */ } /* namespace net */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_NET_LINK_ADDRESS_MAC__HPP_ */
