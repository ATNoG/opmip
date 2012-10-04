//=============================================================================
// Brief   : Netlink Utilities
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

#ifndef OPMIP_SYS_NETLINK_UTILS__HPP_
#define OPMIP_SYS_NETLINK_UTILS__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/sys/netlink/error.hpp>
#include <opmip/sys/netlink/message.hpp>
#include <opmip/sys/netlink/message_iterator.hpp>
#include <boost/system/error_code.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace sys { namespace nl {

///////////////////////////////////////////////////////////////////////////////
template<class Socket, class ConstBufferSequence>
void checked_send(Socket& sock, const ConstBufferSequence& msg, boost::system::error_code& ec)
{
	sock.send(msg, 0, ec);
	if (ec)
		return;


	uchar  resp[256];
	size_t rlen;

	for (;;) {
		rlen = sock.receive(boost::asio::buffer(resp), 0, ec);
		if (ec)
			return;


		nl::message_iterator mit(resp, rlen);
		nl::message_iterator end;
		int                  errc = 0;

		for (; mit != end; ++mit) {
			if (mit->type == nl::header::m_error /*&& mit->sequence == sequence*/) {
				nl::message<nl::error> err(mit);

				errc = -err->error;
				if (errc)
					ec = boost::system::error_code(errc, boost::system::system_category());
				return;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace nl */ } /* namespace sys */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_NETLINK_UTILS__HPP_ */
