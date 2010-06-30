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
class option {
public:
	static option* cast(void* data, size_t len)
	{
		option* opt = reinterpret_cast<option*>(data);

		if (len < sizeof(option) || len < (opt->_length * 8))
			return nullptr;

		return opt;
	}

	template<class T>
	static T* cast(option* opt)
	{
		if (!opt || T::type_value != opt->_type || T::static_size > (opt->_length * 8))
			return nullptr;

		return static_cast<T*>(opt);
	}

	static uint8 type(const option* opt)
	{
		return opt->_type;
	}

	static size_t size(const option* opt)
	{
		return opt->_length * 8;
	}

public:
	option(uint8 type, uint8 length)
		: _type(type), _length(length / 8)
	{
		BOOST_ASSERT((_length * 8) == length);
	}

protected:
	uint8 _type;
	uint8 _length;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_OPTIONS__HPP_ */
