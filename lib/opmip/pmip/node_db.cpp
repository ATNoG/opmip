//=============================================================================
// Brief   : Node Database
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

#include <opmip/pmip/node_db.hpp>
#include <opmip/logger.hpp>
#include <opmip/disposer.hpp>
#include <boost/utility.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
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
	//FIXME: tell rbtree to ignore unlinked nodes
	_router_nodes_by_id.clear_and_dispose(disposer<void>());
	_mobile_nodes_by_id.clear_and_dispose(disposer<void>());
	_router_nodes_by_key.clear_and_dispose(disposer<router_node>());
	_mobile_nodes_by_key.clear_and_dispose(disposer<mobile_node>());
}

std::pair<size_t, size_t> node_db::load(std::istream& input)
{
	using boost::property_tree::ptree;
	size_t rcnt = 0;
	size_t mcnt = 0;
	ptree  pt;

	boost::property_tree::read_json(input, pt);

	ptree& routers = pt.get_child("router-nodes");
	for (ptree::iterator i = routers.begin(), e = routers.end(); i != e; ++i) {
		std::string id;
		ip_address  addr;
		uint        sid;

		id = i->second.get<std::string>("id");
		addr = ip_address::from_string(i->second.get<std::string>("ip-address"));
		sid = i->second.get<uint>("ip-scope-id");

		if (insert_router(id, addr, sid))
			++rcnt;
	}

	ptree& mns = pt.get_child("mobile-nodes");
	for (ptree::iterator i = mns.begin(), e = mns.end(); i != e; ++i) {
		std::string    id;
		ip_prefix_list prefs;
		link_address   laddr;
		std::string    lma_id;

		id = i->second.get<std::string>("id");
		prefs.push_back(ip_prefix::from_string(i->second.get<std::string>("ip-prefix")));
		laddr = link_address::from_string(i->second.get<std::string>("mac"));
		lma_id = i->second.get<std::string>("lma-id");

		if (insert_mobile_node(id, prefs, laddr, lma_id))
			++mcnt;
	}

	return std::make_pair(rcnt, mcnt);
}

const router_node* node_db::find_router(const key& key) const
{
	node_tree::const_iterator i = _router_nodes_by_id.find(key, node::compare());

	if (i != _router_nodes_by_id.end())
		return static_cast<const router_node*>(boost::addressof(*i));

	return nullptr;
}

const mobile_node* node_db::find_mobile_node(const key& key) const
{
	node_tree::const_iterator i = _mobile_nodes_by_id.find(key, node::compare());

	if (i != _mobile_nodes_by_id.end())
		return static_cast<const mobile_node*>(boost::addressof(*i));

	return nullptr;
}

const router_node* node_db::find_router(const router_key& key) const
{
	router_node_tree::const_iterator i = _router_nodes_by_key.find(key, router_node::compare());

	if (i != _router_nodes_by_key.end())
		return boost::addressof(*i);

	return nullptr;
}

const mobile_node* node_db::find_mobile_node(const mn_key& key) const
{
	mobile_node_tree::const_iterator i = _mobile_nodes_by_key.find(key, mobile_node::compare());

	if (i != _mobile_nodes_by_key.end())
		return boost::addressof(*i);

	return nullptr;
}

bool node_db::insert_router(const std::string& id, const ip_address& addr, uint device_id)
{
	router_node* router = new router_node(id, addr, device_id);
	std::pair<node_tree::iterator, bool> ins = _router_nodes_by_id.insert_unique(*router);

	if (!ins.second) {
		delete router;
		log_(0, "cound not insert router node due to duplicate id");
		return false;
	}

	if (!_router_nodes_by_key.insert_unique(*router).second) {
		_router_nodes_by_id.erase_and_dispose(ins.first, disposer<node, router_node>());
		log_(0, "cound not insert router node due to duplicate key");
		return false;
	}

	return true;
}

bool node_db::insert_mobile_node(const std::string& id, const ip_prefix_list& prefs, const link_address& link_addr, const std::string& lma_id)
{
	mobile_node* mn = new mobile_node(id, prefs, link_addr, lma_id);
	std::pair<node_tree::iterator, bool> ins = _mobile_nodes_by_id.insert_unique(*mn);

	if (!ins.second) {
		delete mn;
		log_(0, "cound not insert mobile node due to duplicate id");
		return false;
	}

	if (!_mobile_nodes_by_key.insert_unique(*mn).second) {
		_mobile_nodes_by_id.erase_and_dispose(ins.first, disposer<node, mobile_node>());
		log_(0, "cound not insert mobile node due to duplicate key");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
