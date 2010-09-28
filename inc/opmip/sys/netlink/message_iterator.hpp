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

#ifndef OPMIP_SYS_NETLINK_MESSAGE_ITERATOR__HPP_
#define OPMIP_SYS_NETLINK_MESSAGE_ITERATOR__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/netlink/header.hpp>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace nl {

///////////////////////////////////////////////////////////////////////////////
class message_iterator {
public:
	message_iterator()
		: _header(0)
	{ }

	message_iterator(void* buffer, size_t length)
		: _header(header::cast(buffer, length)), _length(length)
	{ }

	header& operator*()  { return *_header; }
	header* operator->() { return _header; }

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

	friend bool operator==(const message_iterator& rhs, const message_iterator& lhs)
	{
		return rhs._header == lhs._header;
	}

	friend bool operator!=(const message_iterator& rhs, const message_iterator& lhs)
	{
		return rhs._header != lhs._header;
	}

private:
	header* _header;
	size_t  _length;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace nl */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_NETLINK_MESSAGE_ITERATOR__HPP_ */
