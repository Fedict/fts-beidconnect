//
//  InfoRequestHandler.cpp
//  eIDLink
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#ifndef InfoRequestHandler_hpp
#define InfoRequestHandler_hpp

#include <iostream>
#include "RequestHandler.hpp"

class InfoRequestHandler: public RequestHandler
{
public:
   InfoRequestHandler() {};
   virtual ~InfoRequestHandler() {};  //without this destructor of derived handler is not called
   std::string process() override;
};

#endif /* InfoRequestHandler */
