//=============================================================================
// Brief   : Chrono Unit Test
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

#include <opmip/base.hpp>
#include <opmip/chrono.hpp>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
int main()
{
	opmip::chrono cr;

	cr.start();
	cr.stop();
	std::cout << "Chrono: " << cr.get() << std::endl;
}

// EOF ////////////////////////////////////////////////////////////////////////
