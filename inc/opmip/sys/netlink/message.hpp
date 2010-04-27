//=============================================================================
// Brief   : Netlink Message Template
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

#ifndef OPMIP_SYS_NETLINK_MESSAGE__HPP_
#define OPMIP_SYS_NETLINK_MESSAGE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/exception.hpp>
#include <opmip/sys/netlink/header.hpp>
#include <opmip/sys/netlink/message_iterator.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/utility.hpp>
#include <algorithm>
#include <cstdlib>
#include <new>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace nl {

///////////////////////////////////////////////////////////////////////////////
template<class Message>
class message : boost::noncopyable {
	struct frame {
		header  hdr;
		Message msg;
	};

	struct attr_header {
		uint16 length;
		uint16 type;

		attr_header* cast(void* buffer, std::size_t length)
		{
			attr_header* tmp = reinterpret_cast<attr_header*>(buffer);

			if ((length < align_to_<4, sizeof(attr_header)>::value) || (length < tmp->length))
				return nullptr;

			return tmp;
		}
	};

public:
	typedef Message message_type;

	class attr_iterator {
	public:
		attr_iterator()
			: _header(0)
		{ }

		attr_iterator(void* buffer, size_t length)
			: _header(header::cast(buffer, length)), _length(length)
		{ }

		const attr_header& operator*()  const { return *_header; }
		const attr_header* operator->() const { return _header; }

		template<class T>
		T* get()
		{
			uchar* data = reinterpret_cast<uchar*>(_header) + align_to_<4, sizeof(attr_header)>::value;

			return reinterpret_cast<T*>(data);
		}

		size_t length() const
		{
			return _header->length - align_to_<4, sizeof(attr_header)>::value;
		}

		attr_iterator& operator++()
		{
			uchar* next = reinterpret_cast<uchar*>(_header) + align_to<4>(_header->length);
			size_t len  = _length - (std::min)(align_to<4, size_t>(_header->length), _length);

			BOOST_ASSERT(_header);

			_header = header::cast(next, len);
			_length = len;

			return *this;
		}

		attr_iterator operator++(int)
		{
			attr_iterator tmp(*this);

			this->operator++();
			return tmp;
		}

		friend bool operator==(const attr_iterator& rhs, const attr_iterator& lhs)
		{
			return rhs._header == lhs._header;
		}

		friend bool operator!=(const attr_iterator& rhs, const attr_iterator& lhs)
		{
			return rhs._header != lhs._header;
		}

	private:
		attr_header* _header;
		size_t       _length;
	};

public:
	message()
		: _frame(nullptr), _length(0), _capacity(0)
	{
		const size_t len = sizeof(frame);

		alloc(len);
		new(_frame) frame();
		_frame->hdr.length = len;
	}

	explicit message(message_iterator& mit)
		: _frame(nullptr), _length(0), _capacity(0)
	{
		if (mit->type >= message_type::m_begin && mit->type < message_type::m_end)
			_frame = reinterpret_cast<frame*>(mit.operator->());
	}

	~message()
	{
		if (_capacity)
			std::free(_frame);
	}

	uint32 mtype() const    { return _frame->hdr.type; }
	uint16 flags() const    { return _frame->hdr.flags; }
	uint32 sequence() const { return _frame->hdr.sequence; }
	uint32 port_id() const  { return _frame->hdr.port_id; }

	void mtype(uint32 val)    { _frame->hdr.type = val; }
	void flags(uint16 val)    { _frame->hdr.flags = val; }
	void sequence(uint32 val) { _frame->hdr.sequence = val; }
	void port_id(uint32 val)  { _frame->hdr.port_id = val; }

	message_type& operator*()  { return _frame->msg; }
	message_type* operator->() { return &_frame->msg; }

	const message_type& operator*() const  { return _frame->msg; }
	const message_type* operator->() const { return &_frame->msg; }

	void push_attribute(typename message_type::attr_type type, const void* data, size_t length)
	{
		const size_t len = align_to<4>(sizeof(attr_header)) + length;
		attr_header* hdr = reinterpret_cast<attr_header*>(alloc(len));
		const uchar* src = reinterpret_cast<const uchar*>(data);
		uchar*       dst = reinterpret_cast<uchar*>(hdr) + align_to<4>(sizeof(attr_header));

		_frame->hdr.length = align_to<4>(_frame->hdr.length) + len;

		hdr->length = len;
		hdr->type = type;
		std::copy(src, src + length, dst);
	}

	attr_iterator abegin()
	{
		void* buff = reinterpret_cast<uchar*>(_frame) + align_to_<4, sizeof(frame)>::value;
		size_t len = _length - align_to_<4, sizeof(frame)>::value;

		return attr_iterator(buff, len);
	}

	boost::asio::const_buffers_1 cbuffer() const { return boost::asio::const_buffers_1(_frame, _length); }

private:
	uchar* alloc(size_t len)
	{
		BOOST_ASSERT(((_frame && _capacity) || (!_frame && !_capacity)));

		size_t capacity = align_to<512>(_length + len);
		uchar* mem = reinterpret_cast<uchar*>(_frame);

		if (capacity > _capacity) {
			mem = reinterpret_cast<uchar*>(std::realloc(mem, capacity));

			if (!mem)
				throw_exception(std::bad_alloc());

			std::fill(mem + _capacity, mem + capacity, 0);
			_frame = reinterpret_cast<frame*>(mem);
			_capacity = capacity;
		}

		mem += _length;
		_length += align_to<4>(len);

		return mem;
	}

private:
	frame* _frame;
	uint   _length;
	uint   _capacity;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace nl */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_NETLINK_MESSAGE__HPP_ */
