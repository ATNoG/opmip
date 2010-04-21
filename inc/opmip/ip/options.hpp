//=============================================================================
// Brief   : IPv6 Options Base Class
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

#ifndef OPMIP_IP_OPTIONS__HPP_
#define OPMIP_IP_OPTIONS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
template<uint8 TypeValue>
class option {
protected:
	option(uint8 length)
		: _type(TypeValue), _length((length / 8) - 1)
	{
		BOOST_ASSERT(((_length + 1) * 8) == length);
	}

public:
	static const uint8 type_value = TypeValue;

	uint8 length() const { return TypeValue; }

protected:
	const uint8 _type;
	uint8       _length;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_OPTIONS__HPP_ */
