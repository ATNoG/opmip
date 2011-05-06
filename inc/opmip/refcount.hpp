//=============================================================================
// Brief   : Reference Count Utilities
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

#ifndef OPMIP_REFCOUNT__HPP_
#define OPMIP_REFCOUNT__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/assert.hpp>
#include <boost/smart_ptr/detail/atomic_count.hpp>
#include <boost/type_traits/is_base_of.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
class refcount_base {
	friend class refcount_impl;

protected:
	refcount_base()
		: _refcnt(0)
	{ }

private:
	boost::detail::atomic_count _refcnt;
};

///////////////////////////////////////////////////////////////////////////////
class refcount_impl {
public:
	refcount_impl(refcount_base* p)
		: _ptr(p)
	{
		BOOST_ASSERT(p);
		++_ptr->_refcnt;
	}

	refcount_impl(const refcount_impl& app)
		: _ptr(const_cast<refcount_impl&>(app)._ptr)
	{
		BOOST_ASSERT(_ptr);
		BOOST_ASSERT(_ptr->_refcnt > 0);
		++_ptr->_refcnt;
	}

	refcount_impl(refcount_impl&& app)
		: _ptr(app._ptr)
	{
		BOOST_ASSERT(_ptr);
		BOOST_ASSERT(_ptr->_refcnt > 0);
		app._ptr = nullptr;
	}

	refcount_impl& operator=(const refcount_impl& app)
	{
		_ptr = const_cast<refcount_impl&>(app)._ptr;
		++_ptr->_refcnt;
		return *this;
	}

	refcount_impl& operator=(refcount_impl&& app)
	{
		_ptr = app._ptr;
		app._ptr = nullptr;
		return *this;
	}

	uint decref()
	{
		return (_ptr) ? --_ptr->_refcnt : 0;
	}

protected:
	refcount_base* _ptr;
};

///////////////////////////////////////////////////////////////////////////////
template<class T>
class refcount_ptr : refcount_impl {
	OPMIP_STATIC_ASSERT((boost::is_base_of<refcount_base, T>::value), "T must be a base of opmip::refcount_base");

	typedef refcount_impl base;

public:
	typedef T*       pointer;
	typedef const T* const_pointer;

public:
	refcount_ptr(pointer p)
		: base(p)
	{ }

	~refcount_ptr()
	{
		if (!base::decref())
			delete static_cast<pointer>(_ptr);
	}

	pointer operator->() { return static_cast<pointer>(_ptr); }
	pointer get()        { return static_cast<pointer>(_ptr); }
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

///////////////////////////////////////////////////////////////////////////////
namespace boost {

///////////////////////////////////////////////////////////////////////////////
template<class T>
T* get_pointer(::opmip::refcount_ptr<T>& p)
{
	return p.get();
}

///////////////////////////////////////////////////////////////////////////////
}

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_REFCOUNT__HPP_ */
