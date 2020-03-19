//
//  VersionRequestHandler.cpp
//  eIDLink
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#ifndef CertChainRequestHandler_hpp
#define CertChainRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class CertChainRequestHandler: public RequestHandler
{
public:
   CertChainRequestHandler() {};
   virtual ~CertChainRequestHandler() {};  //without this destructor of derived handler is not called
   std::string process() override;
};

#endif /* InfoRequestHandler */
