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

#define message1     "{\"operation\":\"SIGN\", \"cert\": \"MIIDazCCAvGgAwIBAgIQEAAAAAAA8evx/gAAAAFW1jAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDA4MTcyMjAwMDBaFw0zMDA4MTcyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTg2NDB2MBAGByqGSM49AgEGBSuBBAAiA2IABK7UrQFh7jIQz3p6VEx34L+Ol4m21gU06yyrva4p7p+16f8ysGsS9BTzxJC0wKtE6x3F/UWyDENz2oJC/jPX4K1q/Gpz/4qpV7TBaKKDrSfHChktOB9eVJhBEdOWcBEgNKOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIwSKEUbcxy/Qmer+VuBPAk/U0g5bYvU6UWpdcFX/COSDhr8FR27kFGS8Z6Fu/WNEVVAjEA0kOwqkq3g/9BoBRfLCuPQ9LWpHCfNt1kQSWGJEZ2gQyOOB6L5p4Uu8JjEL0azZHL\", \
    \"algo\":\"SHA-256\", \
    \"digest\":\"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=\", \
    \"pin\": \"1234\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"

#define message2   "{\"operation\": \"ID\", \"idflags\": \"511\", \"correlationId\": \"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\"}"

int runTest(int argc, const char * argv[])
{
   shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>(message2);
   
   shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);
   
   string ssResponse = handler->process();
   
   log_info(ssResponse.c_str());
   
   return 0;
}


