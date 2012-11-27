//=======================================================================================================
// Brief   : Configuration DSL
// Authors : Bruno Santos <bsantos@av.it.pt>
// ------------------------------------------------------------------------------------------------------
// OPMIP - Open Proxy Mobile IP
//
// Copyright (C) 2012 Universidade de Aveiro
// Copyrigth (C) 2012 Instituto de Telecomunicações - Pólo de Aveiro
//
// This software is distributed under a license. The full license
// agreement can be found in the file LICENSE in this distribution.
// This software may not be copied, modified, sold or distributed
// other than expressed in the named license agreement.
//
// This software is distributed without any warranty.
//=======================================================================================================

#include <opmip/conf.hpp>
#include <boost/spirit/include/qi_parse_attr.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace opmip { namespace conf {

/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool exec(std::string::const_iterator& begin, std::string::const_iterator end, functions const& cm)
{
	skipper_grammar<std::string::const_iterator> skipper;
	parser_grammar<std::string::const_iterator>  parser(cm);
	std::string::const_iterator pos = begin;

	while (begin != end) {
		if (!qi::phrase_parse(begin, end, parser, skipper) || pos == begin)
			return false;
		pos = begin;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
} /* namespace conf */ } /* namespace opmip */

// EOF //////////////////////////////////////////////////////////////////////////////////////////////////
