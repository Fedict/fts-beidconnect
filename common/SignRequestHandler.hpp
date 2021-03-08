//
//  SignRequestHandler.cpp
//  BeIDConnect
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#ifndef SignRequestHandler_hpp
#define SignRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class SignRequestHandler: public RequestHandler
{
public:
   SignRequestHandler() {};
   virtual ~SignRequestHandler() {};  //without this destructor of derived handler is not called
   std::string process() override;
};

#endif
