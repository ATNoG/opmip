//=============================================================================
// Brief   : Filesystem Utilities
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

#ifndef OPMIP_FSUTIL__HPP_
#define OPMIP_FSUTIL__HPP_

///////////////////////////////////////////////////////////////////////////////
#include <opmip/base.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace opmip {

///////////////////////////////////////////////////////////////////////////////
inline boost::tribool fs_read_bool(const boost::filesystem::path& pathname)
{
	boost::filesystem::ifstream in(pathname);
	bool tmp;

	in >> tmp;

	return (!in) ? boost::tribool() : boost::tribool(tmp);
}

///////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF ////////////////////////////////////////////////////////////////////////
#endif /* OPMIP_FSUTIL__HPP_ */
