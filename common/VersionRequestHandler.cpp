#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "VersionRequestHandler.hpp"
#include "general.h"
#include "log.hpp"
#include "SCardException.h"

using boost::property_tree::ptree;

std::string VersionRequestHandler::process()
{
    ptree response;
    response.put(BeidConnect_JSON_field::result, BeidConnect_Result::OK);
    response.put(BeidConnect_JSON_field::version, BEIDCONNECT_VERSION);
    response.put(BeidConnect_JSON_field::builddate, __DATE__);

    // The beidconnect Windows version is provided by 2 installation type:
    // current user installation and machine (all users) installation defined as the ADMIN installation.
    // To allow the gui-sign to provide the correct update link if a new version
    // is detected, add the admin tag in the version report.
#ifdef ADMIN
    response.put("windowsInstallType", "admin");
#endif

    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);
    return streamResponse.str();
}
