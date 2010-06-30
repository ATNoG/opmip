//=============================================================================
// Brief   : Binding Update List
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
#include <opmip/pmip/bulist.hpp>
#include <boost/utility.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
bulist::bulist()
{
}

bulist::~bulist()
{
	clear();
}

bool bulist::insert(bulist_entry* entry)
{
	std::pair<mn_id_tree::iterator, bool> ins = _mn_id_tree.insert_unique(*entry);
	if (!ins.second) {
		delete entry;
		return false;
	}

	if (!_mn_link_addr_tree.insert_unique(*entry).second) {
		_mn_id_tree.erase_and_dispose(ins.first, disposer<bulist_entry>());
		return false;
	}

	return true;
}

bool bulist::remove(bulist_entry* entry)
{
	_mn_link_addr_tree.erase_and_dispose(*entry, disposer<void>());
	return _mn_id_tree.erase_and_dispose(*entry, disposer<bulist_entry>()) != 0;
}

bulist_entry* bulist::find(const std::string& mn_id)
{
	mn_id_tree::iterator entry = _mn_id_tree.find(mn_id, compare_mn_id());

	if (entry == _mn_id_tree.end())
		return nullptr;

	return boost::addressof(*entry);
}

bulist_entry* bulist::find(const link_address& mn_link_address)
{
	mn_link_addr_tree::iterator entry = _mn_link_addr_tree.find(mn_link_address, compare_mn_link_address());

	if (entry == _mn_link_addr_tree.end())
		return nullptr;

	return boost::addressof(*entry);
}

void bulist::clear()
{
	_mn_id_tree.clear_and_dispose(disposer<void>());
	_mn_link_addr_tree.clear_and_dispose(disposer<bulist_entry>());
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
