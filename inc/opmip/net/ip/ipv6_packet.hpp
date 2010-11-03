//=============================================================================
// Brief   : IPv6 Packet
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

#ifndef OPMIP_NET_IP_IPV6_PACKET__HPP_
#define OPMIP_NET_IP_IPV6_PACKET__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace ip {

///////////////////////////////////////////////////////////////////////////////
class ipv6_packet {
public:
	struct header {
		typedef opmip::ip::address_v6::bytes_type ip6_addr;

		uint8    version;
		uint8    flow_label[3];
		uint16   payload_len;
		uint8    next_header;
		uint8    hop_limit;
		ip6_addr src_addr;
		ip6_addr dst_addr;
	};

	struct pseudo_header {
		pseudo_header(const opmip::ip::address_v6& src_ip,
		              const opmip::ip::address_v6& dst_ip,
		              uint32 payload_len,
		              uint8 next_header)
			: src(src_ip.to_bytes()), dst(dst_ip.to_bytes()),
			  len(::htonl(payload_len)), next(next_header)
		{
			pad[0] = 0;
			pad[1] = 0;
			pad[2] = 0;
		}

		typedef opmip::ip::address_v6::bytes_type ip6_addr;

		ip6_addr src;
		ip6_addr dst;
		uint32   len;
		uint8    pad[3];
		uint8    next;
	};

	typedef std::vector<boost::asio::const_buffers_1> const_buffers;

public:
	ipv6_packet()
	{ }

	template<class PayloadPacket>
	ipv6_packet(const opmip::ip::address_v6& src,
	            const opmip::ip::address_v6& dst,
	            uint8 hop_limit, PayloadPacket& payload)
		: _header()
	{
		operator()(src, dst, hop_limit, payload);
	}

	template<class PayloadPacket>
	void operator()(const opmip::ip::address_v6& src,
	                const opmip::ip::address_v6& dst,
	                uint8 hop_limit, PayloadPacket& payload)
	{
		uint16 len = payload.size();
		uint8  nxh = PayloadPacket::ipv6_next_header;

		_header.version = 0x60;
		_header.payload_len = ::htons(len);
		_header.next_header = nxh;
		_header.hop_limit = hop_limit;
		_header.src_addr = src.to_bytes();
		_header.dst_addr = dst.to_bytes();

		payload.notify(pseudo_header(src, dst, len, nxh));

		_payload = payload.cbuffer();
	}

	size_t size() const
	{
		return sizeof(header);
	}

	const_buffers cbuffer() const
	{
		const_buffers buffs;

		buffs.push_back(boost::asio::buffer(&_header, sizeof(header)));
		buffs.insert(buffs.end(), _payload.begin(), _payload.end());
		return buffs;
	}

private:
	header        _header;
	const_buffers _payload;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_NET_IP_IPV6_PACKET__HPP_ */
