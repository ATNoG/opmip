//=============================================================================
// Brief   : Intrusive Double Linked List support
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

#ifndef OPMIP_LIST_HOOK__HPP_
#define OPMIP_LIST_HOOK__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
struct list_hook {
	void init() { next = prev = this; }

	void push_front(list_hook* node)
	{
		next->prev = node;
		node->prev = this;
		node->next = next;
		next = node;
	}

	void push_back(list_hook* node)
	{
		prev->next = node;
		node->next = this;
		node->prev = prev;
		prev = node;
	}

	list_hook* pop_front()
	{
		list_hook* node = next;
		node->remove();
		return node;
	}

	list_hook* pop_back()
	{
		list_hook* node = prev;
		node->remove();
		return node;
	}

	void swap(list_hook& y)
	{
		list_hook* n = next;
		list_hook* p = prev;

		next = y.next;
		prev = y.prev;
		y.next = n;
		y.prev = p;
	}

	void reverse()
	{
		list_hook* n = next;

		next = prev;
		prev = n;
	}

	void remove()
	{
		prev->next = next;
		next->prev = prev;
	}

	bool empty() const { return (next == this); }


	list_hook* next;
	list_hook* prev;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_LIST_HOOK__HPP_ */
