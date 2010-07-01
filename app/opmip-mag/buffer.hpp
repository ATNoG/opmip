//=============================================================================
// Brief   : Buffer
// Authors : Bruno Santos <bsantos@av.it.pt>
//
//
// Copyright (C) 2009 Universidade Aveiro - Instituto de Telecomunicacoes Polo Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=============================================================================

#ifndef OPMIP_BUFFER__HPP_
#define OPMIP_BUFFER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
template<class T>
class buffer : boost::noncopyable {
	OPMIP_STATIC_ASSERT(boost::is_pod<T>::value, "T must be POD type");

public:
	buffer() : _ptr(nullptr), _len(0)
	{ }

	buffer(opmip::size_t len) : _ptr(nullptr)
	{
		size(len);
	}

	~buffer()
	{
		std::free(_ptr);
	}

	void size(opmip::size_t len)
	{
		if (len != _len) {
			void* p = std::realloc(_ptr, len * sizeof(T));

			if (!p && len)
				boost::throw_exception(std::bad_alloc());

			_ptr = reinterpret_cast<T*>(p);
			_len = len;
		}
	}

	void zero()
	{
		std::memset(_ptr, 0, _len * sizeof(T));
	}

	T*       get()        { return _ptr; }
	const T* get() const  { return _ptr; }
	opmip::size_t   size() const { return _len; }

private:
	T*     _ptr;
	opmip::size_t _len;
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_BUFFER__HPP_ */
