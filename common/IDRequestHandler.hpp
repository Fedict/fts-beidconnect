#pragma once

#ifndef IDRequestHandler_hpp
#define IDRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class IDRequestHandler : public RequestHandler
{
public:
    IDRequestHandler(const std::shared_ptr<boost::property_tree::ptree>& ptreeRequest) : RequestHandler(ptreeRequest) {};
    virtual ~IDRequestHandler(){}; // without this destructor of derived handler is not called
    std::string process() override;
};

#endif