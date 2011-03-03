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

#ifndef OPMIP_LINUX_NETLINK_MESSAGE__HPP_
#define OPMIP_LINUX_NETLINK_MESSAGE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/exception.hpp>
#include <boost/utility.hpp>
#include <boost/asio/buffer.hpp>
#include <cstdlib>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace linux { namespace netlink {

///////////////////////////////////////////////////////////////////////////////
class message : boost::noncopyable {
protected:
	struct header {
		static header*       cast(void* buffer, std::size_t length);
		static const header* cast(const void* buffer, std::size_t length);

		uint32 length;   ///Length of message including header
		uint16 type;     ///Message content
		uint16 flags;    ///Additional flags
		uint32 sequence; ///Sequence number
		uint32 port_id;  ///Sending process port ID
	};

	struct attr_header {
		uint16 length;
		uint16 type;

		static attr_header* cast(void* buffer, size_t length)
		{
			attr_header* tmp = reinterpret_cast<attr_header*>(buffer);

			if ((length < align_to_<4, sizeof(attr_header)>::value) || !tmp->type || (length < tmp->length))
				return nullptr;

			return tmp;
		}
	};

	friend class message_iterator;
	friend class attribute_iterator;

	OPMIP_UNDEFINED_BOOL;

public:
	enum mtype {
		noop  = 1,
		error,
		done,
		overrun,

		min_type = 0x10
	};

	enum mflags {
		request   = 1, ///It is request message
		multipart = 2, ///Multipart message, terminated by NLMSG_DONE
		ack       = 4, ///Reply with ack, with zero or error code
		echo      = 8, ///Echo this request
	};

	enum get_flags {
		root   = 0x100, ///Specify tree root
		match  = 0x200, ///Return all matching
		atomic = 0x400, ///Atomic GET
		dump   = root | match
	};

	enum new_flags {
		replace   = 0x100, ///Override existing
		exclusive = 0x200, ///Do not touch, if it exists
		create    = 0x400, ///Create, if it does not exist
		append    = 0x800, ///Add to end of list
	};

protected:
	message(size_t len)
		: _header(nullptr), _length(0), _capacity(0)
	{
		BOOST_ASSERT(len >= sizeof(header));

		alloc(len);
	}

	explicit message(header* hdr, size_t attr_offset)
		: _header(hdr), _length(attr_offset), _capacity(0)
	{ }

public:
	message()
		: _header(nullptr), _length(0), _capacity(0)
	{
		alloc(sizeof(header));
	}

	~message()
	{
		if (_capacity)
			std::free(_header);
	}

	uint16 type() const     { return _header->type; }
	uint16 flags() const    { return _header->flags; }
	uint32 sequence() const { return _header->sequence; }
	uint32 port_id() const  { return _header->port_id; }

	void type(uint16 val)     { _header->type = val; }
	void flags(uint16 val)    { _header->flags = val; }
	void sequence(uint32 val) { _header->sequence = val; }
	void port_id(uint32 val)  { _header->port_id = val; }

	void push_attr(uint16 type, const void* data, size_t length)
	{
		const size_t len = align_to<4>(sizeof(attr_header)) + length;
		attr_header* hdr = reinterpret_cast<attr_header*>(alloc(len));
		const uchar* src = reinterpret_cast<const uchar*>(data);
		uchar*       dst = reinterpret_cast<uchar*>(hdr) + align_to<4>(sizeof(attr_header));

		hdr->length = len;
		hdr->type = type;
		std::copy(src, src + length, dst);
	}

	boost::asio::const_buffers_1 cbuffer() const { return boost::asio::const_buffers_1(_header, _length); }

protected:
	uint8* alloc(size_t len)
	{
		BOOST_ASSERT(((_header && _capacity) || (!_header && !_capacity)));

		size_t capacity = align_to<512>(_length + len);
		uint8* mem = reinterpret_cast<uint8*>(_header);

		if (capacity > _capacity) {
			mem = reinterpret_cast<uint8*>(std::realloc(mem, capacity));

			if (!mem)
				boost::throw_exception(std::bad_alloc());

			std::fill(mem + _capacity, mem + capacity, 0);
			_header = reinterpret_cast<header*>(mem);
			_capacity = capacity;
		}

		mem += _length;
		_header->length = _length + len;
		_length += align_to<4>(len);

		return mem;
	}

private:
	header* _header;
	uint    _length;
	uint    _capacity;
};

inline message::header* message::header::cast(void* buffer, std::size_t length)
{
	header* tmp = reinterpret_cast<header*>(buffer);

	if ((length < sizeof(header)) || (length < tmp->length))
		return nullptr;

	return tmp;
}

inline const message::header* message::header::cast(const void* buffer, std::size_t length)
{
	const header* tmp = reinterpret_cast<const header*>(buffer);

	if ((length < sizeof(header)) || (length < tmp->length))
		return nullptr;

	return tmp;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace netlink */ } /* namespace linux */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LINUX_NETLINK_MESSAGE__HPP_ */
