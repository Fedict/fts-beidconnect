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

#define message     "{\"operation\":\"SIGN\", \"cert\": \"MIIF1TCCA72gAwIBAgIQEAAAAAAA8evx4wAAAAGTsTANBgkqhkiG9w0BAQsFADAtMQswCQYDVQQGEwJCRTEeMBwGA1UEAwwVZUlEIFRFU1QgRm9yZWlnbmVyIENBMB4XDTE3MDUyOTIyMDAwMFoXDTIwMDUyOTIyMDAwMFowfTEUMBIGA1UEBRMLOTgwODIxMDAwMjkxEzARBgNVBCoMCkJlcnQgQWxpY2UxGDAWBgNVBAUTD1NwZWNpbWVuIGVWSyAxODEpMCcGA1UEAwwgQmVydCBTcGVjaW1lbiBlVksgMTggKFNpZ25hdHVyZSkxCzAJBgNVBAYTAkJFMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA49oyeB2rwqGNkwENXhBuZzXWKJN4RESxd+oLUb8jliwzb2KWk97nk7FhvBGAM7FFV6EP6lZKXmByMsUrP0f0zYkwsPl2SQVMJRiPpBdbE+ZyJVxersSdaTysDuu2RZKR+F1PbwQMEEfGgH4cJ2UVOb/tV4MjJV5KlnrV13IHFESJh/xbJA//Wphjh5kCNpdnN6TWolN3218hJkAvV+98no3MHg0uzQ01NkBF2U0x5llhlJAVX5ua+IZ6k56BL/Uxo6jMInLxzaNxCcK2glSbPyJMXMj01sNTIW58NBMMTktQkkrTX+DGH8edf38xjmsBBOIBf3VptYdwiwjgEmaAWwIDAQABo4IBnzCCAZswHwYDVR0jBBgwFoAU/TbPuTELnWd7LuET5UCD9wgdjJgwDgYDVR0PAQH/BAQDAgZAMGkGA1UdIARiMGAwXgYHYDgMAQECATBTMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MB4GCCsGAQUFBwICMBIaEFRFU1QgdXNlciBub3RpY2UwIgYIKwYBBQUHAQMEFjAUMAgGBgQAjkYBATAIBgYEAI5GAQQwRQYDVR0fBD4wPDA6oDigNoY0aHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY3JsL2ZvcmVpZ25lcmNhLmNybDB/BggrBgEFBQcBAQRzMHEwPAYIKwYBBQUHMAKGMGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQvcm9vdGNhLmNydDAxBggrBgEFBQcwAYYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmU6ODg4ODARBglghkgBhvhCAQEEBAMCBSAwDQYJKoZIhvcNAQELBQADggIBABlETs6MUuqAnCU1yQaNkXa+JTNZ02XNOMTbmwhBLTcqnfyM5vxIu4OwBAfThgf735pr3ffe8kXFelypegV9tNufdmRjqAFq9CVcNbMTsViAd3/TaPCIhveicD4a78KBKwMcEXWvmsoHlYAdM5nZokpJg7sX2NhpjVC8JexJLJKiPRNvSzKTmuUZtus0M1f3E4dYGjQMH3UpVp3bE5OAo//2Y29P6O82Nfi2vJjxAE6YBzAQltb+oEuhKiR0HS/YSgXaYPULlRmPyxetd30tZejgufNyZWjl2+v7xvBym+wBf64Vft+pgO0Dxdq6wycjFF+MtdsfhGQ7Fq7KWut8W7sLzhuXTvheFCzxQzsabQG4kqOZtC7cj32usiL8wb82NdonrLXYnze0RBb3S0azcldcQbEwYX/UInFOnHsKOALTR8Ho4uQRVDmT2XZEHlXWCVg5sA3sT0tWZCfDaoTPkjXMZwVVdat8Os/pb7pBrVe7aig36ME0chC3BscToCM2g56hEWxTy28tOR98O0jEfpxmkL4eJRwhkd610LuzwSkRrhUBZZwvc94aD6F3njxbFXB6xGN53qlwsJXPxFLocRP4a57fwaoU8MVYuEkceOdYOR4G/G0Kfm3bIdTEJ44jIha1UATNIQ287it7UfPYdmRAi0X0JaCX9U6DEg0vSUqi\", \
    \"algo\":\"SHA-256\", \
    \"digest\":\"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=\", \
    \"pin\": \"1234\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://eazysign-qa.zetes.be\" }"


int runTest(int argc, const char * argv[])
{
   shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>(message);
   
   shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);
   
   string ssResponse = handler->process();
   
   log_info(ssResponse.c_str());
   
   return 0;
}


