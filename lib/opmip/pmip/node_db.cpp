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

#include <opmip/disposer.hpp>
#include <opmip/pmip/node_db.hpp>
#include <boost/utility.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
node_db::node_db()
{
}

node_db::~node_db()
{
	_nodes.clear_and_dispose(disposer<void>()); //FIXME: tell rbtree to ignore unlinked nodes
	_lmas.clear_and_dispose(disposer<route_node, lma_node>());
	_mags.clear_and_dispose(disposer<route_node, lma_node>());
	_mobile_nodes.clear_and_dispose(disposer<mobile_node>());
}

size_t node_db::load(std::istream& input)
{
	using boost::property_tree::ptree;
	size_t cnt = 0;
	ptree  pt;

	boost::property_tree::read_json(input, pt);

	ptree& lmas = pt.get_child("lma-nodes");
	for (ptree::iterator i = lmas.begin(), e = lmas.end(); i != e; ++i) {
		std::string id;
		ip_address  addr;
		uint        sid;
		bool        res;

		id = i->second.get<std::string>("id");
		addr = ip_address::from_string(i->second.get<std::string>("ip-address"));
		sid = i->second.get<uint>("ip-scope-id");
		res = insert_lma(id, addr, sid);
		if (res)
			++cnt;
	}

	ptree& mags = pt.get_child("mag-nodes");
	for (ptree::iterator i = mags.begin(), e = mags.end(); i != e; ++i) {
		std::string id;
		ip_address  addr;
		uint        sid;
		bool        res;

		id = i->second.get<std::string>("id");
		addr = ip_address::from_string(i->second.get<std::string>("ip-address"));
		sid = i->second.get<uint>("ip-scope-id");
		res = insert_mag(id, addr, sid);
		if (res)
			++cnt;
	}

	ptree& mns = pt.get_child("mobile-nodes");
	for (ptree::iterator i = mns.begin(), e = mns.end(); i != e; ++i) {
		std::string    id;
		ip_prefix_list prefs;
		link_address   laddr;
		std::string    lma_id;
		bool        res;

		id = i->second.get<std::string>("id");
		prefs.push_back(ip_prefix::from_string(i->second.get<std::string>("ip-prefix")));
		laddr = link_address::from_string(i->second.get<std::string>("mac"));
		lma_id = i->second.get<std::string>("lma-id");
		res = insert_mobile_node(id, prefs, laddr, lma_id);
		if (res)
			++cnt;
	}

	return cnt;
}

const lma_node* node_db::find_lma(const key& key) const
{
	const node* nd = find(key);

	if (nd && nd->type() == node::lma)
		return static_cast<const lma_node*>(nd);

	return nullptr;
}

const mag_node* node_db::find_mag(const key& key) const
{
	const node* nd = find(key);

	if (nd && nd->type() == node::mag)
		return static_cast<const mag_node*>(nd);

	return nullptr;
}

const mobile_node* node_db::find_mobile_node(const key& key) const
{
	const node* nd = find(key);

	if (nd && nd->type() == node::mobile)
		return static_cast<const mobile_node*>(nd);

	return nullptr;
}

const lma_node* node_db::find_lma(const lma_key& key) const
{
	route_node_tree::const_iterator i = _lmas.find(key, route_node::compare());

	if (i != _lmas.end())
		return static_cast<const lma_node*>(boost::addressof(*i));

	return nullptr;
}

const mag_node* node_db::find_mag(const mag_key& key) const
{
	route_node_tree::const_iterator i = _mags.find(key, route_node::compare());

	if (i != _mags.end())
		return static_cast<const mag_node*>(boost::addressof(*i));

	return nullptr;
}

const mobile_node* node_db::find_mobile_node(const mn_key& key) const
{
	mobile_node_tree::const_iterator i = _mobile_nodes.find(key, mobile_node::compare());

	if (i != _mobile_nodes.end())
		return boost::addressof(*i);

	return nullptr;
}

const node* node_db::find(const key& key) const
{
	node_tree::const_iterator i = _nodes.find(key, node::compare());

	if (i != _nodes.end())
		return boost::addressof(*i);

	return nullptr;
}

bool node_db::insert_lma(const std::string& id, const ip_address& addr, uint device_id)
{
	lma_node* lma = new lma_node(id, addr, device_id);
	std::pair<node_tree::iterator, bool> ins = _nodes.insert_unique(*lma);

	if (!ins.second) {
		delete lma;
		return false;
	}

	if (!_lmas.insert_unique(*lma).second) {
		_nodes.erase_and_dispose(ins.first, disposer<node, lma_node>());
		return false;
	}

	return true;
}

bool node_db::insert_mag(const std::string& id, const ip_address& addr, uint device_id)
{
	mag_node* mag = new mag_node(id, addr, device_id);
	std::pair<node_tree::iterator, bool> ins = _nodes.insert_unique(*mag);

	if (!ins.second) {
		delete mag;
		return false;
	}

	if (!_mags.insert_unique(*mag).second) {
		_nodes.erase_and_dispose(ins.first, disposer<node, mag_node>());
		return false;
	}

	return true;
}

bool node_db::insert_mobile_node(const std::string& id, const ip_prefix_list& prefs, const link_address& link_addr, const std::string& lma_id)
{
	mobile_node* mn = new mobile_node(id, prefs, link_addr, lma_id);
	std::pair<node_tree::iterator, bool> ins = _nodes.insert_unique(*mn);

	if (!ins.second) {
		delete mn;
		return false;
	}

	if (!_mobile_nodes.insert_unique(*mn).second) {
		_nodes.erase_and_dispose(ins.first, disposer<node, mobile_node>());
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
