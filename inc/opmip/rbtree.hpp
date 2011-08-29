//=============================================================================
// Brief   : Red-Back Tree Intrusive Container
// Authors : Bruno Santos <bsantos@av.it.pt>
// ----------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2006-2010 Bruno Santos <bsantos@av.it.pt>
// Copyright (C) 2010 Universidade de Aveiro
// Copyrigth (C) 2010 Instituto de Telecomunicações - Pólo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_RBTREE__HPP_
#define OPMIP_RBTREE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <opmip/rbtree_hook.hpp>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
template<class T, rbtree_hook T::* NodeMember, class Compare>
class rbtree;

///////////////////////////////////////////////////////////////////////////////
namespace detail {

///////////////////////////////////////////////////////////////////////////////
template<class T, rbtree_hook T::* NodeMember, class Compare>
class rbtree_iterator : rbtree_hook_iterator {
public:
	rbtree_iterator()
	{ }

	rbtree_iterator(rbtree_hook* next, rbtree_hook* prev)
		: rbtree_hook_iterator(next, prev)
	{ }

	T& operator*()
	{
		return *parent_of<T>(next(), NodeMember);
	}

	T* operator&()
	{
		return parent_of<T>(next(), NodeMember);
	}

	T* operator->()
	{
		return parent_of<T>(next(), NodeMember);
	}

	rbtree_iterator& operator++()
	{
		inc();
		return *this;
	}

	rbtree_iterator& operator--()
	{
		dec();
		return *this;
	}

	rbtree_iterator operator++(int)
	{
		rbtree_iterator tmp(*this);
		inc();
		return tmp;
	}

	rbtree_iterator operator--(int)
	{
		rbtree_iterator tmp(*this);
		dec();
		return tmp;
	}

	bool operator==(const rbtree_iterator& i) { return _next == i._next; }
	bool operator!=(const rbtree_iterator& i) { return _next != i._next; }
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace detail */

///////////////////////////////////////////////////////////////////////////////
template<class T, rbtree_hook T::* NodeMember, class Compare = std::less<T> >
class rbtree {
	rbtree(const rbtree&);
	rbtree& operator=(const rbtree&);

public:
	typedef T*       pointer;
	typedef T const* const_pointer;
	typedef T&       reference;
	typedef T const& const_reference;

	typedef typename detail::rbtree_iterator<T, NodeMember, Compare>       iterator;
	typedef typename detail::rbtree_iterator<const T, NodeMember, Compare> const_iterator;

public:
	rbtree() : _root(nullptr)
	{ }


	iterator begin()
	{
		if (!_root)
			return iterator();

		return iterator(_root->min(), nullptr);
	}

	iterator end()
	{
		if (!_root)
			return iterator();

		return iterator(nullptr, _root->max());
	}

	const_iterator begin() const
	{
		if (!_root)
			return iterator();

		return const_iterator(_root->min(), nullptr);
	}

	const_iterator end() const
	{
		if (!_root)
			return const_iterator();

		return const_iterator(nullptr, _root->max());
	}

	iterator insert_equal(reference elem)
	{
		rbtree_hook*  node   = member_of<rbtree_hook>(&elem, NodeMember);
		rbtree_hook*  parent = nullptr;
		rbtree_hook** next   = &_root;
		Compare       cmp;

		while (*next) {
			parent = *next;
			if (cmp(elem, *parent_of<T>(*next, NodeMember)))
				next = &(*next)->_left;
			else
				next = &(*next)->_right;
		}
		*next = node;
		node->insert(&_root, parent);

		return iterator(node, node->prev());
	}

	std::pair<iterator, bool> insert_unique(reference elem)
	{
		rbtree_hook*  node   = member_of<rbtree_hook>(&elem, NodeMember);
		rbtree_hook*  parent = nullptr;
		rbtree_hook** next   = &_root;
		Compare       cmp;

		while (*next) {
			parent = *next;
			if (cmp(elem, *parent_of<T>(*next, NodeMember)))
				next = &(*next)->_left;
			else if (cmp(*parent_of<T>(*next, NodeMember), elem))
				next = &(*next)->_right;
			else
				return std::pair<iterator, bool>(iterator(*next, (*next)->prev()), false);
		}
		*next = node;
		node->insert(&_root, parent);

		return std::pair<iterator, bool>(iterator(node, node->prev()), true);
	}

	template<class KeyT, class KeyCompareT>
	iterator find(const KeyT& key, KeyCompareT cmp)
	{
		rbtree_hook* next = _root;

		while (next) {
			if (cmp(key, *parent_of<T>(next, NodeMember)))
				next = next->_left;
			else if (cmp(*parent_of<T>(next, NodeMember), key))
				next = next->_right;
			else
				return iterator(next, next->prev());
		}

		return end();
	}

	template<class KeyT, class KeyCompareT>
	const_iterator find(const KeyT& key, KeyCompareT cmp) const
	{
		rbtree_hook* next = const_cast<rbtree_hook*>(_root);

		while (next) {
			if (cmp(key, *parent_of<T>(next, NodeMember)))
				next = next->_left;
			else if (cmp(*parent_of<T>(next, NodeMember), key))
				next = next->_right;
			else
				return const_iterator(next, next->prev());
		}

		return end();
	}

	void remove(iterator& i)
	{
		member_of<rbtree_hook>(&i, NodeMember)->remove(&_root);
	}

	void remove(reference elem)
	{
		member_of<rbtree_hook>(&elem, NodeMember)->remove(&_root);
	}

	template<class Disposer>
	void clear_and_dispose(Disposer disposer)
	{
		if (!_root)
			return;

		rbtree_hook* next = _root->min();
		rbtree_hook* prev;

		while (next) {
			prev = next;
			next = next->next();

			disposer(parent_of<T>(next, NodeMember));
		}
		_root = nullptr;
	}

	bool empty() const
	{
		return !_root;
	}

	void swap(rbtree& t)
	{
		std::swap(_root, t._root);
	}

private:
	rbtree_hook* _root;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_RBTREE__HPP_ */
