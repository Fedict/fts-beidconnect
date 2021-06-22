//
//  IDRequestHandler.cpp
//  BeIDConnect
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#ifndef IDRequestHandler_hpp
#define IDRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class IDRequestHandler: public RequestHandler
{
public:
   IDRequestHandler() {};
   virtual ~IDRequestHandler() {};  //without this destructor of derived handler is not called
   std::string process() override;
};

#endif

