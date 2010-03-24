//=============================================================================
// Brief   : RT Netlink Attribute
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

#ifndef OPMIP_SYS_RTNETLINK_ATTRIBUTE__HPP_
#define OPMIP_SYS_RTNETLINK_ATTRIBUTE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
template<class T>
struct attribute {
	attribute* next();
	T*         data();
	size_t     length() const;


	ushort _length;
	ushort _type;
};

template<class T>
inline attribute<T>* attribute<T>::next()
{
	uchar* tmp = reinterpret_cast<uchar*>(this) + align_to<4>(_length);

	return reinterpret_cast<attribute*>(tmp);
}

template<class T>
inline T* attribute<T>::data()
{
	uchar* tmp = reinterpret_cast<uchar*>(this) + align_to<4>(sizeof(*this));

	return reinterpret_cast<T*>(tmp);
}

template<class T>
inline size_t attribute<T>::length() const
{
	return _length - align_to<4>(sizeof(*this));
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_SYS_RTNETLINK_ATTRIBUTE__HPP_ */
