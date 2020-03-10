#include "log.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <sstream>
#include "util.h"
#include "general.h"
#include "comm.hpp"

#ifdef WIN32
#include <io.h>
#endif

using namespace std;

int runNative(int argc, const char * argv[])
{
   shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>();
   
   int ret = readMessage(*ssRequest);
   if (ret != 0) {
      log_error("runNative() E: readMessage returned (0x%08x)", "runNative", ret);
      return(ret);
   }
   
   shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);

   string ssResponse = handler->process();
   
   log_info(ssResponse.c_str());
   
   sendMessage(ssResponse);
   
   return 0;
}

