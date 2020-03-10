//
//  VersionRequestHandler.cpp
//  eIDLink
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "VersionRequestHandler.hpp"
#include "general.h"
#include <sstream>

using boost::property_tree::ptree;

std::string VersionRequestHandler::process()
{
   std::stringstream response;
   ptree pt;
      
   pt.put("result", "OK");
   pt.put("version", EIDLINK_VERSION);

   boost::property_tree::write_json(response, pt, false);
   
   return response.str();
}
