#ifndef InfoRequestHandler_hpp
#define InfoRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class InfoRequestHandler : public RequestHandler
{
public:
    InfoRequestHandler(const std::shared_ptr<boost::property_tree::ptree>& ptreeRequest) : RequestHandler(ptreeRequest) {};
    virtual ~InfoRequestHandler(){}; // without this destructor of derived handler is not called
    std::string process() override;
};

#endif /* InfoRequestHandler */