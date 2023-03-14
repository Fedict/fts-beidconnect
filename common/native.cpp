#include <fcntl.h>
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

int runNative(int argc, const char *argv[])
{
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    //while (1)
    {
        shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>();

        log_info("runNative : Read new Request");
        switch (readMessage(*ssRequest))
        {
        case BECOM_Code::E_COMM_ENDREQUEST:
        {
            log_info("runNative : Close Request");
        }
        case BECOM_Code::E_COMM_PARAM:
        {
            log_info("runNative : Comm Parameter error");
            return 0;
        }
        default:
            break;
        }

        shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);

        string ssResponse = handler->process();

        sendMessage(ssResponse);
    }
    return 0;
}