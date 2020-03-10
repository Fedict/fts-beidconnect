//
//  VersionRequestHandler.cpp
//  eIDLink
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#ifndef UserCertsRequestHandler_hpp
#define UserCertsRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class UserCertsRequestHandler: public RequestHandler
{
public:
   UserCertsRequestHandler() {};
   virtual ~UserCertsRequestHandler() {};  //without this destructor of derived handler is not called
   std::string process() override;
};

#endif /* InfoRequestHandler */
