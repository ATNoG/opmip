//==================================================================================================
// Brief   : Filesystem Utilities
// Authors : Bruno Santos <bsantos@av.it.pt>
// -------------------------------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2010-2012 Universidade de Aveiro
// Copyrigth (C) 2010-2012 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//==================================================================================================

#include <opmip/fsutil.hpp>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip {

////////////////////////////////////////////////////////////////////////////////////////////////////
boost::tribool fs_read_bool(const boost::filesystem::path& pathname)
{
	boost::filesystem::ifstream in(pathname);
	uint tmp;

	if (!in)
		return boost::tribool();

	in >> tmp;
	return boost::tribool(tmp);
}

bool fs_write_bool(const boost::filesystem::path& pathname, bool value)
{
	boost::filesystem::ofstream out(pathname);

	if (!out)
		return false;

	out << uint(value);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace opmip */

// EOF /////////////////////////////////////////////////////////////////////////////////////////////
