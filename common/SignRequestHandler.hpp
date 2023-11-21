#pragma once

#ifndef SignRequestHandler_hpp
#define SignRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class SignRequestHandler : public RequestHandler
{
public:
    SignRequestHandler(const std::shared_ptr<boost::property_tree::ptree>& ptreeRequest) : RequestHandler(ptreeRequest) {};
    virtual ~SignRequestHandler(){}; // without this destructor of derived handler is not called
    std::string process() override;
};

#endif
