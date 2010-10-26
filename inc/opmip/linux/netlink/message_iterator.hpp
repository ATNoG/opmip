//=============================================================================
// Brief   : Netlink Message Iterator
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

#ifndef OPMIP_LINUX_NETLINK_MESSAGE_ITERATOR__HPP_
#define OPMIP_LINUX_NETLINK_MESSAGE_ITERATOR__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/linux/netlink/message.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux { namespace netlink {

///////////////////////////////////////////////////////////////////////////////
class message_iterator {
	typedef message::header header;

	OPMIP_UNDEFINED_BOOL;

public:
	message_iterator()
		: _header(0)
	{ }

	message_iterator(void* buffer, size_t length)
		: _header(header::cast(buffer, length)), _length(length)
	{ }

	uint16  type()       { return _header->type; }
	header* get()        { return _header; }
	header* operator->() { return _header; }

	int error()
	{
		if (_header->type != message::error)
			return 0;

		uint8* p = reinterpret_cast<uint8*>(_header) + align_to_<4, sizeof(header)>::value;

		return -(*reinterpret_cast<int*>(p));
	}

	message_iterator& operator++()
	{
		uchar* next = reinterpret_cast<uchar*>(_header) + align_to<4>(_header->length);
		size_t len  = _length - std::min<size_t>(align_to<4>(_header->length), _length);

		BOOST_ASSERT(_header);

		_header = header::cast(next, len);
		_length = len;

		return *this;
	}

	message_iterator operator++(int)
	{
		message_iterator tmp(*this);

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
#endif /* OPMIP_LINUX_NETLINK_MESSAGE_ITERATOR__HPP_ */
