//=============================================================================
// Brief   : Binding Cache
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

#ifndef OPMIP_PMIP_BCACHE__HPP_
#define OPMIP_PMIP_BCACHE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/ip/address.hpp>
#include <opmip/ip/prefix.hpp>
#include <opmip/ll/technology.hpp>
#include <opmip/ll/mac_address.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/intrusive/rbtree.hpp>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
class bcache_entry {
	friend class bcache;

public:
	typedef ip::address_v6          net_address;
	typedef ip::prefix_v6           net_prefix;
	typedef std::vector<net_prefix> net_prefix_list;
	typedef std::string             net_access_id;
	typedef ll::technology          link_tech;

	enum bind_status_t {
		k_bind_unknown,
		k_bind_registered,
		k_bind_deregistered,
	};

public:
	bcache_entry(boost::asio::io_service& ios, const std::string& mn_id,
	                                           const net_prefix_list& mn_prefix_list)
		: _id(mn_id), _prefix_list(mn_prefix_list),
		  lifetime(0), sequence(0),
		  link_type(link_tech::k_tech_unknown),
		  bind_status(k_bind_unknown),
		  timer(ios)
	{ }

	const std::string&     id() const          { return _id; }
	const net_prefix_list& prefix_list() const { return _prefix_list; }

private:
	boost::intrusive::set_member_hook<> _id_hook;

	net_access_id    _id;          ///MN Identifier
	net_prefix_list  _prefix_list; ///MN List of Network Prefixes

public:
	net_address care_of_address; ///MN Care of Address
	uint64      lifetime;        ///Remaining lifetime of this entry from last binding update
	uint16      sequence;        ///Sequence Number from last binding update, see also section 9.5.1
	link_tech   link_type;       ///MN Link-Layer Technology

	bind_status_t               bind_status;
	boost::asio::deadline_timer timer;
};

///////////////////////////////////////////////////////////////////////////////
class bcache {
	struct compare {
		bool operator()(const bcache_entry& rhs, const bcache_entry& lhs) const
		{
			return rhs._id < lhs._id;
		}

		bool operator()(const bcache_entry& rhs, const bcache_entry::net_access_id& key) const
		{
			return rhs._id < key;
		}

		bool operator()(const bcache_entry::net_access_id& key, const bcache_entry& lhs) const
		{
			return key < lhs._id;
		}
	};

	typedef boost::intrusive::compare<compare> compare_option;

	typedef boost::intrusive::member_hook<bcache_entry,
	                                      boost::intrusive::set_member_hook<>,
	                                      &bcache_entry::_id_hook> member_hook_option;

	typedef boost::intrusive::rbtree<bcache_entry,
	                                 member_hook_option,
	                                 compare_option> id_tree;

public:
	typedef bcache_entry                  entry_type;
	typedef bcache_entry::net_address     net_address;
	typedef bcache_entry::net_prefix      net_prefix;
	typedef bcache_entry::net_prefix_list net_prefix_list;
	typedef bcache_entry::net_access_id   net_access_id;
	typedef bcache_entry::link_tech       link_tech;

public:
	bcache();
	~bcache();

	bool insert(bcache_entry* entry);
	bool remove(bcache_entry* entry);

	bcache_entry* find(const std::string& mn_id);

	void clear();

private:
	id_tree _id_tree;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_PMIP_BCACHE__HPP_ */
