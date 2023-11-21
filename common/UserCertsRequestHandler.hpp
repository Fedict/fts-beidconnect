#pragma once

#ifndef UserCertsRequestHandler_hpp
#define UserCertsRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class UserCertsRequestHandler : public RequestHandler
{
public:
    UserCertsRequestHandler(const std::shared_ptr<boost::property_tree::ptree>& ptreeRequest) : RequestHandler(ptreeRequest) {};
    virtual ~UserCertsRequestHandler(){}; // without this destructor of derived handler is not called
    std::string process() override;
};

#endif
