//
//  VersionRequestHandler.cpp
//  BeIDConnect
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#ifndef VersionRequestHandler_hpp
#define VersionRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class VersionRequestHandler: public RequestHandler
{
public:
   VersionRequestHandler() {};
   virtual ~VersionRequestHandler() {};  //without this destructor of derived handler is not called
   std::string process() override;
};

#endif /* VersionRequestHandler_hpp */
