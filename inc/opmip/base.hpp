//=============================================================================
// Brief   : Base Types and Definitions
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

#ifndef OPMIP_BASE__HPP_
#define OPMIP_BASE__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <cstddef>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

///////////////////////////////////////////////////////////////////////////////
#define OPMIP_SCOPED(def)     switch(def) case 0: default:
#define OPMIP_COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

#define OPMIP_NOT_USED(x) (void)x;

#ifdef BOOST_HAS_STATIC_ASSERT
#	define OPMIP_STATIC_ASSERT(exp, reason) static_assert(exp, reason)
#else
#	define OPMIP_STATIC_ASSERT(exp, reason) BOOST_STATIC_ASSERT(exp)
#endif

#define OPMIP_UNDEFINED_BOOL                           \
	struct undefined_bool_t {                          \
		void true_() {}                                \
	};                                                 \
	typedef void (undefined_bool_t::*undefined_bool)()

#define OPMIP_UNDEFINED_BOOL_TRUE  &undefined_bool_t::true_
#define OPMIP_UNDEFINED_BOOL_FALSE 0

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
typedef signed char        schar;
typedef signed short       sshort;
typedef signed int         sint;
typedef signed long        slong;
typedef signed long long   sllong;

typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long ullong;

typedef boost::uint8_t     uint8;
typedef boost::uint16_t    uint16;
typedef boost::uint32_t    uint32;
typedef boost::uint64_t    uint64;

typedef boost::int8_t      sint8;
typedef boost::int16_t     sint16;
typedef boost::int32_t     sint32;
typedef boost::int64_t     sint64;

typedef boost::intmax_t    sintmax;
typedef boost::uintmax_t   uintmax;

typedef std::size_t        size_t;
typedef std::ptrdiff_t     ptrdiff_t;

///////////////////////////////////////////////////////////////////////////////
struct nullptr_t { template<class T> operator T*() const { return 0; } };

static const nullptr_t nullptr = nullptr_t();

///////////////////////////////////////////////////////////////////////////////
template<class MemberT, class ParentT>
inline MemberT* member_of(ParentT* parent, MemberT ParentT::* Member)
{
	return &(parent->*Member);
}

template<class ParentT, class MemberT>
inline ParentT* parent_of(MemberT* member, MemberT ParentT::* Member)
{
	ParentT* parent = 0;

	if (!member)
		return nullptr;

	return reinterpret_cast<ParentT*>(reinterpret_cast<char*>(member) - reinterpret_cast<const char*>(&(parent->*Member)));
}

///////////////////////////////////////////////////////////////////////////////
template<class T, class U>
inline T offset_cast(U* from, size_t offset)
{
	//TODO: add support for references
	//      make sure this only works with pointers and references

	uchar* p = reinterpret_cast<uchar*>(from);

	return reinterpret_cast<T>(p + offset);
}

///////////////////////////////////////////////////////////////////////////////
template<class T>
inline T remaining_length(T length, T position)
{
	if (position > length)
		return 0;

	return length - position;
}

///////////////////////////////////////////////////////////////////////////////
template<size_t Alignment, class Integral>
inline Integral align_to(Integral value)
{
	//FIXME/TODO:
	// - add concept check for Integral and Alignment
	// - add specializations for pointers

	return (value + Alignment - 1) & ~(Alignment - 1);
}

template<size_t Alignment, size_t Value>
struct align_to_ {
	typedef size_t    type;
	static const type value = (Value + Alignment - 1) & ~(Alignment - 1);
};

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_BASE__HPP_ */
