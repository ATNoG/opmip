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
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace ip {

///////////////////////////////////////////////////////////////////////////////
class mproto_const_buffers_1 {
public:
	typedef boost::asio::const_buffer        value_type;
	typedef const boost::asio::const_buffer* const_iterator;

public:
	template<class MPacket>
	mproto_const_buffers_1(const MPacket& mp)
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
} /* namespace ip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_IP_MPROTO_BUFFER__HPP_ */
