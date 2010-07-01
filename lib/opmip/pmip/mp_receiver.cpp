//=============================================================================
// Brief   : Mobility Protocol Receiver
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

#include <opmip/pmip/mp_receiver.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
bool pbu_receiver::parse(size_t rbytes, proxy_binding_info& pbinfo)
{
	ip::mproto::header* hdr = ip::mproto::header::cast(_buffer, rbytes);

	if (!hdr)
		return false;


	ip::mproto::pbu* pbu = ip::mproto::pbu::cast(hdr);
	size_t           pos = sizeof(ip::mproto::pbu);

	if (!pbu || !pbu->proxy_reg())
		return false;

	pbinfo.address  = _endpoint.address();
	pbinfo.sequence = pbu->sequence();
	pbinfo.lifetime = 4000 * pbu->lifetime();


	ip::mproto::option*          opt;
	ip::mproto::option::nai*     nai = nullptr;
	ip::mproto::option::handoff* hof = nullptr;
	ip::mproto::option::att*     att = nullptr;

	while ((pos < rbytes) && (opt = ip::mproto::option::cast(_buffer + pos, rbytes - pos))) {
		pos += ip::mproto::option::size(opt);
		switch (opt->type) {
		case ip::mproto::option::nai::type_value:
			if (nai)
				return false;

			nai = opt->get<ip::mproto::option::nai>();

			if (nai->subtype != 1)
				return false;

			pbinfo.id.assign(nai->id, opt->length - 1);
			break;

		case ip::mproto::option::handoff::type_value:
			if (hof)
				return false;

			hof = opt->get<ip::mproto::option::handoff>();
			pbinfo.handoff = static_cast<ip::mproto::option::handoff::type>(hof->indicator);
			break;

		case ip::mproto::option::att::type_value:
			if (att)
				return false;

			att = opt->get<ip::mproto::option::att>();
			pbinfo.link_type = static_cast<ll::technology>(att->tech_type);
			break;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool pba_receiver::parse(size_t rbytes, proxy_binding_info& pbinfo)
{
	ip::mproto::header* hdr = ip::mproto::header::cast(_buffer, rbytes);

	if (!hdr)
		return false;

	ip::mproto::pba* pba = ip::mproto::pba::cast(hdr);
	size_t           pos = sizeof(ip::mproto::pba);

	if (!pba || !pba->proxy_reg())
		return false;

	pbinfo.address  = _endpoint.address();
	pbinfo.sequence = pba->sequence();
	pbinfo.lifetime = 4000 * pba->lifetime();
	pbinfo.status   = pba->status();


	ip::mproto::option*          opt;
	ip::mproto::option::nai*     nai = nullptr;
	ip::mproto::option::handoff* hof = nullptr;
	ip::mproto::option::att*     att = nullptr;

	while ((pos < rbytes) && (opt = ip::mproto::option::cast(_buffer + pos, rbytes - pos))) {
		pos += ip::mproto::option::size(opt);
		switch (opt->type) {
		case ip::mproto::option::nai::type_value:
			if (nai)
				return false;

			nai = opt->get<ip::mproto::option::nai>();

			if (nai->subtype != 1)
				return false;

			pbinfo.id.assign(nai->id, opt->length - 1);
			break;

		case ip::mproto::option::handoff::type_value:
			if (hof)
				return false;

			hof = opt->get<ip::mproto::option::handoff>();
			pbinfo.handoff = static_cast<ip::mproto::option::handoff::type>(hof->indicator);
			break;

		case ip::mproto::option::att::type_value:
			if (att)
				return false;

			att = opt->get<ip::mproto::option::att>();
			pbinfo.link_type = static_cast<ll::technology>(att->tech_type);
			break;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
