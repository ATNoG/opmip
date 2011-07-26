//=============================================================================
// Brief   : Red Black Tree - Self Balanced Binary Tree Hook
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2006-2010 Bruno Santos <bsantos@av.it.pt>
// Copyright (C) 2010 Universidade de Aveiro
// Copyrigth (C) 2010 Instituto de TelecomunicaÃ§Ãµes - PÃ³lo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_RBTREE_HOOK__HPP_
#define OPMIP_RBTREE_HOOK__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/assert.hpp>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
struct rbtree_hook {
	void set_parent(rbtree_hook* p)
	{
		BOOST_ASSERT(!((size_t) p & 0x3));
		_parent = (rbtree_hook*) (((size_t) p) | ((size_t) _parent & 0x3));
	}

	rbtree_hook* get_parent()
	{
		return (rbtree_hook*) ((size_t) _parent & ~0x3);
	}

	enum {
		red   = 0,
		black = 1
	};

	union {
		rbtree_hook* _parent;
		int          _color : 2;
	};
	rbtree_hook*     _left;
	rbtree_hook*     _right;

	void init() { _parent = _left = _right = nullptr; }
	void insert(rbtree_hook** root, rbtree_hook* parent);
	void remove(rbtree_hook** root);

	rbtree_hook* max();
	rbtree_hook* min();
	rbtree_hook* next();
	rbtree_hook* prev();

	void dump(std::ostream& out, unsigned max_deep = 25);
};

///////////////////////////////////////////////////////////////////////////////
class rbtree_hook_iterator {
public:
	rbtree_hook_iterator()
		: _next(nullptr), _prev(nullptr)
	{ }
	rbtree_hook_iterator(rbtree_hook* next, rbtree_hook* prev)
		: _next(next), _prev(prev)
	{ }

//protected:
	void inc()
	{
		BOOST_ASSERT(_next);

		_prev = _next;
		_next = _next->next();
	}

	void dec()
	{
		BOOST_ASSERT(_prev);

		_next = _prev;
		_prev = _prev->prev();
	}

	rbtree_hook* next() { return _next; }
	rbtree_hook* prev() { return _prev; }

protected:
	rbtree_hook* _next;
	rbtree_hook* _prev;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif	/* OPMIP_RBTREE_HOOK__HPP_ */
