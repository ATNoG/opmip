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

#include <opmip/ip/prefix.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
prefix_v6::prefix_v6()
{
	_prefix.assign(0);
	_length = 0;
}

prefix_v6::prefix_v6(const address_v6& addr, uint length)
{
	if (length > 128) {
		_prefix.assign(0);
		_length = 0;

	} else {
		const uint nbits = length % 8;
		const uint nbytes = length / 8 + (nbits ? 1 : 0);

		_prefix = addr.to_bytes();
		std::fill(_prefix.begin() + nbytes, _prefix.end(), 0);
		if (nbits)
			_prefix[nbytes - 1] &= (static_cast<uint8>(~0) << nbits);
		_length = static_cast<uchar>(length);
	}
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
