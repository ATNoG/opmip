//=============================================================================
// Brief   : Node Database
// Authors : Bruno Santos <bsantos@av.it.pt>
// Authors : Filipe Manco <filipe.manco@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2011 Universidade de Aveiro
// Copyrigth (C) 2010-2011 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#include <opmip/pmip/node_db.hpp>
#include <opmip/logger.hpp>
#include <opmip/disposer.hpp>
#include <boost/utility.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
static logger log_("node-db", std::cout);

///////////////////////////////////////////////////////////////////////////////
node_db::node_db()
{
}

node_db::~node_db()
{
	_router_nodes_by_id.clear_and_dispose(disposer<router_node>());
	_mobile_nodes_by_id.clear_and_dispose(disposer<mobile_node>());
}

std::pair<size_t, size_t> node_db::load(std::istream& input)
{
	using boost::property_tree::ptree;
	size_t rcnt = 0;
	size_t mcnt = 0;
	ptree  pt;

	boost::property_tree::read_json(input, pt);

	BOOST_FOREACH(ptree::value_type &rn, pt.get_child("router-nodes")) {
		std::string id;
		ip_address  addr;
		uint        sid;

		id = rn.second.get<std::string>("id");
		addr = ip_address::from_string(rn.second.get<std::string>("ip-address"));
		sid = rn.second.get<uint>("ip-scope-id");

		if (insert_router(id, addr, sid))
			++rcnt;
	}

	BOOST_FOREACH(ptree::value_type &mn, pt.get_child("mobile-nodes")) {
		std::string       id;
		ip_prefix_list    prefs;
		link_address_list laddrs;
		std::string       lma_id;
		ip_address        home_addr;

		id = mn.second.get<std::string>("id");
		BOOST_FOREACH(ptree::value_type &v, mn.second.get_child("ip-prefix"))
			prefs.push_back(ip_prefix::from_string(v.second.get_value<std::string>()));
		BOOST_FOREACH(ptree::value_type &v, mn.second.get_child("link-address"))
			laddrs.push_back(link_address::from_string(v.second.get_value<std::string>()));
		lma_id = mn.second.get<std::string>("lma-id");
		home_addr = ip_address::from_string(mn.second.get<std::string>("home-address"));

		if (insert_mobile_node(id, prefs, laddrs, lma_id, home_addr))
			++mcnt;
	}

	return std::make_pair(rcnt, mcnt);
}

const router_node* node_db::find_router(const key& key) const
{
	router_node_tree::const_iterator i = _router_nodes_by_id.find(key, node::compare());

	if (i != _router_nodes_by_id.end())
		return static_cast<const router_node*>(boost::addressof(*i));

	return nullptr;
}

const mobile_node* node_db::find_mobile_node(const key& key) const
{
	mobile_node_tree::const_iterator i = _mobile_nodes_by_id.find(key, node::compare());

	if (i != _mobile_nodes_by_id.end())
		return static_cast<const mobile_node*>(boost::addressof(*i));

	return nullptr;
}

const router_node* node_db::find_router(const router_key& key) const
{
	router_node_key_tree::const_iterator i = _router_nodes_by_key.find(key, router_node::compare());

	if (i != _router_nodes_by_key.end())
		return boost::addressof(*i);

	return nullptr;
}

const mobile_node* node_db::find_mobile_node(const mn_key& key) const
{
	mobile_node_key_tree::const_iterator i = _mobile_nodes_by_key.find(key);

	if (i != _mobile_nodes_by_key.end())
		return i->second;

	return nullptr;
}

bool node_db::insert_router(const std::string& id, const ip_address& addr, uint device_id)
{
	std::auto_ptr<router_node> router(new router_node(id, addr, device_id));
	std::pair<router_node_tree::iterator, bool> ins = _router_nodes_by_id.insert_unique(*router);

	if (!ins.second) {
		log_(0, "cound not insert router node due to duplicate id");
		return false;
	}

	if (!_router_nodes_by_key.insert_unique(*router).second) {
		_router_nodes_by_id.remove(ins.first);
		log_(0, "cound not insert router node due to duplicate key");
		return false;
	}

	router.release();
	return true;
}

bool node_db::insert_mobile_node(const std::string& id, const ip_prefix_list& prefs,
                                 const link_address_list& link_addrs, const std::string& lma_id,
                                 const ip_address& home_addr)
{
	std::auto_ptr<mobile_node> mn(new mobile_node(id, prefs, link_addrs, lma_id, home_addr));
	std::pair<mobile_node_tree::iterator, bool> ins = _mobile_nodes_by_id.insert_unique(*mn);

	if (!ins.second) {
		log_(0, "cound not insert mobile node due to duplicate id");
		return false;
	}

	try {
		for (link_address_list::const_iterator i = link_addrs.begin(), e = link_addrs.end();
			 i != e; ++i) {
			if (!_mobile_nodes_by_key.insert(std::make_pair(*i, mn.get())).second) {
				_mobile_nodes_by_id.remove(ins.first);
				for (link_address_list::const_iterator j = link_addrs.begin(); j != i; ++j)
					_mobile_nodes_by_key.erase(*j);
				log_(0, "cound not insert mobile node due to duplicate key");
				return false;
			}
		}
	} catch (...) {
		_mobile_nodes_by_id.remove(ins.first);
		for (link_address_list::const_iterator i = link_addrs.begin(), e = link_addrs.end();
			 i != e; ++i)
			_mobile_nodes_by_key.erase(*i);

		throw;
	}

	mn.release();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
