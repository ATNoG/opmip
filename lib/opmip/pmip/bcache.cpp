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

#include <opmip/disposer.hpp>
#include <opmip/pmip/bcache.hpp>
#include <boost/utility.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace pmip {

///////////////////////////////////////////////////////////////////////////////
bcache::bcache()
{
}

bcache::~bcache()
{
	clear();
}

bool bcache::insert(bcache_entry* entry)
{
	if (!_id_tree.insert_unique(*entry).second) {
		delete entry;
		return false;
	}

	return true;
}

bool bcache::remove(bcache_entry* entry)
{
	return _id_tree.erase_and_dispose(*entry, disposer<bcache_entry>()) != 0;
}

bcache_entry* bcache::find(const std::string& mn_id)
{
	id_tree::iterator entry = _id_tree.find(mn_id, compare());

	if (entry == _id_tree.end())
		return nullptr;

	return boost::addressof(*entry);
}

void bcache::clear()
{
	_id_tree.clear_and_dispose(disposer<bcache_entry>());
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace pmip */ } /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
