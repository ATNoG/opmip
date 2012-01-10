//=============================================================================
// Brief   : PIM Generator and Parser
// Authors : Bruno Santos <bsantos@av.it.pt>
// 		   : Sérgio Figueiredo <sfigueiredo@av.it.pt>
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

#include <opmip/base.hpp>
#include <opmip/net/ip/pim_gen_parser.hpp>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {	namespace net {	namespace ip {

///////////////////////////////////////////////////////////////////////////////
//TODO Checksum functions
/*
uint16 inchksum(const void *data, uint32 length) {
        long sum = 0;
        const uint16 *wrd = reinterpret_cast<const uint16 *>(data);
        long slen = static_cast<long>(length);

        while (slen > 1) {
                sum += *wrd++;
                slen -= 2;
        }

        if (slen > 0)
                sum += *reinterpret_cast<const uint8 *>(wrd);

        while (sum >> 16)
                sum = (sum & 0xffff) + (sum >> 16);

        return static_cast<uint16>(sum);
}

uint16 ipv6_checksum(uint8 protocol const address_v6 &src, const address_v6 &dst, const void *data, uint16 len) {
	struct {
    	address_v6 src;
        address_v6 dst;
        uint16 length;
        uint16 zero1;
        uint8 zero2;
        uint8 next;
    }  __attribute__ ((packed)) pseudo;
  	uint32 chksum = 0;

  	pseudo.src = src;
    pseudo.dst = dst;
    pseudo.length = htons(len);
    pseudo.zero1 = 0;
    pseudo.zero2 = 0;
    pseudo.next = protocol;

    chksum = inchksum(&pseudo, sizeof(pseudo));
    chksum += inchksum(data, len);
    chksum = (chksum >> 16) + (chksum & 0xffff);
    chksum += (chksum >> 16);

    chksum = static_cast<uint16>(~chksum);
    if (chksum == 0)
      	chksum = 0xffff;

  	return chksum;
}
*/
///////////////////////////////////////////////////////////////////////////////

size_t hello_msg::gen(uchar* buffer, size_t length) const
{
	size_t pos = /*align_to<4>*/(sizeof(pim::hello));

	if (pos > length)
		return 0;

	pim::hello* hello = new(buffer) pim::hello;

	if (this->holdtime) {
		const size_t npos = pos + (sizeof(pim::hello::holdtime));
		if (npos > length)
			return 0;
		pim::hello::holdtime* ht = new (buffer+pos) pim::hello::holdtime;
		ht->value = htons(*this->holdtime);
		pos = npos;
	}
	if (this->dr_priority) {
		const size_t npos = pos + (sizeof(pim::hello::dr_priority));
		if (npos > length)
			return 0;
		pim::hello::dr_priority* dp = new (buffer+pos) pim::hello::dr_priority;
		dp->value = htonl(*this->dr_priority);
		pos = npos;
	}
	if (this->generation_id) {
		const size_t npos = pos + (sizeof(pim::hello::generation_id));
		if (npos > length)
			return 0;
		pim::hello::generation_id* gi = new (buffer+pos) pim::hello::generation_id;
		gi->value = htonl(*this->generation_id);
		pos = npos;
	}

	if (!this->maddr_list.empty()) {
		const size_t elen = sizeof(enc_unicast);			// encoded unicast structure length
		const size_t alen = elen * this->maddr_list.size();									// maddr_list length
		const size_t npos = pos + sizeof(pim::hello::address_list) + alen;					// pointer to end of maddr_list
		if (npos > length)
			return 0;

		pim::hello::address_list* al = new(buffer + pos) pim::hello::address_list(this->maddr_list.size());
		enc_unicast* entry = al->entries;

		for (std::vector<address_v6>::const_iterator i = this->maddr_list.begin(), e = this->maddr_list.end(); i != e; ++i, ++entry) {
			entry->family = 2 /* IANA IPv6 */;
			entry->type = 0;
			entry->address = i->to_bytes();
		}
		pos = npos;
	}

//	hello->cs = ipv6_checksum() // TODO IPv6 checksum
	return pos;
}

///////////////////////////////////////////////////////////////////////////////
bool hello_msg::parse(uchar* buffer, size_t length)
{
	pim::hello* ph = pim::header::cast<pim::hello>(buffer, length);
	if (!ph)
		return false;

	length -= sizeof(pim::hello);
	if (length < sizeof(pim::hello::option))
		return false;

	pim::hello::option* opt = ph->options;
	do {
		switch (opt->type) {
		case pim::hello::holdtime::type_value: {	// Holdtime option
			pim::hello::holdtime* ht = pim::hello::option_cast<pim::hello::holdtime>(opt);

			this->holdtime = ntohs(ht->value);
		} break;

		case pim::hello::dr_priority::type_value: {	// DR priority option
			pim::hello::dr_priority* dp = pim::hello::option_cast<pim::hello::dr_priority>(opt);

			this->dr_priority = ntohl(dp->value);
		} break;

		case pim::hello::generation_id::type_value: {	// Generation ID option
			pim::hello::generation_id* gi = pim::hello::option_cast<pim::hello::generation_id>(opt);

			this->generation_id = ntohl(gi->value);
		} break;

		case pim::hello::address_list::type_value: {	// Address List option
			pim::hello::address_list* al = pim::hello::option_cast<pim::hello::address_list>(opt);
			const size_t elen = sizeof(enc_unicast) + sizeof(address_v6::bytes_type);
			const size_t cnt  = al->length / elen;
			if (al->length % elen)
				return false;

			for (uint i = 0; i < cnt; ++i) {
				if (al->entries[i].family != 2 /* IPv6 */ || al->entries[i].type != 0)
					return false;
				this->maddr_list.push_back(address_v6(al->entries[i].address));
			}

		} break;
		default:
			break;
		};
	} while ((opt = pim::hello::option_next(opt, length)));

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/*
size_t register_msg::gen(uchar* buffer, size_t length) const
{
	if (sizeof(pim::register_) > length)
		return 0;

	pim::register_* register_ = new(buffer) pim::register
	register_->data = htonl(this->data);
	return sizeof(pim::register_);
}

///////////////////////////////////////////////////////////////////////////////
size_t register_stop_msg::gen(uchar* buffer, size_t length) const
{
	size_t pos = align_to<4>(sizeof(pim::register_stop));
	if (pos > length)
		return 0;

	pim::register_stop* register_stop = new(buffer) pim::register_stop
	register_stop->group = this->grp;
	register_stop->source = this->src;

	return pos;
}
*/
///////////////////////////////////////////////////////////////////////////////
size_t join_prune_msg::gen(uchar* buffer, size_t length) const
{
	size_t pos = sizeof(pim::join_prune);
	if (pos > length)
		return 0;

	pim::join_prune* join_prune = new(buffer) pim::join_prune;

	join_prune->uplink_neigh.family = 2;
	join_prune->uplink_neigh.type = 0;
	join_prune->uplink_neigh.address = this->uplink.to_bytes();
	join_prune->num_groups = this->mcast_groups.size();
	join_prune->holdtime = htons(60);	// TODO check the default value

	pim::join_prune::mcast_group* mg = join_prune->mcast_groups;

	for (std::vector<mcast_group>::const_iterator i = this->mcast_groups.begin(), e = this->mcast_groups.end(); i != e; ++i) {
		size_t npos = pos + sizeof(pim::join_prune::mcast_group)
		            + sizeof(enc_source) * (i->joins.size() + i->prunes.size());
		if (npos > length)
			return 0;

		mg->group.family = 2;
		mg->group.type = 0;
		mg->group.address = i->group.to_bytes();
		mg->count_joins = htons(i->joins.size());
		mg->count_prunes = htons(i->prunes.size());

		enc_source* src = mg->sources;

		for (std::vector<address_v6>::const_iterator j = i->joins.begin(), k = i->joins.end(); j != k; ++j, ++src) {
			src->family = 2;
			src->type = 0;
			src->address = j->to_bytes();
		}

		for (std::vector<address_v6>::const_iterator j = i->prunes.begin(), k = i->prunes.end(); j != k; ++j, ++src) {
			src->family = 2;
			src->type = 0;
			src->address = j->to_bytes();
		}

		mg = offset_cast<pim::join_prune::mcast_group*>(mg, npos - pos);
		pos = npos;
	}

	//	join_prune->cs = ipv6_checksum() // TODO IPv6 checksum
	return pos;
}

///////////////////////////////////////////////////////////////////////////////
bool join_prune_msg::parse(uchar* buffer, size_t length)
{
	pim::join_prune* jp = pim::header::cast<pim::join_prune>(buffer, length);
	if (!jp)
		return false;

	if (jp->uplink_neigh.family != 2 || jp->uplink_neigh.type != 0)
		return false;
	this->uplink = address_v6(jp->uplink_neigh.address);

	pim::join_prune::mcast_group* mg = jp->mcast_groups;
	uint cnt = jp->num_groups;
	size_t pos = sizeof(pim::join_prune);
	size_t npos;

	while (cnt--) {
		npos = pos + sizeof(pim::join_prune::mcast_group);
		if (npos > length)
			return false;

		const uint jcnt = mg->count_joins;
		const uint pcnt = mg->count_prunes;
		npos += sizeof(enc_source) * (jcnt + pcnt);
		if (npos > length)
			return false;

		mcast_group mc;

		if (mg->group.family != 2 || mg->group.type != 0)
			return false;
		mc.group = address_v6(mg->group.address);

		for (uint i = 0; i < jcnt; ++i)	{
			if (mg->sources[i].family != 2 || mg->sources[i].type != 0)
				return false;

			mc.joins.push_back(address_v6(mg->sources[i].address));
		}
		for (uint i = jcnt; i < (pcnt + jcnt); ++i)	{
			if (mg->sources[i].family != 2 || mg->sources[i].type != 0)
				return false;

			mc.prunes.push_back(address_v6(mg->sources[i].address));
		}

		this->mcast_groups.push_back(mc);

		mg = offset_cast<pim::join_prune::mcast_group*>(mg, npos - pos);
		pos = npos;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace ip */ } /* namespace net */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
