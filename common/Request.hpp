//
//  Request.hpp
//  BeIDConnect
//
//  Created by Vital on 19/03/2020.
//  Copyright © 2020 bosa. All rights reserved.
//

#ifndef Request_hpp
#define Request_hpp

#include <stdio.h>

#include <iostream>
#include "RequestHandler.hpp"

class Request
{
public:
   Request() {};
   virtual ~Request() {};

   std::string valueForKey(std::string key);
private:
   std::shared_ptr <std::stringstream> ssRequest;
};

#endif /* Request_hpp */
