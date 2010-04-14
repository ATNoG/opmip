//=============================================================================
// Brief   : Mobility Protocol Buffer
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

#ifndef OPMIP_IP_MPROTO_BUFFER__HPP_
#define OPMIP_IP_MPROTO_BUFFER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/mproto.hpp>
#include <boost/make_shared.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
class mproto_const_buffers_1 {
public:
	typedef boost::asio::const_buffer        value_type;
	typedef const boost::asio::const_buffer* const_iterator;

public:
	template<class MPacket>
	explicit mproto_const_buffers_1(const MPacket& mp)
		: _header(boost::make_shared<mproto::header>())
	{
		_header->init(MPacket::mh_type, MPacket::mh_size);
		_header->update(mp.data(), MPacket::mh_size);
		_header->finalize();

		_buffs[0] = value_type(_header.get(), mproto::header::mh_size);
		_buffs[1] = value_type(mp.data(), MPacket::mh_size);
		_buffs[2] = _header->pad_buffer();
	}

	const_iterator begin() const { return &_buffs[0]; }
	const_iterator end() const   { return &_buffs[3]; }

private:
	value_type                        _buffs[3];
	boost::shared_ptr<mproto::header> _header;
};

class mproto_const_buffers_2 {
public:
	typedef boost::asio::const_buffer        value_type;
	typedef const boost::asio::const_buffer* const_iterator;

public:
	template<class MPacket>
	mproto_const_buffers_2(const MPacket& mp, const value_type& payload)
		: _header(boost::make_shared<mproto::header>())
	{
		const size_t len = MPacket::mh_size + boost::asio::buffer_size(payload);

		_header->init(MPacket::mh_type, len);
		_header->update(mp.data(), MPacket::mh_size);
		_header->update(boost::asio::buffer_cast<const void*>(payload), boost::asio::buffer_size(payload));
		_header->finalize();

		_buffs[0] = value_type(_header.get(), mproto::header::mh_size);
		_buffs[1] = value_type(mp.data(), MPacket::mh_size);
		_buffs[2] = payload;
		_buffs[3] = _header->pad_buffer();
	}

	const_iterator begin() const { return &_buffs[0]; }
	const_iterator end() const   { return &_buffs[4]; }

private:
	value_type                        _buffs[4];
	boost::shared_ptr<mproto::header> _header;
};

///////////////////////////////////////////////////////////////////////////////
template<class MPacket>
mproto_const_buffers_1 mproto_cbuffer(const MPacket& mp)
{
	return mproto_const_buffers_1(mp);
}

template<class MPacket>
mproto_const_buffers_2 mproto_cbuffer(const MPacket& mp, const void* buff, size_t len)
{
	return mproto_const_buffers_2(mp, boost::asio::const_buffer(buff, len));
}

template<class MPacket>
mproto_const_buffers_2 mproto_cbuffer(const MPacket& mp, const std::pair<const void*, size_t>& buff)
{
	return mproto_const_buffers_2(mp, boost::asio::const_buffer(buff.first, buff.second));
}

///////////////////////////////////////////////////////////////////////////////
class mproto_mutable_buffers_1 {
public:
	typedef boost::asio::mutable_buffer        value_type;
	typedef const boost::asio::mutable_buffer* const_iterator;

public:
	explicit mproto_mutable_buffers_1(const value_type& buff)
		: _header(boost::make_shared<mproto::header>())
	{
		_buffs[0] = value_type(_header.get(), mproto::header::mh_size);
		_buffs[1] = buff;
	}

	const_iterator begin() const { return &_buffs[0]; }
	const_iterator end() const   { return &_buffs[2]; }

	bool checksum(size_t len) const
	{
		const size_t clen = (_header->length + 1) * 8;

		if ((clen != len)
		    || (_header->next != 59)
		    || (_header->reserved != 0))
			return false;

/*		FIXME:
		_header->update(_header.get(), mproto::header::mh_size);
		_header->update(boost::asio::buffer_cast<const void*>(_buffs[1]),
		                std::min(boost::asio::buffer_size(_buffs[1]), len - mproto::header::mh_size));
		_header->finalize();

		return !_header->checksum;
*/
		return true;
	}

	mproto::mh_types mh_type() const
	{
		return static_cast<mproto::mh_types>(_header->mh_type);
	}

private:
	value_type                        _buffs[2];
	boost::shared_ptr<mproto::header> _header;
};

///////////////////////////////////////////////////////////////////////////////
mproto_mutable_buffers_1 mproto_mbuffer(void* buff, size_t len)
{
	return mproto_mutable_buffers_1(boost::asio::mutable_buffer(buff, len));
}

mproto_mutable_buffers_1 mproto_mbuffer(const std::pair<void*, size_t>& buff)
{
	return mproto_mutable_buffers_1(boost::asio::mutable_buffer(buff.first, buff.second));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_MPROTO_BUFFER__HPP_ */
