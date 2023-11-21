#include "log.hpp"
#include "RequestHandler.hpp"
#include <iostream>
#include <sstream>
#include <list>
#include "util.h"
#include "general.h"
#include "comm.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#include <Wincrypt.h>
#include <io.h>
#endif
#include "ReaderList.hpp"
#include "CardFactory.hpp"

using namespace std;

#ifdef _DEBUG
bool unittest_Generate_Exception_No_Reader = false;
bool unittest_Generate_Exception_Other = false;
bool unittest_Generate_Exception_Pin_Blocked = false;
bool unittest_Generate_Exception_Pin_3_attempts = false;
bool unittest_Generate_Exception_Pin_2_attempts = false;
bool unittest_Generate_Exception_Pin_1_attempt = false;
bool unittest_Generate_Exception_Src_Command_not_allowed = false;
bool unittest_Generate_Exception_Removed_Card = false;
bool unittest_Generate_Exception_Transaction_Fail = false;
bool unittest_Generate_Exception_PinPad_TimeOut = false;
bool unittest_Generate_Exception_PinPad_Cancel = false;
bool unittest_Generate_Exception_PIN_TOO_SHORT = false;
bool unittest_Generate_Exception_PIN_TOO_LONG = false;
bool unittest_Generate_Exception_PIN_Incorrect = false;
#endif

constexpr char TestHeader[] = ">>> TEST : ";
constexpr char message_SHA256[] = "{\"operation\":\"SIGN\",\"cert\":\"%%CERT%%\",\"algo\":\"SHA256\",\"digest\":\"H//bub3+V6UP3gk9/yofBaCLS6+HoPE7DOi4CRTfc+U=\",\"pin\":\"null\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\"}";
constexpr char message_SHA384[] = "{\"operation\":\"SIGN\",\"cert\":\"%%CERT%%\",\"algo\":\"SHA384\",\"digest\":\"wj0jMglI2lnsNlcJmJmOLpWMyrl4r97CGbxKQMIpf1PtkaEnhQXe47AznDlhszCV\",\"pin\":\"null\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\"}";
constexpr char message_SHA512[] = "{\"operation\":\"SIGN\",\"cert\":\"%%CERT%%\",\"algo\":\"SHA512\",\"digest\":\"lsk2FCVI0FR/zGpbZ1w94KZYMvxon8Z+3hyjpkESnrulRCdpUMhKa/4XjLILG3PmMaCkJiwxvbietHgCHB5LTw==\",\"pin\":\"null\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\" }";
constexpr char message_Get_Card_Info_ID_Only[] = "{\"operation\":\"ID\",\"idflags\":\"1\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\"}";
constexpr char message_Get_Card_Info[] = "{\"operation\":\"ID\",\"idflags\":\"511\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\"}";
constexpr char message_Get_User_Certificates[] = "{\"operation\":\"USERCERTS\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\"}";
constexpr char message_Get_Certificates_Chain[] = "{\"operation\":\"CERTCHAIN\",\"cert\":\"%%CERT%%\",\"language\":\"en\",\"mac\":\"0123456789ABCDEF0123456789ABCDEF\",\"correlationId\":\"07386ce7-f73e-4e99-dfc3-8d69b6adf33d\",\"origin\":\"https://sign.belgium.be\"}";

void dumpCert(const std::shared_ptr<const CardFile>& Cert) {
    std::cout << "------------------------------ Certificate ------------------------------" << endl;
    if (Cert->getBase64().length() == 0)
    {
        std::cout << "No certificate found" << endl << "-------------------------------------------------------------------------" << endl;
        return;
    }
    std::cout << Cert->getBase64() << endl << "-------------------------------------------------------------------------" << endl;

#ifdef _WIN32
    PCCERT_CONTEXT certContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, Cert->getRaw().data(), (DWORD)Cert->getRaw().size());
    if (certContext == NULL) return;
    //std::cout << certContext->pCertInfo->Subject.cbData;
    SYSTEMTIME st_NotBefore;
    SYSTEMTIME st_NotAfter;
    FileTimeToSystemTime(&(certContext->pCertInfo->NotBefore), &st_NotBefore);
    FileTimeToSystemTime(&(certContext->pCertInfo->NotAfter), &st_NotAfter);

    char str[256];
    DWORD strlen = 256;
    if (CertGetNameStringA(certContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, str, 128))
    {
        std::cout << str << endl;
    }

    if (certContext->pCertInfo->SerialNumber.cbData > 0)
    {
        std::cout << "Serial number ";
        for (DWORD i = 0; i < certContext->pCertInfo->SerialNumber.cbData; i++)
        {
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)certContext->pCertInfo->SerialNumber.pbData[-1+certContext->pCertInfo->SerialNumber.cbData-i];
        }
        std::cout << endl << std::dec;
    }

    std::cout << "Valid from " << st_NotBefore.wDay << "/" << st_NotBefore.wMonth << "/" << st_NotBefore.wYear << " to " << st_NotAfter.wDay << "/" << st_NotAfter.wMonth << "/" << st_NotAfter.wYear << endl;

    WCHAR wstr[256];
    DWORD wstrlen = 256;
    if (CertGetCertificateContextProperty(certContext, CERT_PUB_KEY_CNG_ALG_BIT_LENGTH_PROP_ID,
        wstr,
        &wstrlen))
    {
        std::cout << "Public key : ";
        std::wcout << wstr << endl;
    }

    CertFreeCertificateContext(certContext);

    std::cout << "-------------------------------------------------------------------------" << endl;
#endif
}


/*
* sample TestDB file
*
* ps: Base64 can contain "/" characters. They must be escaped in the json file
*
{
    "cards": [
        {
            "Description": "Applet 1.8 TEST eID Active/Active",
            "SignCert": "MIIDazCCAvGgAwIBAgIQEAAAAAAA8evx/gAAAAGOsjAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDEwMjIyMjAwMDBaFw0zMDEwMjIyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTg2NDB2MBAGByqGSM49AgEGBSuBBAAiA2IABEagJVhqmu6OTkMuVtdgNGYgh2qs95YNMX13+E4wT//T4jGJ4I3q4yc+NGx+ZilGYPhxevKR1+PgmS7vR8yRA4FEWpmVRiIG2agrmVjzjlqsEHLty8W2NewPilIRwvmb0KOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIxAMAFskETLSpdxA52NLJB/ztbsKYr2x2YEZNqCzYGKLk6ubiDF74behZ9PK02Q48XqAIwYfLa5BpTDfK9muyDDG9+VZ/R3YwM1s31CqiWBpPUGGNjGBIOqqv0A8SewjElW4CZ",
            "AuthCert": "MIIDQTCCAsegAwIBAgIQEAAAAAAA8evx/gAAAAGOrTAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMDEwMjIyMjAwMDBaFw0zMDEwMjIyMjAwMDBaMHYxCzAJBgNVBAYTAkJFMScwJQYDVQQDDB5Ob3JhIFNwZWNpbWVuIChBdXRoZW50aWNhdGlvbikxETAPBgNVBAQMCFNwZWNpbWVuMRUwEwYDVQQqDAxOb3JhIEFuZ8OobGUxFDASBgNVBAUTCzAxMDUwMzk5ODY0MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAER0e7XnFrdzHsQpefWohaz6093Umz9mhZ8WB7WFnxJ5K+M6lKARvSvgs26lTgMV14H8cSu1wiq/JRYvVViZuV7N6ot0j+SM0aM6sS9Ts6dcMxw8zGFf8Y+ORyQjWL/IOPo4IBYDCCAVwwHwYDVR0jBBgwFoAU3bN/45oZjlnJEVYCLfX6nW/ehEswDgYDVR0PAQH/BAQDAgeAMEkGA1UdIARCMEAwPgYHYDgMAQECAjAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MBMGA1UdJQQMMAoGCCsGAQUFBwMCMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIxAJG/7BHew6uR//66szBzh2WgNn5M/GFFcZKXHWyQ40IC6qGm206FEyjfkqjJ0dlDZQIwCXTHUlM3B+fSnvQPZWPaXxTTC5XL1rIDMB9/higX8NZxpwz+VzQjeKOngoc0hWZj",
            "CACert": "MIIDKTCCAq+gAwIBAgIIcND8I1qptLUwCgYIKoZIzj0EAwMwKzELMAkGA1UEBhMCQkUxHDAaBgNVBAMME2VJRCBURVNUIEVDIFJvb3QgQ0EwIBcNMDcwNDMwMjIwMDIwWhgPMjA4NzA0MTAyMjAwMjBaMC4xCzAJBgNVBAYTAkJFMR8wHQYDVQQDDBZlSUQgVEVTVCBFQyBDaXRpemVuIENBMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEJAiNoOQfY0r8N6JVPMLedXyRZ7MwppGwQ9ZxFzLjVsbeKuUvqEFR0yKKyEidXc875m4UF5lRpf/FSWagg2IXGWrypnRZkgnNVP6s5W2LzKdV09hd6v7O8j/8knfHOj+No4IBmTCCAZUwHQYDVR0OBBYEFN2zf+OaGY5ZyRFWAi31+p1v3oRLMB8GA1UdIwQYMBaAFCHAclfKHAQEGR3ZjH4+tYPrrBwCMA4GA1UdDwEB/wQEAwIBBjBIBgNVHSAEQTA/MD0GBmA4DAEBAjAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDBDBCBgNVHR8EOzA5MDegNaAzhjFodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jcmwvcm9vdGNhRUMuY3JsMIGBBggrBgEFBQcBAQR1MHMwPgYIKwYBBQUHMAKGMmh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQvcm9vdGNhRUMuY3J0MDEGCCsGAQUFBzABhiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZTo4ODg4MBIGA1UdEwEB/wQIMAYBAf8CAQAwCgYIKoZIzj0EAwMDaAAwZQIxAOMiiByF0aLEA6zUrobMw7aSH5o2u1hGVMe0AL4ezYztRdfxvXVU+m1JosBVBDDjeAIwYJJN7bLWw8BVi/lkxRjKL/+zAJP6djGywXI1pVh4HKb0D+tipq5StO+QnM8cnPmg",
            "ROOTCert": "MIICNDCCAbugAwIBAgIBATAKBggqhkjOPQQDAzArMQswCQYDVQQGEwJCRTEcMBoGA1UEAwwTZUlEIFRFU1QgRUMgUm9vdCBDQTAgFw0wNzA0MzAyMjAwMTBaGA8yMDg4MDQwOTIyMDAxMFowKzELMAkGA1UEBhMCQkUxHDAaBgNVBAMME2VJRCBURVNUIEVDIFJvb3QgQ0EwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAASYqtYIKayPGXFNDaGkPdCadQCSC8D2W8aKE7xh850ykG0bJXMV7IaKZWo0ZXUb55g9S95gjDNeZ0iNo75dY/mWoozI6I2l106OdPL+yAcHI6id4uR7Fd0nQxeBICdmjnCjgbAwga0wHQYDVR0OBBYEFCHAclfKHAQEGR3ZjH4+tYPrrBwCMB8GA1UdIwQYMBaAFCHAclfKHAQEGR3ZjH4+tYPrrBwCMA4GA1UdDwEB/wQEAwIBBjBHBgNVHSAEQDA+MDwGBWA4DAEBMDMwMQYIKwYBBQUHAgEWJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQwEgYDVR0TAQH/BAgwBgEB/wIBATAKBggqhkjOPQQDAwNnADBkAjBM2P48H8f2FY0NHm1uAdgXwYoBRkUFOq8Kccd7l6Y8RavzAkMQmLgVF3s5euuv6fcCMCW4UGWpnOTOA+t4V9/+kPMjGqgC9Uw4nOKkwkwQs3IeWfc7Na6l+U8r4M7VH49/cw==",
            "RRNCert": "MIICgzCCAgmgAwIBAgIIA1F0wfUKrp4wCgYIKoZIzj0EAwMwKzELMAkGA1UEBhMCQkUxHDAaBgNVBAMME2VJRCBURVNUIEVDIFJvb3QgQ0EwIBcNMDcwNDMwMjIwMDIwWhgPMjA1NzA0MTcyMjAwMTBaMDYxCzAJBgNVBAYTAkJFMQ0wCwYDVQQKDARURVNUMRgwFgYDVQQDDA9lSUQgVEVTVCBFQyBSUk4wdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARlTeyTcHbrwvN5elieA+y302LhHoXKHEmIVN+H/cHUNPLQrRwmY5TQerzJ/f+swsKGjM3p3n4oebT/bWo+AICDtPLBJcRO0Q2JaixV1lLXeGoSziwguT8L/oa6B36ytEOjgewwgekwHQYDVR0OBBYEFLV/8yWBmfffDwVYb3r3+DiB/9i4MB8GA1UdIwQYMBaAFCHAclfKHAQEGR3ZjH4+tYPrrBwCMA4GA1UdDwEB/wQEAwIGwDBIBgNVHSAEQTA/MD0GBmA4DAEBBDAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MEIGA1UdHwQ7MDkwN6A1oDOGMWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9yb290Y2FFQy5jcmwwCQYDVR0TBAIwADAKBggqhkjOPQQDAwNoADBlAjEA2ONS6D4ktQtjPZOTczWFHYDvHT1BfT+bAoNfPTCNB01QETa0A6S1PiV8m1/PQOGmAjB56VlGloc2Jz0fGbnXRH0TcQIOTUEqWDmrypoPC1PiKP8Som4j1wWqN2Ajx8zMzc8=",
            "DoPINOps": true,
            "PIN":  "1234",
            "IdFile": "AQwwMDAwMDEwMjA2MjECEFNMQlCRKQECikFlThKSknADCjIzLjEwLjIwMjAECjIzLjEwLjIwMzAFB0JydXNzZWwGCzAxMDUwMzk5ODY0BwhTcGVjaW1lbggMTm9yYSBBbmfDqGxlCQAKBEJFTEcLBkxldXZlbgwLMDMgTUVJIDIwMDENAVYOAA8BMRABMBEwP3nrz7GuTqVIsmsrjt60nYNd+cBIoT15KSH+0O\/mYKnBvHD0TwkcynUykHoEtj40FQAWABcAGAAZABowtfbtm4pQfHkWUQjIeXk59NDoL4568PUmXfAA4JM8rUzLUt6a2R8jNZootYNBDw0N",
            "AddrFile": "ARFTcGVjaW1lbnN0cmFhdCAxMgIEMTAwMAMHQnJ1c3NlbAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==",
            "Photo": "\/9j\/4AAQSkZJRgABAgEBLAEsAAD\/2wBDABwTFRgVERwYFhgfHRwhKUUtKSYmKVQ8QDJFZFhpZ2JYYF9ufJ6GbnWWd19giruLlqOpsbOxa4TC0MGszp6usar\/wAALCADIAIwBAREA\/8QA0gAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoLEAACAQMDAgQDBQUEBAAAAX0BAgMABBEFEiExQQYTUWEHInEUMoGRoQgjQrHBFVLR8CQzYnKCCQoWFxgZGiUmJygpKjQ1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4eLj5OXm5+jp6vHy8\/T19vf4+fr\/2gAIAQEAAD8A38n1NGT6mjJ9TRk+poyfU0ZPqaMn1NIWwMlsfjUf2qEdZ0\/77FPWVX+5IG+hzTsn1NGT6mjJ9TRk+poyfU0ZPqaMn1NGT6mkooooqnNqMUb7V+bB5OeKq3OpO4\/c\/KuMk55rKkvS5yWZiO7ZJqMXMnI2gj0K\/wD1qljncn5QjHsAf8mr0OoyIcHeh7g\/MP1rUgvI5sDoT6cj\/wCt+NWKKKKKKKKKRmVFLMQFUZJPQCub1DVpLlikBKxZ4GOvuaz1+Zz5shwO3rQ01tnHl5wOuTR58GP9WPypN8LdE2+4bH86nXdgeW4ZT2k\/xFTJN5ZCSAxk9A+Cv4GrA4PIHHY1qafPvXyyeFHyg9R7VdooooooorG1y5aRls4Wx\/FKR2HYf1\/Ksn5EUqmPc1Un25+U8\/Wog397mkx6Uqt2ZcipUTPMLEN6ZwfzqZbqRBsmQOvdWFWIZAiZt23xjqh6r9P8KuQyK2HU7cHhhwVNbNrcefH82BIPvKP51PRRRRRTJpBDC8rDIRS2B3x2rmLqRjudjhnbc5\/oKzZJS3AOBQqFzjGanSydlyeKY1uyn+tIIdynoGHamFGU9CMVNHKrDbOCR\/eHUUpV7ciSMhvcdGHoRVmGZZCHj+QnhlJ6\/wD1\/T1rTtZTGyyJk7eCvt6VsAggEHIPINLRRRRVDV5glusZBw53E46BcH+eP1rlrmUu3PrTYITK4ArctLAKASK0Bars6dqqXNjjkCsyaLZ7HsfSq4Kjg\/TjsailXutNjlK8dQ3UUp+Rt6\/dYYatewnLJycuvDD+Vblocxbc5A6fSp6KKKKw9flxJs3ZAQAj0PJP\/stc8Ms1bWl24ChiOvtW0i9OlTDpQQCOap3dksyEqMN\/OucvLd4ZCdpFVQ56HpTXAzkd6cjfw9iPzqzYyeVON3TofpXS2LkNsJ5q9RRRRXJ6pL5lxc5YE+YfyBwP0ArPjGZFHqcV0tmu2MAcVfXPpUop2KMVTvbFblTjAb19a5q9sZLZzlTiqojLAlT+FMwR14qeMjcpPQnaa6GzkOYWY84CsfeteiiigsFBZjhQMn6VwjOS5LHJJ596faLvuUH+1munhARRuIHHepxPGvU8etSrLG33XU\/jUgIozQSB1qGeOOWMiTBHvWHd6ZEHDW8oJ\/u9aoXNrIULGMZXupBz+FVk+bOe4xW5buNkZH9\/+lbyHcin1GaWiiqmqvs02YjqRt646nB\/TNc1bbZYlikjTaykhh97PrS6TDuuhkZx3rbldVbNQPqEKjDLwe\/GKrm8gLjymOT6c1es70MQN4YHjOa0QcjNVbq48sEZIrLluufmZiO\/oKkS7t1A2tGx9N4Jp0kkUyn5cH3rBK+XOyehrSs5D5PJ+64x\/n8a6S2ObdD+FSUUVleIpNliq92bP4AY\/qKyLIh7VWYf6oso\/n\/WrulKPMdgMVbuLcyLkHFZRsAryCVS4cYDd0P09KWytJWkQXR3xRZKIPUgD8uBV1LLa5YE8njJ5\/GteIfuhnriqN1B5suCcVny6ajI6Orb8fI6nhT9KrwWUmSbpDIyrsRQRjHvVqxsHiXDHj09PpWfqkPkXoYfddf8\/wBKS0fKFPcmum0191sOtW6KKwPEjfOAG42gY9Dkn\/CqGksGEsDNjOCMfr\/StWzQxzsp6Y4PrWgi560jwA9qRLdR\/Din+WN3pipV4FMIycGkaEMPf1pv2cZzTim1aw9fUCKI8Z3f0rLtmxMue5xXS6O2YyvYfpWlRRXNeIMieTPQuuP++RWZBKILhJgDgYyPYjmtyOVftCMrgox+Ug1pI3zEVOKOKhdizEL0FTryKic4Py8kU+Nw65H5U84qKVsLXM63NvuUQdFBP+fyrPjbDKfQ1v6HP+\/Kn+IYrdoormvEIYXEmTwWUge20D+hrIbBQHnpSQuI50cnhWDH866xXHDVZR+KefmFUrqK5UN5LYz34P8AOoo7qZFCMMseDgU6NLsz7t\/ynqvGP8auopU5FK0gFVrmX5OtctfPvu5DnODiolxx9eav6dKUmUg4ZTkV1\/06UUVheJYD+7n5KkbD7EZI\/mfyrECE22fQ1B3+tb+n3BmtFLHJHB+taERPY1OJMUsk0aLl3Cj3NQLc2rniZOPwNOW8td2BMuffipC4YZBHsRzUcp9qzruQJGzE8KM1zpyTk96VTgirFucOrKeQa7K1fzLaNv8AZAqWiqOsxebpsnGSpDD8\/wDAmuf8sJZZPXNZ7DDH2NXNLmMczRk\/Kwz+NbkMgp8mX6MRVOaxDPvd3l9mNILW0PVJAfZjTjZW7jCRke7E5qxBbJbgCMtj03E0+aQBcViapOCBEp5PLc1nAUuMsKmhjZjlBnAJP0rsLBAlomGYhhnB6A98VYopCoZSrDIIwR6iuY1IqEEaDheCfesthlvriiBtkqkVsxyY4P51ehIdeDzU6wbhjIpDaU77OEFRyMIxz0rOurjapxyRWEWLvubkk07PJPahOlaujwvI7BSRvXacdwTz\/KunVQqhR0AxS0VVvbny4yiNh8cn0Fc5cNuZ\/wAxVNuinHtUR4J4rWjO6FWHcU6O4aE5PStGG\/jYc4BqcXaEdR+dRyXsYH3h+dZ01y9w2EBwe9RTx7Ldjg5IrHFKTxinqcL0roPD0saowY4ZsAH2\/wD11uUVSluNiZuDtbqEFZc1wW3Ek464P86zLhsYPXrmol+YkZ75qNgQK0LFt0O09jUrrwRUDx+nWmbZ+in9amitiSC\/NXYogMYFRXh\/dMB6Vilcc0hUg0biSB71PFcyxbWRyGAwO\/HpWlba3PGAsmNvqBWylzdFQTACDyCOhFc+rTE\/PIF\/U1FNLh\/LU8dz6mqsr73PoKYr4fPapHO5c96sWD4Zl9cVodRQFz1FSLGO9O2gcDrTiMDNV5+VxWXImHK4xyaktYRNh3OAKpsQZCRwuePpRnmjceKmS5mVQFlcAdgxpTOcHb1PFRMdoIz8x6+1R0Ac08E7amtztkB961I24x7VIPWlyacDSkk1EVz2qpcQ4IfHQ5NVJXaEPEpOCc1W70HikFLnFO3qg+UZPqaYSTQKcn3vpQO9WrdMtnHpWgg9u1SgDPFLTwDT1jPcU9IcmmXUSxxsz8ADmudwJd7EkNxtGM57UxgVfaeoODTDnNLRSUUUv0\/GnICxAHU1s2lt+7yO5\/Srn2c4+gpVhyead5HNOEJB6VOkJ9KlVAo5FYviC52hYEbljlsHtWEGZTkEg+oNJ0I+tB60UZpKAcHNLmgDvSgkdDXT6ZIlxbLIowR8rD0NXwgpQnOetL5YpQgBp9RTyrFEzsQABkk1x13cG4uHlOfmPGew7VAKO4oopRU\/9nX3\/Pncf9+m\/wAKT+zr7\/nzuP8Av03+FKNPvf8AnzuP+\/Tf4Uv9n33\/AD53H\/fpv8KBp99\/z53H\/fpv8K0tFivLa4MclrOI5O5jOARW+EbH3G\/KnBG\/ut+VO2tj7p\/Kgq390\/lTCH5+RvyrJ1oXTwCKCCZt5+YqhOBWSmj3siO3kOpVN+GQjPsPf2qH+zr3\/nzuP+\/bf4Un9nXuf+PO4\/79N\/hR\/Z19\/wA+dx\/36b\/Cj+z77\/nzuP8Av03+FL\/Z97\/z53H\/AH6b\/Cv\/2Q==",
            "IdSigFile": "MGQCMFhhdXOMTsxp45AC+J+9LJSQL5zqksbZVGcLx+4nlRrxy7GqwRaQQj80aOgCbtipbQIwD78KKYa6TE51xdKIvvhLon6re81iTgTo3Bi7NQgGj8f+2oqQmbUs2blg9BzjLhP8",
            "AddrSigFile": "MGUCMQCR7I2YSIeWS8LYKFxU+9ZOx7U1zA4b3ITEWRYA\/351hSn209TTtzhmpDSsRJp+Y2ACMCIM91zic8X5UoehjcAvSzBrKKFeSRt+898d8Ualfx16nxNK\/+eD0XhusjrPgB2AwA=="
        },
        {
            "Description": "Applet 1.7 ReKeyed TEST eID Active/Active",
            "SignCert": "MIIEGTCCA5+gAwIBAgIQEAAAAAAA8evx/gAAAAHn6jAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMjA3MDQyMjAwMDBaFw0zMjA3MDQyMjAwMDBaMHExCzAJBgNVBAYTAkJFMSIwIAYDVQQDDBlOb3JhIFNwZWNpbWVuIChTaWduYXR1cmUpMREwDwYDVQQEDAhTcGVjaW1lbjEVMBMGA1UEKgwMTm9yYSBBbmfDqGxlMRQwEgYDVQQFEwswMTA1MDM5OTk2MzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMz7/VtHKJmPdBIvg0LuzzlFWWn0QpVSWbUumVZ/Bl1A/9gNYQiKeiTog3BGgZGEc12HGxsxYjoL5sJ1oSzEx+cV7DfnA/7rcta5gxxSgpi08s4qESYmKbQXHiIe1DkpY53IuBYSMy+ynNqlAwltBzda2iaKLvQrZ/6+Qt+N5lh1yyUGpo7dtvivch62XeqXiMOIqYG1nASJj+R3B30iH1LBZ1Y76ryadSfdo1j78UDZ/We4VS7qFKc/g0/zwojCKT6SfZTFG0OjtfENHNjzhUtUVLwlgejzLHb3+fGegiTVAw2jJL5tZlQxN5LECl5L2s0WkT6eDM2RszQ950pYUhECAwEAAaOCAY8wggGLMB8GA1UdIwQYMBaAFN2zf+OaGY5ZyRFWAi31+p1v3oRLMA4GA1UdDwEB/wQEAwIGQDBpBgNVHSAEYjBgMF4GB2A4DAEBAgEwUzAxBggrBgEFBQcCARYlaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydDAeBggrBgEFBQcCAjASGhBURVNUIHVzZXIgbm90aWNlMCIGCCsGAQUFBwEDBBYwFDAIBgYEAI5GAQEwCAYGBACORgEEMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIxAI53Ey3cefE7jQuuF++Lxg0srVcyC2GAqxYVhPz0Cvj1vvvSuKhgChSttNr49ejLOAIwfdYwtjs0zfKjh5+t9vms065G0XX4hE0Z8c8dweNJSSNoktEe6nI43E8lu6ZKN2pd",
            "AuthCert": "MIID7zCCA3WgAwIBAgIQEAAAAAAA8evx/gAAAAHn5TAKBggqhkjOPQQDAzAuMQswCQYDVQQGEwJCRTEfMB0GA1UEAwwWZUlEIFRFU1QgRUMgQ2l0aXplbiBDQTAeFw0yMjA3MDQyMjAwMDBaFw0zMjA3MDQyMjAwMDBaMHYxCzAJBgNVBAYTAkJFMScwJQYDVQQDDB5Ob3JhIFNwZWNpbWVuIChBdXRoZW50aWNhdGlvbikxETAPBgNVBAQMCFNwZWNpbWVuMRUwEwYDVQQqDAxOb3JhIEFuZ8OobGUxFDASBgNVBAUTCzAxMDUwMzk5OTYzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzbUJbVlUtpUGjucOZa4sD49MdATfCATpE3TrUcTDPbLEhvSFk0KP0tZCom7RDpi6LvSna68PGEr55BPUqLEZ5DALhf4X+FuJbx7mz1GyoTC4M9FFcZjDY3XOiRcYmJ57U4R9EPX/jgqVagEINHzdceRgs5gUYjRlMzLLMVSQYl5zcLQXuW+G72ud6MmpvVWFmhkuO0oAIEGfDkkWduteQFU2ERqgyeVLvNFRYgKLPgpSRuGxZzOlzjyPJc42KX5vTDCkcpw8nYgOxqC9yvhszPkgQwYq4f0OpWOux98aW0WvXLpxMe3cmYWLSKnVxukuycEZWwfZQYHGDCHDW5OSdQIDAQABo4IBYDCCAVwwHwYDVR0jBBgwFoAU3bN/45oZjlnJEVYCLfX6nW/ehEswDgYDVR0PAQH/BAQDAgeAMEkGA1UdIARCMEAwPgYHYDgMAQECAjAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MBMGA1UdJQQMMAoGCCsGAQUFBwMCMEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9jaXRpemVuY2FFQy5jcmwwgYEGCCsGAQUFBwEBBHUwczA+BggrBgEFBQcwAoYyaHR0cDovL2VpZGRldmNhcmRzLnpldGVzY2FyZHMuYmUvY2VydC9yb290Y2FFQy5jcnQwMQYIKwYBBQUHMAGGJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlOjg4ODgwCgYIKoZIzj0EAwMDaAAwZQIwBVF/ArIduCYGU4r6EYFvNkLjVrthHCmqKy3a4mrlZoh+94TsJ5vuMtKe7aixX+CWAjEA4EDryTIlkX3o4Wv5w0SdIQmRdjO1NUTLdN+ae4Bwxslycj0YXc4CGwb7iGnwXdw+",
            "CACert": "MIIDKTCCAq+gAwIBAgIIcND8I1qptLUwCgYIKoZIzj0EAwMwKzELMAkGA1UEBhMCQkUxHDAaBgNVBAMME2VJRCBURVNUIEVDIFJvb3QgQ0EwIBcNMDcwNDMwMjIwMDIwWhgPMjA4NzA0MTAyMjAwMjBaMC4xCzAJBgNVBAYTAkJFMR8wHQYDVQQDDBZlSUQgVEVTVCBFQyBDaXRpemVuIENBMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEJAiNoOQfY0r8N6JVPMLedXyRZ7MwppGwQ9ZxFzLjVsbeKuUvqEFR0yKKyEidXc875m4UF5lRpf/FSWagg2IXGWrypnRZkgnNVP6s5W2LzKdV09hd6v7O8j/8knfHOj+No4IBmTCCAZUwHQYDVR0OBBYEFN2zf+OaGY5ZyRFWAi31+p1v3oRLMB8GA1UdIwQYMBaAFCHAclfKHAQEGR3ZjH4+tYPrrBwCMA4GA1UdDwEB/wQEAwIBBjBIBgNVHSAEQTA/MD0GBmA4DAEBAjAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDBDBCBgNVHR8EOzA5MDegNaAzhjFodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jcmwvcm9vdGNhRUMuY3JsMIGBBggrBgEFBQcBAQR1MHMwPgYIKwYBBQUHMAKGMmh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQvcm9vdGNhRUMuY3J0MDEGCCsGAQUFBzABhiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZTo4ODg4MBIGA1UdEwEB/wQIMAYBAf8CAQAwCgYIKoZIzj0EAwMDaAAwZQIxAOMiiByF0aLEA6zUrobMw7aSH5o2u1hGVMe0AL4ezYztRdfxvXVU+m1JosBVBDDjeAIwYJJN7bLWw8BVi/lkxRjKL/+zAJP6djGywXI1pVh4HKb0D+tipq5StO+QnM8cnPmg",
            "ROOTCert": "MIICNDCCAbugAwIBAgIBATAKBggqhkjOPQQDAzArMQswCQYDVQQGEwJCRTEcMBoGA1UEAwwTZUlEIFRFU1QgRUMgUm9vdCBDQTAgFw0wNzA0MzAyMjAwMTBaGA8yMDg4MDQwOTIyMDAxMFowKzELMAkGA1UEBhMCQkUxHDAaBgNVBAMME2VJRCBURVNUIEVDIFJvb3QgQ0EwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAASYqtYIKayPGXFNDaGkPdCadQCSC8D2W8aKE7xh850ykG0bJXMV7IaKZWo0ZXUb55g9S95gjDNeZ0iNo75dY/mWoozI6I2l106OdPL+yAcHI6id4uR7Fd0nQxeBICdmjnCjgbAwga0wHQYDVR0OBBYEFCHAclfKHAQEGR3ZjH4+tYPrrBwCMB8GA1UdIwQYMBaAFCHAclfKHAQEGR3ZjH4+tYPrrBwCMA4GA1UdDwEB/wQEAwIBBjBHBgNVHSAEQDA+MDwGBWA4DAEBMDMwMQYIKwYBBQUHAgEWJWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NlcnQwEgYDVR0TAQH/BAgwBgEB/wIBATAKBggqhkjOPQQDAwNnADBkAjBM2P48H8f2FY0NHm1uAdgXwYoBRkUFOq8Kccd7l6Y8RavzAkMQmLgVF3s5euuv6fcCMCW4UGWpnOTOA+t4V9/+kPMjGqgC9Uw4nOKkwkwQs3IeWfc7Na6l+U8r4M7VH49/cw==",
            "RRNCert": "MIICgzCCAgmgAwIBAgIIA1F0wfUKrp4wCgYIKoZIzj0EAwMwKzELMAkGA1UEBhMCQkUxHDAaBgNVBAMME2VJRCBURVNUIEVDIFJvb3QgQ0EwIBcNMDcwNDMwMjIwMDIwWhgPMjA1NzA0MTcyMjAwMTBaMDYxCzAJBgNVBAYTAkJFMQ0wCwYDVQQKDARURVNUMRgwFgYDVQQDDA9lSUQgVEVTVCBFQyBSUk4wdjAQBgcqhkjOPQIBBgUrgQQAIgNiAARlTeyTcHbrwvN5elieA+y302LhHoXKHEmIVN+H/cHUNPLQrRwmY5TQerzJ/f+swsKGjM3p3n4oebT/bWo+AICDtPLBJcRO0Q2JaixV1lLXeGoSziwguT8L/oa6B36ytEOjgewwgekwHQYDVR0OBBYEFLV/8yWBmfffDwVYb3r3+DiB/9i4MB8GA1UdIwQYMBaAFCHAclfKHAQEGR3ZjH4+tYPrrBwCMA4GA1UdDwEB/wQEAwIGwDBIBgNVHSAEQTA/MD0GBmA4DAEBBDAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MEIGA1UdHwQ7MDkwN6A1oDOGMWh0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9yb290Y2FFQy5jcmwwCQYDVR0TBAIwADAKBggqhkjOPQQDAwNoADBlAjEA2ONS6D4ktQtjPZOTczWFHYDvHT1BfT+bAoNfPTCNB01QETa0A6S1PiV8m1/PQOGmAjB56VlGloc2Jz0fGbnXRH0TcQIOTUEqWDmrypoPC1PiKP8Som4j1wWqN2Ajx8zMzc8=",
            "DoPINOps": true,
            "PIN": "1234",
            "IdFile": "AQwwMDAwMDEyNDkwNzQCEFNMR5ACUQAAIaAojxKSA2MDCjA1LjA3LjIwMjIECjA1LjA3LjIwMzIFB0JydXNzZWwGCzAxMDUwMzk5OTYzBwhTcGVjaW1lbggMTm9yYSBBbmfDqGxlCQAKBEJFTEcLBkxldXZlbgwLMDMgTUVJIDIwMDENAVYOAA8BMRABMBEgR2osQHL1+wdkLiVtXt\/XnGpnX8aSYtMn+ZeQUVF7CAE=",
            "AddrFile": "ARFTcGVjaW1lbnN0cmFhdCAxMgIEMTAwMAMHQnJ1c3NlbAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==",
            "Photo": "\/9j\/4AAQSkZJRgABAgEBLAEsAAD\/2wBDABwTFRgVERwYFhgfHRwhKUUtKSYmKVQ8QDJFZFhpZ2JYYF9ufJ6GbnWWd19giruLlqOpsbOxa4TC0MGszp6usar\/wAALCADIAIwBAREA\/8QA0gAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoLEAACAQMDAgQDBQUEBAAAAX0BAgMABBEFEiExQQYTUWEHInEUMoGRoQgjQrHBFVLR8CQzYnKCCQoWFxgZGiUmJygpKjQ1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4eLj5OXm5+jp6vHy8\/T19vf4+fr\/2gAIAQEAAD8A38n1NGT6mjJ9TRk+poyfU0ZPqaMn1NIWwMlsfjUf2qEdZ0\/77FPWVX+5IG+hzTsn1NGT6mjJ9TRk+poyfU0ZPqaMn1NGT6mkooooqnNqMUb7V+bB5OeKq3OpO4\/c\/KuMk55rKkvS5yWZiO7ZJqMXMnI2gj0K\/wD1qljncn5QjHsAf8mr0OoyIcHeh7g\/MP1rUgvI5sDoT6cj\/wCt+NWKKKKKKKKKRmVFLMQFUZJPQCub1DVpLlikBKxZ4GOvuaz1+Zz5shwO3rQ01tnHl5wOuTR58GP9WPypN8LdE2+4bH86nXdgeW4ZT2k\/xFTJN5ZCSAxk9A+Cv4GrA4PIHHY1qafPvXyyeFHyg9R7VdooooooorG1y5aRls4Wx\/FKR2HYf1\/Ksn5EUqmPc1Un25+U8\/Wog397mkx6Uqt2ZcipUTPMLEN6ZwfzqZbqRBsmQOvdWFWIZAiZt23xjqh6r9P8KuQyK2HU7cHhhwVNbNrcefH82BIPvKP51PRRRRRTJpBDC8rDIRS2B3x2rmLqRjudjhnbc5\/oKzZJS3AOBQqFzjGanSydlyeKY1uyn+tIIdynoGHamFGU9CMVNHKrDbOCR\/eHUUpV7ciSMhvcdGHoRVmGZZCHj+QnhlJ6\/wD1\/T1rTtZTGyyJk7eCvt6VsAggEHIPINLRRRRVDV5glusZBw53E46BcH+eP1rlrmUu3PrTYITK4ArctLAKASK0Bars6dqqXNjjkCsyaLZ7HsfSq4Kjg\/TjsailXutNjlK8dQ3UUp+Rt6\/dYYatewnLJycuvDD+Vblocxbc5A6fSp6KKKKw9flxJs3ZAQAj0PJP\/stc8Ms1bWl24ChiOvtW0i9OlTDpQQCOap3dksyEqMN\/OucvLd4ZCdpFVQ56HpTXAzkd6cjfw9iPzqzYyeVON3TofpXS2LkNsJ5q9RRRRXJ6pL5lxc5YE+YfyBwP0ArPjGZFHqcV0tmu2MAcVfXPpUop2KMVTvbFblTjAb19a5q9sZLZzlTiqojLAlT+FMwR14qeMjcpPQnaa6GzkOYWY84CsfeteiiigsFBZjhQMn6VwjOS5LHJJ596faLvuUH+1munhARRuIHHepxPGvU8etSrLG33XU\/jUgIozQSB1qGeOOWMiTBHvWHd6ZEHDW8oJ\/u9aoXNrIULGMZXupBz+FVk+bOe4xW5buNkZH9\/+lbyHcin1GaWiiqmqvs02YjqRt646nB\/TNc1bbZYlikjTaykhh97PrS6TDuuhkZx3rbldVbNQPqEKjDLwe\/GKrm8gLjymOT6c1es70MQN4YHjOa0QcjNVbq48sEZIrLluufmZiO\/oKkS7t1A2tGx9N4Jp0kkUyn5cH3rBK+XOyehrSs5D5PJ+64x\/n8a6S2ObdD+FSUUVleIpNliq92bP4AY\/qKyLIh7VWYf6oso\/n\/WrulKPMdgMVbuLcyLkHFZRsAryCVS4cYDd0P09KWytJWkQXR3xRZKIPUgD8uBV1LLa5YE8njJ5\/GteIfuhnriqN1B5suCcVny6ajI6Orb8fI6nhT9KrwWUmSbpDIyrsRQRjHvVqxsHiXDHj09PpWfqkPkXoYfddf8\/wBKS0fKFPcmum0191sOtW6KKwPEjfOAG42gY9Dkn\/CqGksGEsDNjOCMfr\/StWzQxzsp6Y4PrWgi560jwA9qRLdR\/Din+WN3pipV4FMIycGkaEMPf1pv2cZzTim1aw9fUCKI8Z3f0rLtmxMue5xXS6O2YyvYfpWlRRXNeIMieTPQuuP++RWZBKILhJgDgYyPYjmtyOVftCMrgox+Ug1pI3zEVOKOKhdizEL0FTryKic4Py8kU+Nw65H5U84qKVsLXM63NvuUQdFBP+fyrPjbDKfQ1v6HP+\/Kn+IYrdoormvEIYXEmTwWUge20D+hrIbBQHnpSQuI50cnhWDH866xXHDVZR+KefmFUrqK5UN5LYz34P8AOoo7qZFCMMseDgU6NLsz7t\/ynqvGP8auopU5FK0gFVrmX5OtctfPvu5DnODiolxx9eav6dKUmUg4ZTkV1\/06UUVheJYD+7n5KkbD7EZI\/mfyrECE22fQ1B3+tb+n3BmtFLHJHB+taERPY1OJMUsk0aLl3Cj3NQLc2rniZOPwNOW8td2BMuffipC4YZBHsRzUcp9qzruQJGzE8KM1zpyTk96VTgirFucOrKeQa7K1fzLaNv8AZAqWiqOsxebpsnGSpDD8\/wDAmuf8sJZZPXNZ7DDH2NXNLmMczRk\/Kwz+NbkMgp8mX6MRVOaxDPvd3l9mNILW0PVJAfZjTjZW7jCRke7E5qxBbJbgCMtj03E0+aQBcViapOCBEp5PLc1nAUuMsKmhjZjlBnAJP0rsLBAlomGYhhnB6A98VYopCoZSrDIIwR6iuY1IqEEaDheCfesthlvriiBtkqkVsxyY4P51ehIdeDzU6wbhjIpDaU77OEFRyMIxz0rOurjapxyRWEWLvubkk07PJPahOlaujwvI7BSRvXacdwTz\/KunVQqhR0AxS0VVvbny4yiNh8cn0Fc5cNuZ\/wAxVNuinHtUR4J4rWjO6FWHcU6O4aE5PStGG\/jYc4BqcXaEdR+dRyXsYH3h+dZ01y9w2EBwe9RTx7Ldjg5IrHFKTxinqcL0roPD0saowY4ZsAH2\/wD11uUVSluNiZuDtbqEFZc1wW3Ek464P86zLhsYPXrmol+YkZ75qNgQK0LFt0O09jUrrwRUDx+nWmbZ+in9amitiSC\/NXYogMYFRXh\/dMB6Vilcc0hUg0biSB71PFcyxbWRyGAwO\/HpWlba3PGAsmNvqBWylzdFQTACDyCOhFc+rTE\/PIF\/U1FNLh\/LU8dz6mqsr73PoKYr4fPapHO5c96sWD4Zl9cVodRQFz1FSLGO9O2gcDrTiMDNV5+VxWXImHK4xyaktYRNh3OAKpsQZCRwuePpRnmjceKmS5mVQFlcAdgxpTOcHb1PFRMdoIz8x6+1R0Ac08E7amtztkB961I24x7VIPWlyacDSkk1EVz2qpcQ4IfHQ5NVJXaEPEpOCc1W70HikFLnFO3qg+UZPqaYSTQKcn3vpQO9WrdMtnHpWgg9u1SgDPFLTwDT1jPcU9IcmmXUSxxsz8ADmudwJd7EkNxtGM57UxgVfaeoODTDnNLRSUUUv0\/GnICxAHU1s2lt+7yO5\/Srn2c4+gpVhyead5HNOEJB6VOkJ9KlVAo5FYviC52hYEbljlsHtWEGZTkEg+oNJ0I+tB60UZpKAcHNLmgDvSgkdDXT6ZIlxbLIowR8rD0NXwgpQnOetL5YpQgBp9RTyrFEzsQABkk1x13cG4uHlOfmPGew7VAKO4oopRU\/9nX3\/Pncf9+m\/wAKT+zr7\/nzuP8Av03+FKNPvf8AnzuP+\/Tf4Uv9n33\/AD53H\/fpv8KBp99\/z53H\/fpv8K0tFivLa4MclrOI5O5jOARW+EbH3G\/KnBG\/ut+VO2tj7p\/Kgq390\/lTCH5+RvyrJ1oXTwCKCCZt5+YqhOBWSmj3siO3kOpVN+GQjPsPf2qH+zr3\/nzuP+\/bf4Un9nXuf+PO4\/79N\/hR\/Z19\/wA+dx\/36b\/Cj+z77\/nzuP8Av03+FL\/Z97\/z53H\/AH6b\/Cv\/2Q==",
            "IdSigFile": "MGUCMGqW4St63Y5ACZw\/ni3cxzH7TVoOhpd72CGVX57aFmV1hk13dhtEZ5VjD5hcq5zhsAIxAORfMtkpEVibI4NUq94RyyRMatvJfS1eiIf4UxgXbu1UgQwOyoYzgYTcTOnIDuByhAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==",
            "AddrSigFile": "MGYCMQDMCaZiL78EdRo8aMNYmXZwAt8VJQSzui1h8l5e7oLgwvdnvNTqVSqrfZhaSf12meMCMQDXiGjBpJKT4PxcnTs4wmUTRK\/q\/di++Ln2+tjeZcJXTm4CQ+HlPIFX5tiWdoM4JFsAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==",
        }
    ]
}
*/

struct TestCard
{
    std::string Description;
    std::string SignCert;
    std::string AuthCert;
    std::string CACert;
    std::string ROOTCert;
    std::string RRNCert;
    bool DoPINOps = false;
    std::string PIN;
    std::string IdFile;
    std::string AddrFile;
    std::string Photo;
    std::string IdSigFile;
    std::string AddrSigFile;
};
class TestDB
{
    std::vector<TestCard> testcards;
    void LoadTestDBFile()
    {
        try
        {
            std::ifstream t(TEST_FILE);
            std::stringstream buffer;
            buffer << t.rdbuf();

            boost::property_tree::ptree pt;
            boost::property_tree::read_json(buffer, pt);

            auto it = pt.get_child("cards");

            for (auto it2 = it.begin(); it2 != it.end(); ++it2)
            {
                TestCard tc;
                tc.Description = it2->second.get<std::string>("Description");
                tc.SignCert = it2->second.get<std::string>("SignCert");
                tc.AuthCert = it2->second.get<std::string>("AuthCert");
                tc.CACert = it2->second.get<std::string>("CACert");
                tc.ROOTCert = it2->second.get<std::string>("ROOTCert");
                tc.RRNCert = it2->second.get<std::string>("RRNCert");
                tc.DoPINOps = it2->second.get<bool>("DoPINOps");
                tc.PIN = it2->second.get<std::string>("PIN");
                tc.IdFile = it2->second.get<std::string>("IdFile");
                tc.AddrFile = it2->second.get<std::string>("AddrFile");
                tc.Photo = it2->second.get<std::string>("Photo");
                tc.IdSigFile = it2->second.get<std::string>("IdSigFile");
                tc.AddrSigFile = it2->second.get<std::string>("AddrSigFile");
                testcards.push_back(tc);
            }
        }
        catch (boost::property_tree::json_parser_error& e)
        {
            std::cout << "Smart card test DB file parsing error " << e.what() << endl;
        }
        catch (...)
        {
            std::cout << "No smart card test DB file found." << endl;
        }
    }
public:
    TestDB()
    {
        LoadTestDBFile();
    }
    const TestCard GetTestCard(const std::string SignCert)
    {
        for (auto it = testcards.begin(); it != testcards.end(); ++it)
        {
            if (it->SignCert == SignCert)
            {
                return *it;
            }
        }
        return TestCard();
    }
};


std::string PrepareCmd(const std::string& cmd, const std::string& crt, bool isPinPadReader, const std::string& pin)
{
    std::string preparedCmd = cmd;
    // Inject the sign cert
    size_t pos = preparedCmd.find("%%CERT%%");
    if (pos != std::string::npos) {
        preparedCmd.replace(pos, 8, crt);
    }
    // Inject the pin if reader has no pinpad
    if (!isPinPadReader)
    {
        size_t pos = preparedCmd.find("\"pin\":\"null\"");
        if (pos != std::string::npos) {
            preparedCmd.replace(pos + 7, 4, pin);
        }
    }
    return preparedCmd;
}
std::string GetSignatureFromResponse(const std::string& result)
{
    boost::property_tree::ptree pt;
    std::stringstream ss(result);
    boost::property_tree::read_json(ss, pt);
    try
    {
        return pt.get<std::string>(BeidConnect_JSON_field::signature);
    }
    catch (...)
    {

    }
    return "";
}
std::string GetResultFromFileCompare(const std::shared_ptr<const CardFile>& File, const std::string& ExpectedFileContent)
{
    if (ExpectedFileContent.length() == 0 && File->getBase64().length() != 0)
    {
        return "OK but no reference file for comparison";
    }
    else if (ExpectedFileContent == File->getBase64())
    {
        return "OK";
    }
    return "Fail retrieved file empty";
}
std::string GetResultFromResponse(const std::string& result, const std::string& ExpectedReader)
{
    boost::property_tree::ptree pt;
    std::stringstream ss(result);
    boost::property_tree::read_json(ss, pt);
    try
    {
        std::string ReaderName;
        if (pt.get_optional<std::string>(BeidConnect_JSON_field::ReaderName).is_initialized())
        {
            ReaderName = pt.get<std::string>(BeidConnect_JSON_field::ReaderName);
        }
        std::string res = pt.get<std::string>(BeidConnect_JSON_field::result);

        if ((ReaderName == ExpectedReader || ExpectedReader == "All") && res == BeidConnect_Result::OK)
        {
            return "OK";
        }
        else if (ReaderName != ExpectedReader)
        {
            return "Fail (" + res + ") operation complete on reader " + ReaderName + " but expected on reader " + ExpectedReader;
        }
        return "Fail :" + res;
    }
    catch (...)
    {

    }
    return "Failed parsing result";
}

std::string ExpectResultInResponse(const std::string& result, const std::string& ExpectResult)
{
    boost::property_tree::ptree pt;
    std::stringstream ss(result);
    boost::property_tree::read_json(ss, pt);
    try
    {
        std::string res = pt.get<std::string>(BeidConnect_JSON_field::result);

        if (res == ExpectResult)
        {
            return "OK";
        }
        return "Fail : Expect result " + ExpectResult + " but receive " + res;
    }
    catch (...)
    {

    }
    return "Failed parsing result";
}

class TestResult
{
public:
    TestResult(const std::string& Test) : Test(Test) {}
    TestResult(const std::string& Test, const std::string& Result) :Test(Test), Result(Result) {}
    TestResult(const std::string& Test, const std::string& Result, long long Duration) :Test(Test), Result(Result), Duration(Duration) {}
    std::string Test;
    std::string Result;
    long long Duration = 0;
    std::string cmd;
    std::string cmdResponse;
};

TestResult TestSign(const std::string& DisplayMessage, const std::string& cmdTemplate, const std::string& crt, bool isPinPadReader, const std::string& pin, const std::function<std::string(const std::string& Response)>& f)
{
    TestResult tr(DisplayMessage);
    std::cout << endl << TestHeader << DisplayMessage << endl;
    tr.cmd = PrepareCmd(cmdTemplate, crt, isPinPadReader, pin);
    std::cout << tr.cmd << endl;

    auto startTime = std::chrono::steady_clock::now();
    shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>(tr.cmd);
    shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);
    handler->AddTraceInfoInJsonResult();
    string Response = handler->process();
    tr.Duration = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count();
    std::cout << Response << endl;
    tr.Result = f(Response);
    return tr;
}
TestResult TestOp(const std::string& DisplayMessage, const std::string& cmdTemplate, const std::function<std::string(const std::string& Response)>& f)
{
    TestResult tr(DisplayMessage);
    std::cout << endl << TestHeader << DisplayMessage << endl;
    tr.cmd = cmdTemplate;
    std::cout << tr.cmd << endl;

    auto startTime = std::chrono::steady_clock::now();
    shared_ptr<stringstream> ssRequest = std::make_shared<stringstream>(tr.cmd);
    shared_ptr<RequestHandler> handler = RequestHandler::createRequestHandler(ssRequest);
    handler->AddTraceInfoInJsonResult();
    string Response = handler->process();
    tr.Duration = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count();
    std::cout << Response << endl;
    tr.Result = f(Response);
    return tr;
}
TestResult TestGetFile(const std::string& DisplayMessage, const std::shared_ptr<Card>& card, CardFiles fileType, const std::function<std::string(const std::shared_ptr<const CardFile>& File)>& ResponseFile)
{
    TestResult tr(DisplayMessage);
    std::cout << endl << TestHeader << DisplayMessage << endl;
    auto startTime = std::chrono::steady_clock::now();
    std::shared_ptr<const CardFile> file = card->getFile(fileType);
    tr.Duration = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count();
    tr.Result = ResponseFile(file);
    return tr;
}


int runTest(int argc, const char* argv[])
{
    std::cout << "BeidConnect Version " << BEIDCONNECT_VERSION << " Build on " << __DATE__ << " " << __TIME__ << endl;

    TestDB testDB;
    shared_ptr<stringstream> ssRequest;
    shared_ptr<RequestHandler> handler;
    std::list< TestResult> testResults;
    string ssResponse;

    ReaderList readerList;
    for (auto& cr : readerList.readers)
    {
        std::cout << "Reader " << cr->name << endl;

        std::shared_ptr<Card> card = CardFactory::createCard(cr);
        if (card != nullptr)
        {
            std::cout << "BEID card detected " << endl;
            cr->connect();
            const std::map<std::string, std::string> cardData = card->getCardData();
            auto it = cardData.find("AppletVersion");
            if ((it != cardData.end()) && (it->second == "1.7"))
            {
                std::cout << "Applet version 1.7" << endl;
            }
            else if ((it != cardData.end()) && (it->second == "1.8"))
            {
                std::cout << "Applet version 1.8" << endl;
            }
            else
            {
                std::cout << "Applet version not handled" << endl;
            }

            TestCard tc;
            std::shared_ptr<const CardFile> signCert;

            // Test the getFile function to retrieve the sign cert
            testResults.push_back(TestGetFile("Retrieve Sign certificate (getFile)", card, CardFiles::Signcert,
                [&signCert, &testDB, &tc](const std::shared_ptr<const CardFile>& File) {
                    signCert = File;
                    dumpCert(File);
                    tc = testDB.GetTestCard(File->getBase64());
                    return GetResultFromFileCompare(File, tc.SignCert);
                }));
            // Test the getFile function to retrieve the Auth cert
            testResults.push_back(TestGetFile("Retrieve Auth certificate (getFile)", card, CardFiles::Authcert,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    dumpCert(File);
                    return GetResultFromFileCompare(File, tc.AuthCert);
                }));
            // Test the getFile function to retrieve the CA cert
            testResults.push_back(TestGetFile("Retrieve CA certificate (getFile)", card, CardFiles::Cacert,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    dumpCert(File);
                    return GetResultFromFileCompare(File, tc.CACert);
                }));
            // Test the getFile function to retrieve the ROOT cert
            testResults.push_back(TestGetFile("Retrieve ROOT certificate (getFile)", card, CardFiles::Rootcert,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    dumpCert(File);
                    return GetResultFromFileCompare(File, tc.ROOTCert);
                }));
            // Test the getFile function to retrieve the RRN cert
            testResults.push_back(TestGetFile("Retrieve RRN certificate (getFile)", card, CardFiles::Rrncert,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    dumpCert(File);
                    return GetResultFromFileCompare(File, tc.RRNCert);
                }));
            // Test the getFile function to retrieve the ID File
            testResults.push_back(TestGetFile("Retrieve ID File (getFile)", card, CardFiles::Id,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    return GetResultFromFileCompare(File, tc.IdFile);
                }));
            // Test the getFile function to retrieve the Addr File
            testResults.push_back(TestGetFile("Retrieve Addr File (getFile)", card, CardFiles::Address,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    return GetResultFromFileCompare(File, tc.AddrFile);
                }));
            // Test the getFile function to retrieve the Photo File
            testResults.push_back(TestGetFile("Retrieve Photo File (getFile)", card, CardFiles::Photo,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    return GetResultFromFileCompare(File, tc.Photo);
                }));
            // Test the getFile function to retrieve the ID File Signature
            testResults.push_back(TestGetFile("Retrieve ID File Signature (getFile)", card, CardFiles::Id_sig,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    return GetResultFromFileCompare(File, tc.IdSigFile);
                }));
            // Test the getFile function to retrieve the Addr File Signature
            testResults.push_back(TestGetFile("Retrieve Addr File Signature (getFile)", card, CardFiles::Address_sig,
                [&tc](const std::shared_ptr<const CardFile>& File) {
                    return GetResultFromFileCompare(File, tc.AddrSigFile);
                }));
            bool isPinPadReader = cr->isPinPad();
            cr->disconnect();
            do_sleep(500);
            // Test the browser extension message to retrieve the card information (idflags set to 1 to retrieve only the ID file)
            testResults.push_back(TestOp("Get Card Info (ID only) Browser Extension command", message_Get_Card_Info_ID_Only,
                [](const std::string& Response) {
                    std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                    if (r != "OK") {
                        return r;
                    }
                    return GetResultFromResponse(Response, "All");
                }));
            // Test the browser extension message to retrieve the card information (idflags set to 511 to retrieve all file)
            testResults.push_back(TestOp("Get Card Info (all fields) Browser Extension command", message_Get_Card_Info,
                [](const std::string& Response) {
                    std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                    if (r != "OK") {
                        return r;
                    }
                    return GetResultFromResponse(Response, "All");
                }));
#ifdef _DEBUG // Test generating exception to check the error handling on Card Info
            // Test error No_Reader 
            unittest_Generate_Exception_No_Reader = true;
            testResults.push_back(TestOp("Test error handling (Card Info (all fields)) no_reader", message_Get_Card_Info,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::no_reader);
                }));
            unittest_Generate_Exception_No_Reader = false;
            // Test error Card Removed
            unittest_Generate_Exception_Removed_Card = true;
            testResults.push_back(TestOp("Test error handling (Card Info (all fields)) Card Removed", message_Get_Card_Info,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::no_card);
                }));
            unittest_Generate_Exception_Removed_Card = false;
            // Test error Transaction Fail
            unittest_Generate_Exception_Transaction_Fail = true;
            testResults.push_back(TestOp("Test error handling (Card Info (all fields)) Transaction Fail", message_Get_Card_Info,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::busy);
                }));
            unittest_Generate_Exception_Transaction_Fail = false;
#endif
            // Test the browser extension message to retrieve the user certificates
            testResults.push_back(TestOp("Get User Certificates (USERCERTS) Browser Extension command", message_Get_User_Certificates,
                [](const std::string& Response) {
                    std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                    if (r != "OK") {
                        return r;
                    }
                    return GetResultFromResponse(Response, "All");
                }));
#ifdef _DEBUG // Test generating exception to check the error handling on Card Info
            // Test error No_Reader 
            unittest_Generate_Exception_No_Reader = true;
            testResults.push_back(TestOp("Test error handling (USERCERTS) no_reader", message_Get_User_Certificates,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::no_reader);
                }));
            unittest_Generate_Exception_No_Reader = false;
            // Test error Card Removed
            unittest_Generate_Exception_Removed_Card = true;
            testResults.push_back(TestOp("Test error handling (USERCERTS) Card Removed", message_Get_User_Certificates,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::no_card);
                }));
            unittest_Generate_Exception_Removed_Card = false;
            // Test error Transaction Fail
            unittest_Generate_Exception_Transaction_Fail = true;
            testResults.push_back(TestOp("Test error handling (USERCERTS) Transaction Fail", message_Get_User_Certificates,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::busy);
                }));
            unittest_Generate_Exception_Transaction_Fail = false;
#endif
            // Test the browser extension message to retrieve the certificate chain
            testResults.push_back(TestSign("Get Certificate Chain (CERTCHAIN) Browser Extension command", message_Get_Certificates_Chain, signCert->getBase64(), isPinPadReader, tc.PIN,
                [cr](const std::string& Response) {
                    std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                    if (r != "OK") {
                        return r;
                    }
                    return GetResultFromResponse(Response, cr->name);
                }));
#ifdef _DEBUG // Test generating exception to check the error handling on Card Info
            // Test error No_Reader 
            unittest_Generate_Exception_No_Reader = true;
            testResults.push_back(TestSign("Test error handling (CERTCHAIN) no_reader", message_Get_Certificates_Chain, signCert->getBase64(), isPinPadReader, tc.PIN,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::no_reader);
                }));
            unittest_Generate_Exception_No_Reader = false;
            // Test error Card Removed
            unittest_Generate_Exception_Removed_Card = true;
            testResults.push_back(TestSign("Test error handling (CERTCHAIN) Card Removed", message_Get_Certificates_Chain, signCert->getBase64(), isPinPadReader, tc.PIN,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::no_card);
                }));
            unittest_Generate_Exception_Removed_Card = false;
            // Test error Transaction Fail
            unittest_Generate_Exception_Transaction_Fail = true;
            testResults.push_back(TestSign("Test error handling (CERTCHAIN) Transaction Fail", message_Get_Certificates_Chain, signCert->getBase64(), isPinPadReader, tc.PIN,
                [](const std::string& Response) {
                    return ExpectResultInResponse(Response, BeidConnect_Result::busy);
                }));
            unittest_Generate_Exception_Transaction_Fail = false;
#endif
            // Check signing command only on the preconfigured (defined in the code) test smart card to avoid locking a real card
            if (signCert->getBase64() == tc.SignCert && tc.DoPINOps)
            {
                // Test the browser extension message to sign a SHA256 digest
                testResults.push_back(TestSign("Signing Browser Extension command (SHA256)", message_SHA256, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [cr](const std::string& Response) {
                        std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                        if (r != "OK") {
                            return r;
                        }
                        return GetResultFromResponse(Response, cr->name);
                    }));
                // Test the browser extension message to sign a SHA384 digest
                testResults.push_back(TestSign("Signing Browser Extension command (SHA384)", message_SHA384, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [cr](const std::string& Response) {
                        std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                        if (r != "OK") {
                            return r;
                        }
                        return GetResultFromResponse(Response, cr->name);
                    }));
                // Test the browser extension message to sign a SHA512 digest
                testResults.push_back(TestSign("Signing Browser Extension command (SHA512)", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [cr](const std::string& Response) {
                        std::string r = ExpectResultInResponse(Response, BeidConnect_Result::OK);
                        if (r != "OK") {
                            return r;
                        }
                        return GetResultFromResponse(Response, cr->name);
                    }));
#ifdef _DEBUG // Test generating exception to check the error handling
                // Test error Transaction Fail
                unittest_Generate_Exception_Transaction_Fail = true;
                testResults.push_back(TestSign("Test error handling (signing) Transaction Fail", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::busy);
                    }));
                unittest_Generate_Exception_Transaction_Fail = false;
                // Test error no_reader
                unittest_Generate_Exception_No_Reader = true;
                testResults.push_back(TestSign("Test error handling (signing) no_reader", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::no_reader);
                    }));
                unittest_Generate_Exception_No_Reader = false;
                // Test error unhandled CardException
                unittest_Generate_Exception_Other = true;
                testResults.push_back(TestSign("Test error handling (signing) general_error", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::general_error);
                    }));
                unittest_Generate_Exception_Other = false;
                // Test error Pin Blocked
                unittest_Generate_Exception_Pin_Blocked = true;
                testResults.push_back(TestSign("Test error handling (signing) card_blocked", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::card_blocked);
                    }));
                unittest_Generate_Exception_Pin_Blocked = false;
                // Test error Pin_3_attempts
                unittest_Generate_Exception_Pin_3_attempts = true;
                testResults.push_back(TestSign("Test error handling (signing) pin_3_attempts_left", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::pin_3_attempts_left);
                    }));
                unittest_Generate_Exception_Pin_3_attempts = false;
                // Test error Pin_2_attempts
                unittest_Generate_Exception_Pin_2_attempts = true;
                testResults.push_back(TestSign("Test error handling (signing) pin_2_attempts_left", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::pin_2_attempts_left);
                    }));
                unittest_Generate_Exception_Pin_2_attempts = false;
                // Test error Pin_1_attempts
                unittest_Generate_Exception_Pin_1_attempt = true;
                testResults.push_back(TestSign("Test error handling (signing) pin_1_attempt_left", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::pin_1_attempt_left);
                    }));
                unittest_Generate_Exception_Pin_1_attempt = false;
                // Test error pin_too_short
                unittest_Generate_Exception_PIN_TOO_SHORT = true;
                testResults.push_back(TestSign("Test error handling (signing) pin_too_short", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::pin_too_short);
                    }));
                unittest_Generate_Exception_PIN_TOO_SHORT = false;
                // Test error pin_too_long
                unittest_Generate_Exception_PIN_TOO_LONG = true;
                testResults.push_back(TestSign("Test error handling (signing) pin_too_long", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::pin_too_long);
                    }));
                unittest_Generate_Exception_PIN_TOO_LONG = false;
                // Test error pin_incorrect
                unittest_Generate_Exception_PIN_Incorrect = true;
                testResults.push_back(TestSign("Test error handling (signing) pin_incorrect", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                    [](const std::string& Response) {
                        return ExpectResultInResponse(Response, BeidConnect_Result::pin_incorrect);
                    }));
                unittest_Generate_Exception_PIN_Incorrect = false;
                // Test error PinPad TimeOut
                if (isPinPadReader) {
                    unittest_Generate_Exception_PinPad_TimeOut = true;
                    testResults.push_back(TestSign("Test error handling (signing) pin_timeout", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                        [](const std::string& Response) {
                            return ExpectResultInResponse(Response, BeidConnect_Result::pin_timeout);
                        }));
                    unittest_Generate_Exception_PinPad_TimeOut = false;
                }
                // Test error PinPad Cancel button
                if (isPinPadReader) {
                    unittest_Generate_Exception_PinPad_Cancel = true;
                    testResults.push_back(TestSign("Test error handling (signing) cancel", message_SHA512, signCert->getBase64(), isPinPadReader, tc.PIN,
                        [](const std::string& Response) {
                            return ExpectResultInResponse(Response, BeidConnect_Result::cancel);
                        }));
                    unittest_Generate_Exception_PinPad_Cancel = false;
                }
#endif
            }
            else
            {
                testResults.push_back(TestResult("Signing Browser Extension command (SHA256)", "SKIPPED", 0));
                testResults.push_back(TestResult("Signing Browser Extension command (SHA384)", "SKIPPED", 0));
                testResults.push_back(TestResult("Signing Browser Extension command (SHA512)", "SKIPPED", 0));
            }
        }
        else
        {
            std::cout << "no BEID card detected " << endl;
        }
    }

    std::cout << endl << "Test result" << endl;
    for (auto& r : testResults)
    {
        cout << r.Test << " (" << r.Duration << " ms)" << " : " << r.Result << endl;
    }
    getchar();
    return 0;
}


