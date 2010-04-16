//=============================================================================
// Brief   : Mobile Access Gateway Main Entry Point
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
#include <opmip/pmip/mag.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <iostream>
#include "mag.hpp"

///////////////////////////////////////////////////////////////////////////////
int main()
{
	size_t concurrency = boost::thread::hardware_concurrency();
	boost::asio::io_service ios(concurrency);
	opmip::pmip::mag mag(ios);
	boost::thread_group tg;

	mag.start();

	for (size_t i = 1; i < concurrency; ++i)
		tg.create_thread(boost::bind(&boost::asio::io_service::run, &ios));

	try {
		ios.run();
		tg.join_all();

	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

// EOF ////////////////////////////////////////////////////////////////////////
