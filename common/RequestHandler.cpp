#include "RequestHandler.hpp"
#include "VersionRequestHandler.hpp"
#include "IDRequestHandler.hpp"
#include "InfoRequestHandler.hpp"
#include "UserCertsRequestHandler.hpp"
#include "CertChainRequestHandler.hpp"
#include "SignRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "log.hpp"
#include <unordered_map>

using namespace boost::property_tree;

std::shared_ptr<RequestHandler> RequestHandler::createRequestHandler(const std::shared_ptr<std::stringstream>& ssRequest)
{
    std::shared_ptr<boost::property_tree::ptree> ptreeRequestTmp = std::make_shared<boost::property_tree::ptree>();

    try
    {
        std::stringstream ss(ssRequest->str());
        boost::property_tree::read_json(ss, *ptreeRequestTmp);
    }
    catch (std::exception &e)
    {
        log_error("error: %s", e.what());
    }

    std::shared_ptr<RequestHandler> requestHandler = nullptr;
    std::string operation = ptreeRequestTmp->get<std::string>("operation");

    if (operation == "VERSION")
    {
        requestHandler = std::make_shared<VersionRequestHandler>(ptreeRequestTmp);
    }
    else if (operation == "ID")
    {
        requestHandler = std::make_shared<IDRequestHandler>(ptreeRequestTmp);
    }
    else if (operation == "USERCERTS")
    {
        requestHandler = std::make_shared<UserCertsRequestHandler>(ptreeRequestTmp);
    }
    else if (operation == "CERTCHAIN")
    {
        requestHandler = std::make_shared<CertChainRequestHandler>(ptreeRequestTmp);
    }
    else if (operation == "SIGN")
    {
        requestHandler = std::make_shared<SignRequestHandler>(ptreeRequestTmp);
    }
    else if (operation == "AUTH")
    {
        requestHandler = std::make_shared<SignRequestHandler>(ptreeRequestTmp);
    }
    else
    {
        log_error("Unknown operation <%s>", operation.c_str());
    }

    return (requestHandler);
}

void RequestHandler::post_process(boost::property_tree::ptree &response)
{
    if (ptreeRequest)
    {
        std::string correlationId = ptreeRequest->get<std::string>("correlationId");
        if (correlationId != "")
        {
            response.put("correlationId", correlationId);
        }
        std::string operation = ptreeRequest->get<std::string>("operation");
        if (operation != "")
        {
            response.put("operation", operation);
        }
    }
}
