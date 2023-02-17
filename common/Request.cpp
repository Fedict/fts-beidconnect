#include "Request.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "util.h"

#define WHERE "Request::valueForKey"
std::string Request::valueForKey(std::string key)
{
    //   std::vector<char> getCert
    std::stringstream ss(ssRequest->str());
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    return (pt.get<std::string>(key));
    //   int l_cert = base64decode_len((unsigned char*) certif.c_str());
    //   unsigned char *cert = (unsigned char*) malloc(l_cert);
    //   if (cert == 0) {
    //      log_error("%s mem alloc failed for cert (%d)", WHERE, l_cert);
    //   }
    //   l_cert = base64decode((unsigned char*)certif.c_str(), cert);
}
#undef WHERE