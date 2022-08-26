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

//#define message1     "{\"operation\":\"SIGN\", \"cert\": \"MIIDazCCAvGgAwIBAgIQEAAAAAAA8evx/gAAAAFW1jAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDA4MTcyMjAwMDBaFw0zMDA4MTcyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTg2NDB2MBAGByqGSM49AgEGBSuBBAAiA2IABK7UrQFh7jIQz3p6VEx34L+Ol4m21gU06yyrva4p7p+16f8ysGsS9BTzxJC0wKtE6x3F/UWyDENz2oJC/jPX4K1q/Gpz/4qpV7TBaKKDrSfHChktOB9eVJhBEdOWcBEgNKOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIwSKEUbcxy/Qmer+VuBPAk/U0g5bYvU6UWpdcFX/COSDhr8FR27kFGS8Z6Fu/WNEVVAjEA0kOwqkq3g/9BoBRfLCuPQ9LWpHCfNt1kQSWGJEZ2gQyOOB6L5p4Uu8JjEL0azZHL\", \
//    \"algo\":\"SHA-256\", \
//    \"digest\":\"JMRVmmssdqPelSUruIhdDOTGXc2Y2dOq8Bf989ZDPH0=\", \
//    \"pin\": \"1234\", \
//    \"language\":\"en\", \
//    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
//    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
//    \"origin\":\"https://sign.belgium.be\" }"
#define message_Applet1_8_SHA256     "{\"operation\":\"SIGN\", \"cert\": \"MIIDazCCAvGgAwIBAgIQEAAAAAAA8evx/gAAAAGOsjAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDEwMjIyMjAwMDBaFw0zMDEwMjIyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTg2NDB2MBAGByqGSM49AgEGBSuBBAAiA2IABEagJVhqmu6OTkMuVtdgNGYgh2qs95YNMX13+E4wT//T4jGJ4I3q4yc+NGx+ZilGYPhxevKR1+PgmS7vR8yRA4FEWpmVRiIG2agrmVjzjlqsEHLty8W2NewPilIRwvmb0KOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIxAMAFskETLSpdxA52NLJB/ztbsKYr2x2YEZNqCzYGKLk6ubiDF74behZ9PK02Q48XqAIwYfLa5BpTDfK9muyDDG9+VZ/R3YwM1s31CqiWBpPUGGNjGBIOqqv0A8SewjElW4CZ\", \
    \"algo\":\"SHA256\", \
    \"digest\":\"H//bub3+V6UP3gk9/yofBaCLS6+HoPE7DOi4CRTfc+U=\", \
    \"pin\": \"null\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"
#define message_Applet1_8_SHA384     "{\"operation\":\"SIGN\", \"cert\": \"MIIDazCCAvGgAwIBAgIQEAAAAAAA8evx/gAAAAGOsjAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDEwMjIyMjAwMDBaFw0zMDEwMjIyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTg2NDB2MBAGByqGSM49AgEGBSuBBAAiA2IABEagJVhqmu6OTkMuVtdgNGYgh2qs95YNMX13+E4wT//T4jGJ4I3q4yc+NGx+ZilGYPhxevKR1+PgmS7vR8yRA4FEWpmVRiIG2agrmVjzjlqsEHLty8W2NewPilIRwvmb0KOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIxAMAFskETLSpdxA52NLJB/ztbsKYr2x2YEZNqCzYGKLk6ubiDF74behZ9PK02Q48XqAIwYfLa5BpTDfK9muyDDG9+VZ/R3YwM1s31CqiWBpPUGGNjGBIOqqv0A8SewjElW4CZ\", \
    \"algo\":\"SHA384\", \
    \"digest\":\"wj0jMglI2lnsNlcJmJmOLpWMyrl4r97CGbxKQMIpf1PtkaEnhQXe47AznDlhszCV\", \
    \"pin\": \"null\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"
#define message_Applet1_8_SHA512     "{\"operation\":\"SIGN\", \"cert\": \"MIIDazCCAvGgAwIBAgIQEAAAAAAA8evx/gAAAAGOsjAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDEwMjIyMjAwMDBaFw0zMDEwMjIyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTg2NDB2MBAGByqGSM49AgEGBSuBBAAiA2IABEagJVhqmu6OTkMuVtdgNGYgh2qs95YNMX13+E4wT//T4jGJ4I3q4yc+NGx+ZilGYPhxevKR1+PgmS7vR8yRA4FEWpmVRiIG2agrmVjzjlqsEHLty8W2NewPilIRwvmb0KOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIxAMAFskETLSpdxA52NLJB/ztbsKYr2x2YEZNqCzYGKLk6ubiDF74behZ9PK02Q48XqAIwYfLa5BpTDfK9muyDDG9+VZ/R3YwM1s31CqiWBpPUGGNjGBIOqqv0A8SewjElW4CZ\", \
    \"algo\":\"SHA512\", \
    \"digest\":\"lsk2FCVI0FR/zGpbZ1w94KZYMvxon8Z+3hyjpkESnrulRCdpUMhKa/4XjLILG3PmMaCkJiwxvbietHgCHB5LTw==\", \
    \"pin\": \"null\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"

#define message_Applet1_7_SHA256     "{\"operation\":\"SIGN\", \"cert\": \"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\", \
    \"algo\":\"SHA256\", \
    \"digest\":\"H//bub3+V6UP3gk9/yofBaCLS6+HoPE7DOi4CRTfc+U=\", \
    \"pin\": \"null\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"
#define message_Applet1_7_SHA384     "{\"operation\":\"SIGN\", \"cert\": \"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\", \
    \"algo\":\"SHA384\", \
    \"digest\":\"wj0jMglI2lnsNlcJmJmOLpWMyrl4r97CGbxKQMIpf1PtkaEnhQXe47AznDlhszCV\", \
    \"pin\": \"null\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"
#define message_Applet1_7_SHA512     "{\"operation\":\"SIGN\", \"cert\": \"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\", \
    \"algo\":\"SHA512\", \
    \"digest\":\"z6L23f6NABm76gmf3YvM8IwRf0lw1D+ZT/ULB65nM5/j0ZEUlwVhvY1sxBHl5WEbsPwcplppOE53CqHD8Fmf7Q==\", \
    \"pin\": \"null\", \
    \"language\":\"en\", \
    \"mac\":\"0123456789ABCDEF0123456789ABCDEF\", \
    \"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\", \
    \"origin\":\"https://sign.belgium.be\" }"

#define message2   "{\"operation\": \"ID\", \"idflags\": \"511\", \"correlationId\": \"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\"}"

int runTest(int argc, const char * argv[])
{
   shared_ptr<stringstream> ssRequest;
   shared_ptr<RequestHandler> handler;
   
   string ssResponse;
   
   //ssRequest = std::make_shared<stringstream>(message_Applet1_7_SHA256);
   //handler = RequestHandler::createRequestHandler(ssRequest);
   //
   //ssResponse = handler->process();

   //log_info(ssResponse.c_str());
   //
   //ssRequest = std::make_shared<stringstream>(message_Applet1_7_SHA384);
   //handler = RequestHandler::createRequestHandler(ssRequest);
   //
   //ssResponse = handler->process();
   //
   //log_info(ssResponse.c_str());

   //ssRequest = std::make_shared<stringstream>(message_Applet1_7_SHA512);
   //handler = RequestHandler::createRequestHandler(ssRequest);
   //
   //ssResponse = handler->process();
   //
   //log_info(ssResponse.c_str());

   ssRequest = std::make_shared<stringstream>(message_Applet1_8_SHA256);
   handler = RequestHandler::createRequestHandler(ssRequest);
   
   ssResponse = handler->process();

   log_info(ssResponse.c_str());
   
   //ssRequest = std::make_shared<stringstream>(message_Applet1_8_SHA384);
   //handler = RequestHandler::createRequestHandler(ssRequest);
   //
   //ssResponse = handler->process();
   //
   //log_info(ssResponse.c_str());

   //ssRequest = std::make_shared<stringstream>(message_Applet1_8_SHA512);
   //handler = RequestHandler::createRequestHandler(ssRequest);
   //
   //ssResponse = handler->process();
   //
   //log_info(ssResponse.c_str());

   return 0;
}


