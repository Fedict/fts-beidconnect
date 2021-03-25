#include "log.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <sstream>
#include "util.h"
#include "general.h"
#include "comm.hpp"

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;

#define message     "{\"operation\":\"SIGN\", \"cert\": \"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\", \
    \"algo\":\"SHA-256\", \
    \"digest\":\"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=\", \
    \"pin\": \"1234\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"


int runTest(int argc, const char * argv[])
{
   shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>(message);
   
   shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);
   
   string ssResponse = handler->process();
   
   log_info(ssResponse.c_str());
   
   return 0;
}


