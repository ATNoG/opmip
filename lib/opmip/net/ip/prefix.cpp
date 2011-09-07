//=============================================================================
// Brief   : IP Address Prefix
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

#include <opmip/net/ip/prefix.hpp>
#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip {

///////////////////////////////////////////////////////////////////////////////
prefix_v6 prefix_v6::from_string(const std::string& str)
{
	size_t pos = str.rfind('/');

	if (pos == std::string::npos)
		return prefix_v6(); //FIXME: throw_exception

	address_v6 addr = address_v6::from_string(str.substr(0, pos));
	uint       plen = boost::lexical_cast<uint>(str.substr(pos + 1));

	return prefix_v6(addr, plen);
}

prefix_v6::prefix_v6()
{
	_prefix.assign(0);
	_length = 0;
}

prefix_v6::prefix_v6(const bytes_type& addr, uint length)
{
	if (length > 128) {
		_prefix.assign(0);
		_length = 0;

	} else {
		_prefix = addr;
		_length = static_cast<uchar>(length);
	}
}

prefix_v6::prefix_v6(const address_v6& addr, uint length)
{
	if (length > 128) {
		_prefix.assign(0);
		_length = 0;

	} else {
		_prefix = addr.to_bytes();
		_length = static_cast<uchar>(length);
	}
}

std::ostream& operator<<(std::ostream& out, const prefix_v6& lhr)
{
	return out << address_v6(lhr._prefix)
	           << '/' << static_cast<uint>(lhr._length);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace net */ } /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
