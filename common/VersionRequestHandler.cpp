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
#include "log.hpp"

using boost::property_tree::ptree;

std::string VersionRequestHandler::process()
{
   ptree response;
      
   response.put("result", "OK");
   response.put("version", EIDLINK_VERSION);

   post_process(response);
   std::stringstream streamResponse;
   boost::property_tree::write_json(streamResponse, response, false);
   return streamResponse.str();
}
