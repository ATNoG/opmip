//=============================================================================
// Brief   : Disposer Utility
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

#ifndef OPMIP_DISPOSER__HPP_
#define OPMIP_DISPOSER__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
template<class T, class U = void>
struct disposer;

template<>
struct disposer<void, void>
 {
	void operator()(void*) const
	{
	}
};

template<class T>
struct disposer<T, void>
 {
	void operator()(T* p) const
	{
		delete p;
	}
};

template<class T, class U>
struct disposer {
	void operator()(T* p) const
	{
		delete static_cast<U*>(p);
	}
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_DISPOSER__HPP_ */
