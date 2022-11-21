#ifndef UserCertsRequestHandler_hpp
#define UserCertsRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class UserCertsRequestHandler : public RequestHandler
{
public:
    UserCertsRequestHandler(){};
    virtual ~UserCertsRequestHandler(){}; // without this destructor of derived handler is not called
    std::string process() override;
};

#endif
