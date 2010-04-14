//=============================================================================
// Brief   : Mobility Protocol
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

#ifndef OPMIP_IP_MPROTO__HPP_
#define OPMIP_IP_MPROTO__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/error.hpp>
#include <opmip/ip/address.hpp>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/raw_socket_service.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
class mproto {
public:
	class endpoint;
	class socket;
	class header;
	class pbu;
	class pba;

	enum mh_types {
		mh_pbu = 5,
		mh_pba = 6,
	};

public:
	int family() const   { return AF_INET6; }
	int type() const     { return SOCK_RAW; }
	int protocol() const { return 135; }
};

///////////////////////////////////////////////////////////////////////////////
class mproto::endpoint {
public:
	typedef mproto protocol_type;

public:
	endpoint();
	endpoint(const address_v6& addr);

	protocol_type     protocol() const { return protocol_type(); }
	::sockaddr*       data()           { return reinterpret_cast< ::sockaddr*>(&_addr); }
	const ::sockaddr* data() const     { return reinterpret_cast<const ::sockaddr*>(&_addr); }
	size_t            size() const     { return sizeof(_addr); }
	size_t            capacity() const { return sizeof(_addr); }
	address_v6        address() const;

	void resize(size_t len) { BOOST_ASSERT(len == sizeof(_addr)); }
	void address(const address_v6& addr);

private:
	::sockaddr_in6 _addr;
};

inline mproto::endpoint::endpoint()
{
	::in6_addr tmp = IN6ADDR_ANY_INIT;

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = 0;
	_addr.sin6_flowinfo = 0;
	_addr.sin6_addr = tmp;
	_addr.sin6_scope_id = 0;
}

inline mproto::endpoint::endpoint(const address_v6& addr)
{
	address_v6::bytes_type& tmp = reinterpret_cast<address_v6::bytes_type&>(_addr.sin6_addr.s6_addr);

	_addr.sin6_family = AF_INET6;
	_addr.sin6_port = 0;
	_addr.sin6_flowinfo = 0;
	tmp = addr.to_bytes();
	_addr.sin6_scope_id = addr.scope_id();
}

inline address_v6 mproto::endpoint::address() const
{
	return address_v6(reinterpret_cast<const address_v6::bytes_type&>(_addr.sin6_addr.s6_addr), _addr.sin6_scope_id);
}

inline void mproto::endpoint::address(const address_v6& addr)
{
	address_v6::bytes_type& tmp = reinterpret_cast<address_v6::bytes_type&>(_addr.sin6_addr.s6_addr);

	tmp = addr.to_bytes();
	_addr.sin6_scope_id = addr.scope_id();
}

///////////////////////////////////////////////////////////////////////////////
struct mproto::header {
	static const size_t mh_size = 6;

	void init(uint8 type, size_t len)
	{
		len += mh_size;

		next = 59;
		length = (align_to<8>(len) / 8) - 1;
		mh_type = type;
		reserved = 0;
		checksum = 0;
		plen = len % 8;
		pad.assign(0);

		update(this, mh_size);
	}

	void update(const void* data, size_t len)
	{
		BOOST_ASSERT(len == align_to<2>(len));
		const size_t cnt = len / 2;
		uint sum = checksum;

		for (size_t i = 0; i < cnt; ++i)
			sum += reinterpret_cast<const uint16*>(data)[i];

		sum += (sum >> 16) & 0xffff;
		sum += (sum >> 16);
		checksum = sum;
	}

	void finalize()
	{
		checksum = ~checksum;
	}

	boost::asio::const_buffer pad_buffer() const
	{
		return boost::asio::const_buffer(pad.elems, plen);
	}


	uint8  next;
	uint8  length;
	uint8  mh_type;
	uint8  reserved;
	uint16 checksum;

	uint8                  plen;
	boost::array<uint8, 7> pad;
};

///////////////////////////////////////////////////////////////////////////////
class mproto::pbu {
public:
	static const size_t mh_type = 5;
	static const size_t mh_size = 6;

public:
	static pbu* cast(void* buff, size_t len)
	{
		if (len < sizeof(pbu))
			return nullptr;

		return reinterpret_cast<pbu*>(buff);
	}

	static const pbu* cast(const void* buff, size_t len)
	{
		if (len < sizeof(pbu))
			return nullptr;

		return reinterpret_cast<const pbu*>(buff);
	}

public:
	pbu()
		: _sequence(0), _flags1(0), _flags2(0), _lifetime(0)
	{ }

	uint16 sequence() const { return ::ntohs(_sequence); }
	bool   a() const;
	bool   h() const;
	bool   l() const;
	bool   k() const;
	bool   m() const;
	bool   r() const;
	bool   p() const;
	uint16 lifetime() const { return ::ntohs(_sequence); }

	void sequence(uint16 value) { _sequence = ::ntohs(value); }
	void a(bool value);
	void h(bool value);
	void l(bool value);
	void k(bool value);
	void m(bool value);
	void r(bool value);
	void p(bool value);
	void lifetime(uint16 value) { _lifetime = ::ntohs(value); }

	const void* data() const
	{
		return this;
	}

private:
	uint16 _sequence;
	uint8  _flags1;
	uint8  _flags2;
	uint16 _lifetime;
};

bool mproto::pbu::a() const
{
	const uint8 v = 1u << 7;

	return _flags1 & v;
}

bool mproto::pbu::h() const
{
	const uint8 v = 1u << 6;

	return _flags1 & v;
}

bool mproto::pbu::l() const
{
	const uint8 v = 1u << 5;

	return _flags1 & v;
}

bool mproto::pbu::k() const
{
	const uint8 v = 1u << 4;

	return _flags1 & v;
}

bool mproto::pbu::m() const
{
	const uint8 v = 1u << 3;

	return _flags1 & v;
}

bool mproto::pbu::r() const
{
	const uint8 v = 1u << 2;

	return _flags1 & v;
}

bool mproto::pbu::p() const
{
	const uint8 v = 1u << 1;

	return _flags1 & v;
}

void mproto::pbu::a(bool value)
{
	const uint8 v = 1u << 7;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::h(bool value)
{
	const uint8 v = 1u << 6;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::l(bool value)
{
	const uint8 v = 1u << 5;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::k(bool value)
{
	const uint8 v = 1u << 4;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::m(bool value)
{
	const uint8 v = 1u << 3;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::r(bool value)
{
	const uint8 v = 1u << 2;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

void mproto::pbu::p(bool value)
{
	const uint8 v = 1u << 1;

	if (value)
		_flags1 |= v;
	else
		_flags1 &= ~v;
}

///////////////////////////////////////////////////////////////////////////////
class mproto::pba {
public:
	static const size_t mh_type = 6;
	static const size_t mh_size = 6;

	enum status_type {
		status_ok                     = 0,   ///Accepted
		status_ok_needs_prefix        = 1,   ///Accepted but prefix discovery necessary
		status_unspecified            = 128, ///Reason unspecified
		status_prohibited             = 129, ///Administratively prohibited
		status_insufficient_resources = 130, ///Insufficient resources
		status_hr_not_supported       = 131, ///Home registration not supported
		status_not_home_subnet        = 132, ///Not home subnet
		status_not_home_agent         = 133, ///Not home agent for this mobile node
		status_duplicate_address      = 134, ///Duplicate Address Detection failed
		status_bad_sequence           = 135, ///Sequence number out of window
		status_expired_home           = 136, //Expired home nonce index
		status_expired_care_of        = 137, ///Expired care-of nonce index
		status_expired                = 138, ///Expired nonces
		status_invalid_registration   = 139, ///Registration type change disallowed
	};

public:
	static pba* cast(void* buff, size_t len)
	{
		if (len < mh_size)
			return nullptr;

		return reinterpret_cast<pba*>(buff);
	}

	static const pba* cast(const void* buff, size_t len)
	{
		if (len < mh_size)
			return nullptr;

		return reinterpret_cast<const pba*>(buff);
	}

public:
	pba()
		: _status(0), _flags(0), _sequence(0), _lifetime(0)
	{ }

	status_type status() const   { return status_type(_status); }
	bool        k() const;
	bool        r() const;
	bool        p() const;
	uint16      sequence() const { return ::ntohs(_sequence); }
	uint16      lifetime() const { return ::ntohs(_sequence); }

	void status(status_type value) { _status = value; }
	void k(bool value);
	void r(bool value);
	void p(bool value);
	void sequence(uint16 value)  { _sequence = ::ntohs(value); }
	void lifetime(uint16 value)  { _lifetime = ::ntohs(value); }

	const void* data() const
	{
		return this;
	}

private:
	uint8  _status;
	uint8  _flags;
	uint16 _sequence;
	uint16 _lifetime;
};

bool mproto::pba::k() const
{
	const uint8 v = 1u << 7;

	return _flags & v;
}

bool mproto::pba::r() const
{
	const uint8 v = 1u << 6;

	return _flags & v;
}

bool mproto::pba::p() const
{
	const uint8 v = 1u << 6;

	return _flags & v;
}

void mproto::pba::k(bool value)
{
	const uint8 v = 1u << 7;

	if (value)
		_flags |= v;
	else
		_flags &= ~v;
}

void mproto::pba::r(bool value)
{
	const uint8 v = 1u << 6;

	if (value)
		_flags |= v;
	else
		_flags &= ~v;
}

void mproto::pba::p(bool value)
{
	const uint8 v = 1u << 5;

	if (value)
		_flags |= v;
	else
		_flags &= ~v;
}

///////////////////////////////////////////////////////////////////////////////
class mproto::socket
	: public boost::asio::basic_socket<mproto,
	                                   boost::asio::raw_socket_service<mproto> > {

	typedef boost::asio::basic_socket<mproto,
		                              boost::asio::raw_socket_service<mproto> > base;

	typedef boost::asio::socket_base ocket_base;

	template<class WriteHandler>
	struct write_handler_impl {
		write_handler_impl(WriteHandler handler)
			: _handler(handler)
		{ }

		void operator()(const boost::system::error_code& ec, size_t bwritten) const
		{
			_handler(ec, (bwritten < mproto::header::mh_size) ? 0 : (bwritten - mproto::header::mh_size));
		}

		WriteHandler _handler;
	};

	template<class MutableBufferSequence, class ReadHandler>
	struct read_handler_impl {
		read_handler_impl(const MutableBufferSequence& buffers, ReadHandler handler)
			: _buffers(buffers), _handler(handler)
		{ }

		void operator()(const boost::system::error_code& ec, size_t bread) const
		{
			if (!ec && !_buffers.checksum(bread))
				_handler(boost::system::error_code(boost::system::errc::bad_message,
				                                   boost::system::get_generic_category()),
				         static_cast<size_t>(0), static_cast<mproto::mh_types>(-1));
			else
				_handler(ec, (bread < mproto::header::mh_size) ? 0 : (bread - mproto::header::mh_size), _buffers.mh_type());
		}

		MutableBufferSequence _buffers;
		ReadHandler           _handler;
	};

public:
	typedef base::native_type           native_type;
	typedef mproto                      protocol_type;
	typedef mproto::endpoint            endpoint_type;
	typedef std::pair<size_t, mh_types> receive_type;

public:
	socket(boost::asio::io_service& ios)
		: base(ios)
	{ }
	socket(boost::asio::io_service& ios, const endpoint_type& ep)
		: base(ios, ep)
	{ }
	socket(boost::asio::io_service& ios, const native_type& sock)
		: base(ios, mproto(), sock)
	{ }

	template<class ConstBufferSequence>
	size_t send(const ConstBufferSequence& buffers)
	{
		boost::system::error_code ec;

	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		std::size_t s = this->service.send(this->implementation, buffers, 0, ec);
		sys::throw_on_error(ec);
		return (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
	}

	template<class ConstBufferSequence>
	size_t send(const ConstBufferSequence& buffers, socket_base::message_flags flags)
	{
		boost::system::error_code ec;

	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		size_t s = this->service.send(this->implementation, buffers, flags, ec);
		sys::throw_on_error(ec);
		return (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
	}

	template<class ConstBufferSequence>
	size_t send(const ConstBufferSequence& buffers, socket_base::message_flags flags, boost::system::error_code& ec)
	{
	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		size_t s = this->service.send(this->implementation, buffers, flags, ec);
		return (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
	}

	template<class ConstBufferSequence, class WriteHandler>
	void async_send(const ConstBufferSequence& buffers, WriteHandler handler)
	{
	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		this->service.async_send(this->implementation, buffers, 0, write_handler_impl<WriteHandler>(handler));
	}

	template<class ConstBufferSequence, class WriteHandler>
	void async_send(const ConstBufferSequence& buffers, socket_base::message_flags flags, WriteHandler handler)
	{
	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		this->service.async_send(this->implementation, buffers, flags, write_handler_impl<WriteHandler>(handler));
	}

	template<class ConstBufferSequence>
	size_t send_to(const ConstBufferSequence& buffers, const endpoint_type& destination)
	{
		boost::system::error_code ec;

	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		size_t s = this->service.send_to(this->implementation, buffers, destination, 0, ec);
		sys::throw_on_error(ec);
		return (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
	}

	template<class ConstBufferSequence>
	size_t send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, socket_base::message_flags flags)
	{
		boost::system::error_code ec;

	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		size_t s = this->service.send_to(this->implementation, buffers, destination, flags, ec);
		sys::throw_on_error(ec);
		return (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
	}

	template<class ConstBufferSequence>
	size_t send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, socket_base::message_flags flags, boost::system::error_code& ec)
	{
	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		return this->service.send_to(this->implementation, buffers, destination, flags, ec);
	}

	template<class ConstBufferSequence, class WriteHandler>
	void async_send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, WriteHandler handler)
	{
	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		this->service.async_send_to(this->implementation, buffers, destination, 0, write_handler_impl<WriteHandler>(handler));
	}

	template<class ConstBufferSequence, class WriteHandler>
	void async_send_to(const ConstBufferSequence& buffers, const endpoint_type& destination, socket_base::message_flags flags, WriteHandler handler)
	{
	#ifndef NDEBUG
		typename ConstBufferSequence::const_iterator i = buffers.begin();
		const mproto::header* hdr = boost::asio::buffer_cast<const mproto::header*>(*i);
		assert(boost::asio::buffer_size(*i) == mproto::header::mh_size);
		assert(hdr->reserved == 0 && hdr->next == 59);
	#endif

		this->service.async_send_to(this->implementation, buffers, destination, flags, handler);
	}

	template<class MutableBufferSequence>
	receive_type receive(const MutableBufferSequence& buffers)
	{
		boost::system::error_code ec;

		size_t s = this->service.receive(this->implementation, buffers, 0, ec);
		if (!ec && !buffers.checksum(s))
			ec = boost::system::error_code(boost::system::errc::bad_message, boost::system::get_generic_category());
		sys::throw_on_error(ec);
		s = (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
		return receive_type(s, buffers.mh_type());
	}

	template<class MutableBufferSequence>
	receive_type receive(const MutableBufferSequence& buffers, socket_base::message_flags flags)
	{
		boost::system::error_code ec;

		size_t s = this->service.receive(this->implementation, buffers, flags, ec);
		if (!ec && !buffers.checksum(s))
			ec = boost::system::error_code(boost::system::errc::bad_message, boost::system::get_generic_category());
		sys::throw_on_error(ec);
		s = (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
		return receive_type(s, buffers.mh_type());
	}

	template<class MutableBufferSequence>
	receive_type receive(const MutableBufferSequence& buffers, socket_base::message_flags flags, boost::system::error_code& ec)
	{
		size_t s = this->service.receive(this->implementation, buffers, flags, ec);
		if (!ec && !buffers.checksum(s))
			ec = boost::system::error_code(boost::system::errc::bad_message, boost::system::get_generic_category());
		s = (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
		return receive_type(s, buffers.mh_type());
	}

	template<class MutableBufferSequence, class ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, ReadHandler handler)
	{
		this->service.async_receive(this->implementation, buffers, 0, read_handler_impl<MutableBufferSequence, ReadHandler>(buffers, handler));
	}

	template<class MutableBufferSequence, class ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, socket_base::message_flags flags, ReadHandler handler)
	{
		this->service.async_receive(this->implementation, buffers, flags, read_handler_impl<MutableBufferSequence, ReadHandler>(buffers, handler));
	}

	template<class MutableBufferSequence>
	receive_type receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint)
	{
		boost::system::error_code ec;

		size_t s = this->service.receive_from(this->implementation, buffers, sender_endpoint, 0, ec);
		if (!ec && !buffers.checksum(s))
			ec = boost::system::error_code(boost::system::errc::bad_message, boost::system::get_generic_category());
		sys::throw_on_error(ec);
		s = (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
		return receive_type(s, buffers.mh_type());
	}

	template<class MutableBufferSequence>
	receive_type receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags)
	{
		boost::system::error_code ec;

		size_t s = this->service.receive_from(this->implementation, buffers, sender_endpoint, flags, ec);
		if (!ec && !buffers.checksum(s))
			ec = boost::system::error_code(boost::system::errc::bad_message, boost::system::get_generic_category());
		sys::throw_on_error(ec);
		s = (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
		return receive_type(s, buffers.mh_type());
	}

	template<class MutableBufferSequence>
	receive_type receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags, boost::system::error_code& ec)
	{
		size_t s = this->service.receive_from(this->implementation, buffers, sender_endpoint, flags, ec);
		if (!ec && !buffers.checksum(s))
			ec = boost::system::error_code(boost::system::errc::bad_message, boost::system::get_generic_category());
		s = (s < mproto::header::mh_size) ? 0 : (s - mproto::header::mh_size);
		return receive_type(s, buffers.mh_type());
	}

	template<class MutableBufferSequence, class ReadHandler>
	void async_receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, ReadHandler handler)
	{
		this->service.async_receive_from(this->implementation, buffers, sender_endpoint, 0, read_handler_impl<MutableBufferSequence, ReadHandler>(buffers, handler));
	}

	template<class MutableBufferSequence, class ReadHandler>
	void async_receive_from(const MutableBufferSequence& buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags, ReadHandler handler)
	{
		this->service.async_receive_from(this->implementation, buffers, sender_endpoint, flags, read_handler_impl<MutableBufferSequence, ReadHandler>(buffers, handler));
	}
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_MIP__HPP_ */
