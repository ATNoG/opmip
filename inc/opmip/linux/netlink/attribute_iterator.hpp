//=============================================================================
// Brief   : Netlink Attribute
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

#ifndef OPMIP_LINUX_NETLINK_ATTRIBUTE__HPP_
#define OPMIP_LINUX_NETLINK_ATTRIBUTE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/linux/netlink/message.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux { namespace netlink {

///////////////////////////////////////////////////////////////////////////////
class attribute_iterator {
	typedef message::attr_header header;

	OPMIP_UNDEFINED_BOOL;

public:
	attribute_iterator()
		: _header(0)
	{ }

	explicit attribute_iterator(message& msg)
	{
		void* buff = offset_cast<void*>(msg._header, msg._length);
		size_t len = remaining_length(msg._header->length , msg._length);

		_header = header::cast(buff, len);
		_length = len;
	}

	explicit attribute_iterator(message& /*msg*/, attribute_iterator& ai)
	{
		_header = header::cast(ai.get<header>(), ai.length());
		_length = ai.length();
	}

	uint16 type() const   { return _header->type; }
	size_t length() const { return _header->length - align_to_<4, sizeof(header)>::value; }

	template<class T>
	T* get()
	{
		uchar* data = reinterpret_cast<uchar*>(_header) + align_to_<4, sizeof(header)>::value;

		return reinterpret_cast<T*>(data);
	}

	attribute_iterator& operator++()
	{
		uchar* next = reinterpret_cast<uchar*>(_header) + align_to<4>(_header->length);
		size_t len  = _length - std::min<size_t>(align_to<4>(_header->length), _length);

		BOOST_ASSERT(_header);

		_header = header::cast(next, len);
		_length = len;

		return *this;
	}

	attribute_iterator operator++(int)
	{
		attribute_iterator tmp(*this);

		this->operator++();
		return tmp;
	}

	operator undefined_bool()
	{
		return !_header ? OPMIP_UNDEFINED_BOOL_FALSE : OPMIP_UNDEFINED_BOOL_TRUE;
	}

private:
	header* _header;
	size_t  _length;
};


///////////////////////////////////////////////////////////////////////////////
} /* namespace netlink */ } /* namespace linux */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LINUX_NETLINK_ATTRIBUTE__HPP_ */
