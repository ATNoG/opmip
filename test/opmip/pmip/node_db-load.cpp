//=============================================================================
// Brief   : Node DB
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
#include <opmip/pmip/node_db.hpp>
#include <iostream>
#include <fstream>

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	opmip::pmip::node_db db;
	std::ifstream in;
	size_t cnt;

	if (argc != 2) {
		std::cerr << "usage: node-dabase-file\n\n";
		return 1;
	}

	in.open(argv[1]);
	if (!in) {
		std::cerr << "failed to open database file: " << argv[1] << "\n\n";
		return 1;
	}

	try {
		cnt = db.load(in);
		std::cout << "loaded " << cnt << " node entries from database file\n";

	} catch (std::exception& e) {
		std::cerr << "database parse error: " << e.what() << "\n\n";
		return 1;
	}
}

// EOF ////////////////////////////////////////////////////////////////////////
