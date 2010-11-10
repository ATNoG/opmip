//=============================================================================
// Brief   : Ethernet Protocol
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

#include <opmip/net/link/ethernet.hpp>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
template class boost::asio::basic_datagram_socket<opmip::net::link::ethernet>;

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace net { namespace link {

///////////////////////////////////////////////////////////////////////////////
ethernet::endpoint::endpoint()
	: _family(17/*AF_PACKET*/), _protocol(0), _ifindex(0),
	  _hatype(1/*ARPHRD_ETHER*/), _pkttype(0), _halen(0)
{
	std::fill(_addr, _addr + 8, 0);
}

ethernet::endpoint::endpoint(uint16 proto, uint ifindex)
	: _family(AF_PACKET), _protocol(htons(proto)), _ifindex(ifindex),
	  _hatype(1/*ARPHRD_ETHER*/), _pkttype(0), _halen(0)
{
	std::fill(_addr, _addr + 8, 0);
}

ethernet::endpoint::endpoint(uint16 proto, uint ifindex, pk_type pktp, const ll::mac_address& destination)
	: _family(AF_PACKET), _protocol(htons(proto)), _ifindex(ifindex),
	  _hatype(1/*ARPHRD_ETHER*/), _pkttype(pktp), _halen(0)
{
	*reinterpret_cast<ll::mac_address::bytes_type*>(_addr) = destination.to_bytes();
	_addr[6] = 0;
	_addr[7] = 0;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace link */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
