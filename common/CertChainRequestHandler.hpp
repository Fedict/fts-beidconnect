#pragma once

#ifndef CertChainRequestHandler_hpp
#define CertChainRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class CertChainRequestHandler : public RequestHandler
{
public:
    CertChainRequestHandler(const std::shared_ptr<boost::property_tree::ptree>& ptreeRequest) : RequestHandler(ptreeRequest) {};
    virtual ~CertChainRequestHandler(){}; // without this destructor of derived handler is not called
    std::string process() override;
};

#endif
