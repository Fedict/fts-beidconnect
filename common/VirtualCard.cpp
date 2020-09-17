#include "VirtualCard.hpp"
#include "log.hpp"
#include "util.h"
#include "hash.h"
#include "general.h"
#include "util.h"

#define test "test"

#define id_file "AQwwMDAwMDEwMTQ3NTkCEFNMR5BwZQAAtEUYbBKScTADCjMwLjA1LjIwMTcECjMwLjA1LjIwMjcFBkxldXZlbgYLOTgwODIxMDAwMjkHElNwZWNpbWVuIGVJRCAxOC03NQgKQWxpY2UgQmVydAkBQwoEQkVMRwsGTGV1dmVuDAsyMSBBVUcgMTk5OA0BVg4ADwExEAEwESBHaixAcvX7B2QuJW1e39ecamdfxpJi0yf5l5BRUXsIAQ=="

#define id_sig_file "T/pCV4h+WMvcqpaoGCNlOTof2S8P+TNN2LZgQp+wlJ19czsh3K7URdq3al8CHD2DilyV2Zev2gxp71aOHQNtiENpfkMu9jTdAGNqabsVbDoyZgwBEBW1DwFuMoxplrltIN0kY0s5gPWboph6j/l6tVUyqit2xdQEhkk/iaawjtheMdHhbdPyKB7KYFwoejdrqn6hrP0/jP0Om9W5jSTgPWflkMoAF+x/f6Fnu4/DjOhOYFJ8ff5eZqrMXKJdPkD1V2ZQe+Q0Bm6L32it9grd0Hp3eZCcteI4qtsiAH+RvV+JmtH7IpaT3kS/IkzHXOVDr9I60EYiib9lj8XRPiX1Lg=="

#define address_file "ARFTcGVjaW1lbnN0cmFhdCAxMgIEMDAwMAMGVXRvcGlhAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="

#define address_sig_file "FZAi0tZLJbACgJJiUEymIbZI1rSp2F4IR4VFvkbP0vt7PSEpATtkH82lssYiFv0W5d2Lcdghs0BuQBjPbw8yMxSLNq++LVZnzsZG+tDlEHHAw7zPnDK0xDK6uxcqjqgNWK0mtv7n5Sz2+LY7KDK33AViPkNeRq/w5Sf9AYqMxUS9GCSeo1ILcmkHaozP8rI8uo0ky11nbTjaCdVLkKVMwifKvvH9Hi+i/2G6RoftoZt0XaMt2JEambuf+PSQ8vT6duhEe61iJVw+MwdEjuOcrx2SIFuPRc5v7X9MehK83VNph7xhT/0ZLsVXbOyCKMIHcZcql+xKMTbtsYZgV/v5KA=="

#define photo_file "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"

#define rrn_cert "MIIEyjCCArKgAwIBAgIIZg7xFlP4U2owDQYJKoZIhvcNAQELBQAwKDELMAkGA1UEBhMCQkUxGTAXBgNVBAMMEGVJRCBURVNUIFJvb3QgQ0EwIBcNMDcwNDMwMjIwMDIwWhgPMjA1NzA0MTcyMjAwMjBaMDMxCzAJBgNVBAYTAkJFMQ0wCwYDVQQKDARURVNUMRUwEwYDVQQDDAxlSUQgVEVTVCBSUk4wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCgytleUhkenDsU3blwQoJZfLcgKCxX7s+u/a+fVcm34Hwyqhikn7RF7s8YADG0RE+JuUeaMZYbwDU4qaQLS8dJbO700t07Zx/d5FodxJBfypRKqXUH+etuKF748SBL4rDUPlFiRWV+EmeqSFDrG3kHfZj+maMapai+mdFDLInBoXMoBSX2Wr0gh/iyVzT3wWjwRfG7TE7IEn3NWSv4SSO9PAGsT3sG0LaODPMbhn58RaT1ZWVCSyOB2cHyGQw1XsqqnwbqqqT1kkeuIdH/FyhLZavAInVk6NjFheuQQDvyhxkJ8tmUlopmU8HFa+b0TTC1WI4WU9iWfjTOx6mWSOJLAgMBAAGjgeowgecwHQYDVR0OBBYEFD7Xtn/VCO9Y0NeD9PeabHxmPpSLMB8GA1UdIwQYMBaAFCIUjFNyFTz1JWbK3Tmsmx7DQi9gMA4GA1UdDwEB/wQEAwIGwDBIBgNVHSAEQTA/MD0GBmA4DAEBBDAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MEAGA1UdHwQ5MDcwNaAzoDGGL2h0dHA6Ly9laWRkZXZjYXJkcy56ZXRlc2NhcmRzLmJlL2NybC9yb290Y2EuY3JsMAkGA1UdEwQCMAAwDQYJKoZIhvcNAQELBQADggIBACSf0Rwp4Eth7sj6k9Uz5IZqXgf0jZt2wdB1CxDvvkDmsq/Gy2YyObhcYAVs+mPqp92hcGbPs1pRBJTMbc1Mc0cn6twSKsK6HbZK6UujwU0CMlkAzKpnPv3LdSaYv95Re8anyNIBMEYV6dGa+aBObzm6Lc6JHFjONRR4RmBLlEejWT43u1YqU+PCdu35nr3FJXBZbkkT30I54WLmD8ml5SB8PffzMGtRXaQu6A95W101/2pVgDH2zQ+XB7ImegWCVv0lapZh20SExFDalxH/Jjx0T8cvGDB5IWPsGz51d0Vhk33siVL/c/idBXXNPiPC7jhIZYpY4MEV9JFi+GYB/qtxAp9lbywno1yLclRs7wfeqHPxXOk1LrMsRXWLMceNZkzH46h2FenoA2h707U8NjbYX3jvJs2uOqwJ9c1tZ9T6p8Zyusaz8U+xZVDLLcFEegg6SPJjjwMVPaXlSEdezSMPPfgHQUPuT6tvxAI1P+98eEaREVdcwn2Xyp/7Jj+k/KtR4rCfbu0SLmwlNOV0C9UaQUfW3LFyfbBdOywBCxEUYtI9Bw134t4YAAkNGqau1LbaNWEow0pIKVMqHvYjnwi9K0UbA0Pavw9id8BdV+ACumbs9k4XYPSkVlv1qhsclUDKijovLKYn269mi3Q9UDwQMjP0BF2gAkEikoN6Iad3"

#define auth_cert  "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"

#define nonrep_cert  "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"

//eID SubCA
#define ca_cert "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"

//eID root ca
#define root_cert "MIIFjjCCA3agAwIBAgIBATANBgkqhkiG9w0BAQsFADAoMQswCQYDVQQGEwJCRTEZMBcGA1UEAwwQZUlEIFRFU1QgUm9vdCBDQTAgFw0wNzA0MzAyMjAwMTBaGA8yMDg4MDQwOTIyMDAxMFowKDELMAkGA1UEBhMCQkUxGTAXBgNVBAMMEGVJRCBURVNUIFJvb3QgQ0EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQCDghj3eJtmTqV6USkBkstPl+8mRZl0ACsiUSe6FY3O73RXjztH54pAQgvN1VFMZdUE8p5bh3erX2AFDOVHcqgYv+i1Lxn22hEynXTTgmrXlKPj0AAzjC754SZ/fD7anIsA14WKv9rf2vU0evSHdHhMQAzy/Tvp+J3r9+b56QvUm+KcBakCGs+ifek9nIWFwzPPh0cx2HUblRRhbboO7d9LLQi16So5GmhqPXRhp91WhXmBvFPSieLAeh1DcCdEQWup0hYlSzRpcYqgxW0jMsjXk34xh84l/oQ7b0r6yGBdkQMjBm9Si6gEdW5dhX6LuojUUZy6DYjWjmiULfMWhtkjZfRtHvVRNhfSQqb67ZFVjGsHIFGT/O9Exz33HdOZsJT2iAwEAOIGpebb9C0giDViBJeW3UsxGM5LN/LH14DRuad92ODIyQ+dTsKQf5JEd3NinGMQdzDiWw7YQ3ID+4WgacLxCYO5Ku1IRKTZfYHCKlDhNpbk9CJppzjNSqJj+3TqB5bOK4fuWgd4jlvodX5LD3s99iJiOMgQIyfkz2fUuxYjh5HpRDOKpmSlIJabxbdnV2Xp/4RA2Iflh22BANLggdDl0SCCIcfvaviFJcxLPv2QLmS4CvUTRyEWqbTtQMeo73wDo+20yP6gE0vVk94dp6nd6bhRCBIPldEa8djduwIDAQABo4HAMIG9MB0GA1UdDgQWBBQiFIxTchU89SVmyt05rJsew0IvYDAfBgNVHSMEGDAWgBQiFIxTchU89SVmyt05rJsew0IvYDAOBgNVHQ8BAf8EBAMCAQYwRwYDVR0gBEAwPjA8BgVgOAwBATAzMDEGCCsGAQUFBwIBFiVodHRwOi8vZWlkZGV2Y2FyZHMuemV0ZXNjYXJkcy5iZS9jZXJ0MBEGCWCGSAGG+EIBAQQEAwIABzAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4ICAQAFS3rF8jgW4DTWbOqwA+tf49PVW+UPSxghwCJp2NWcT3nbFgPI73buGa4VIYah5XYlPYNAiiGZmgyIZS/4qp/UmSx6WmvNBPEVlGdeDSxCRQ3Zp6axQ7x+JEMbRijwAqEzuCfQTEVnDJXsZxypHWSSyb0thg3ooz3pUFxN/YU90V3UdPvVN6Tkb8P6gaKXSOME2Tzx9AS5ccToqX7LOR/Tq9vr8zhI6Nh2Gg+cEDqam7gfntDVdz5EO0wPMEaeJUOh3+pU997rNYBxKiyhCASt6hJxSvi7tLD57LwCDhjwhW96I5+94Vpx3EM4pzR5Ajw2vSJEKMvBHReaAHfS7IW54KVuKYToZIQ+kaq6q4gt42zNZ8jnD+Ki+Hkjkcybm0AzKVNCtcbiFcaqYDkl8E/ewgky0mMXXvQ36ON7TSUxpNu8upcyXQrqPbvHWDVz1VJQfiV7dnEHDDqKieiLDu5GWgFBB6XKm+c5Gk9ktMCguJ9MpOmDkE070MGu4A6Q18qqsmML5QKBtXKKgj2ZyDoaOCu+3Nx1FIYacIz+yswb+pBI6F29a57ME3AccQed4UPlUCf3rgUGHSwBSwUf0ODBudz2UurnEzvfBgg+rIPLPQrWCL+bfna95ep7CEVOFOz+vuHVxTIt/iWjkicH7c09TotcNbFWqJmWShf7NoFjhw=="

std::string VirtualCard::strType()
{
   return "BEID";
};


int VirtualCard::type()
{
   return CARD_TYPE_BEID;
   //return CARD_TYPE_PKCS15;
};


#define WHERE "VirtualCard::read_certificate"
int VirtualCard::readCertificate(int type, int format, int* l_cert, unsigned char** pp_cert)
{
   int ret = 0;
   
   /* select certificate */
   switch (type)
   {
      case CERT_TYPE_AUTH:
         *l_cert = sizeof(auth_cert /* -1 */); //alloc + 1 byte to add 0
         if ((*pp_cert = (unsigned char*) malloc(*l_cert)) == NULL) {
            CLEANUP(E_ALLOC_ERR);
         }
         memcpy(*pp_cert, auth_cert, *l_cert);
         break;
      case CERT_TYPE_NONREP:
         *l_cert = sizeof(nonrep_cert);
         if ((*pp_cert = (unsigned char*) malloc(*l_cert)) == NULL) {
            CLEANUP(E_ALLOC_ERR);
         }
         memcpy(*pp_cert, nonrep_cert, *l_cert);
         break;
      case CERT_TYPE_CA:
         *l_cert = sizeof(ca_cert);
         if ((*pp_cert = (unsigned char*) malloc(*l_cert)) == NULL) {
            CLEANUP(E_ALLOC_ERR);
         }
         memcpy(*pp_cert, ca_cert, *l_cert);
         break;
      case CERT_TYPE_ROOT:
         *l_cert = sizeof(root_cert);
         if ((*pp_cert = (unsigned char*) malloc(*l_cert)) == NULL) {
            CLEANUP(E_ALLOC_ERR);
         }
         memcpy(*pp_cert, root_cert, *l_cert);
         break;
      default:
         log_error("%s: wrong certificate type (bad implementation)", WHERE);
         CLEANUP(-1);
   }
   
   if (format == FORMAT_RADIX64) {
   }
   else {
      ret = -1; //not supported in virtual card since we don't need it anyhow
   }
   
   do_sleep(500); //virtual read time
   
cleanup:
   
   return (ret);
}
#undef WHERE



#define WHERE "VirtualCard::readUserCertificates"
int VirtualCard::readUserCertificates(int format, std::vector<std::vector<char>> &certificates)
{
   int ret = 0;
   if (format == FORMAT_RADIX64) {

      std::vector<char> buf1(auth_cert, auth_cert + sizeof(auth_cert)-1);
      certificates.push_back(buf1);

      std::vector<char> buf2(nonrep_cert, nonrep_cert + sizeof(nonrep_cert)-1);
      certificates.push_back(buf2);
   }
   else if (format == FORMAT_HEX) {
      ret = -1; //not supported in virtual card since we don't need it anyhow
   }
   else {

      unsigned char *buf;
      int l_buf = base64decode_len((const unsigned char*) auth_cert);
      buf = (unsigned char*) malloc(l_buf);
      if (buf == 0) {
         log_error("%s mem alloc error ", WHERE);
         return -1;
      }
      int n = base64decode((const unsigned char*)auth_cert, buf);
      std::vector<char> buf1(buf, buf + n);
      certificates.push_back(buf1);
      free (buf);

      l_buf = base64decode_len((const unsigned char*) nonrep_cert);
      buf = (unsigned char*) malloc(l_buf);
      if (buf == 0) {
         log_error("%s mem alloc error ", WHERE);
         return -1;
      }
      n = base64decode((const unsigned char*)nonrep_cert, buf);
      std::vector<char> buf2(buf, buf + n);
      certificates.push_back(buf2);
      free (buf);
      ret = 0;
   }

   do_sleep(1000); //virtual read time

   return (ret);
}
#undef WHERE


#define WHERE "VirtualCard::readCertificateChain"
int VirtualCard::readCertificateChain(int format, unsigned char *cert, int l_cert, std::vector<std::vector<char>>  &subCerts, std::vector<char> &root)
{
   int ret = 0;
   if (format == FORMAT_RADIX64) {
      
      std::vector<char> buf1(ca_cert, ca_cert + sizeof(ca_cert)-1);
      subCerts.push_back(buf1);
      
      std::vector<char> buf2(root_cert, root_cert + sizeof(root_cert)-1);
      root = buf2;
   }
   else if (format == FORMAT_HEX) {
      ret = -1;
   }
   else {
      ret = -1;
   }

   do_sleep(1000); //virtual read time

   return (ret);
}
#undef WHERE



#define WHERE "VirtualCard::SelectKey()"
int VirtualCard::selectKey(int pintype, unsigned char* cert, int l_cert)
{
   int ret = 0;

#define eaZyID_PROVIDES_SIGNING_CERT    0
   
#if (eaZyID_PROVIDES_SIGNING_CERT)
   //compare first 500 bytes with certificate we want to use to make sure we are using the correct card
   //This should be changed to find out if we want to do an authentication or non-rep signing depending on input certificate
   if (memcmp(buf, cert, 500) != 0) {
      log_error("I: Virtual card does not contain the key for the requested certificate");
      ret = - 1;
      goto cleanup;
   }
#endif
   
   if (pintype == CERT_TYPE_NONREP)
   {
   }
   else
   {
   }
   
   return ret;
}


int VirtualCard::logon(int l_pin, char *pin)
{
   int ret = 0;
   
   //test Pin conditions
   if ((l_pin > 0) && (l_pin < 4))
      CLEANUP(E_PIN_TOO_SHORT);
   if (l_pin > 12)
      CLEANUP(E_PIN_TOO_LONG);
   

   if ((l_pin == 0) && (reader->isPinPad())) {
      ret = 0;
   }
   else if (pin != 0) {
      
      if (strcmp(pin, "1234") == 0)
         ret = 0;
      else if (strcmp(pin, "1235") == 0)
         ret = E_PIN_2_ATTEMPTS;
      else if (strcmp(pin, "1236") == 0)
         ret = E_PIN_1_ATTEMPT;
      else if (strcmp(pin, "1237") == 0)
         ret = E_PIN_BLOCKED;
      else
         ret = E_PIN_INCORRECT;
   }
   else
      ret = -1;
  
  
cleanup:
   
   return (ret);
}


int VirtualCard::logoff()
{
   return 0;
}

int VirtualCard::sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw)
{
   int ret = 0;
   char rand[256] = "the quick brown fox or something...";
   
   if ((l_in == 0) || (l_in != hash_length_for_algo(hashAlgo)) ) {
      log_error("hash input has wrong length for the specified digesting algo");
      CLEANUP(E_DIGEST_LEN);
   }
   
   memcpy(out, rand, 256);
   *l_out = (unsigned int) 256;
   *sw = 0x9000;
   
   do_sleep(300); //virtual read time

cleanup:

   return (ret);
}

int VirtualCard::getFile(int fileType, int* l_out, unsigned char* p_out)
{
   int ret = 0;

   switch (fileType) {
         
      case IDFILE:
         *l_out = base64decode((unsigned char*) id_file, p_out);
         do_sleep(100); //virtual read time
         break;
      case IDSIGFILE:
         *l_out = base64decode((unsigned char*) id_sig_file, p_out);
         do_sleep(100); //virtual read time
         break;
      case ADDRESSFILE:
         *l_out = base64decode((unsigned char*) address_file, p_out);
         do_sleep(100); //virtual read time
         break;
      case ADDRESSSIGFILE:
         *l_out = base64decode((unsigned char*) address_sig_file, p_out);
         do_sleep(100); //virtual read time
         break;
      case PHOTOFILE:
         *l_out = base64decode((unsigned char*) photo_file, p_out);
         do_sleep(1000); //virtual read time
         break;
      case RRNCERT:
         *l_out = base64decode((unsigned char*) rrn_cert, p_out);
         do_sleep(500); //virtual read time
         break;
      case AUTHCERT:
         *l_out = base64decode((unsigned char*) auth_cert, p_out);
         do_sleep(500); //virtual read time
         break;
      case SIGNCERT:
         *l_out = base64decode((unsigned char*) nonrep_cert, p_out);
         do_sleep(500); //virtual read time
         break;
      case CACERT:
         *l_out = base64decode((unsigned char*) ca_cert, p_out);
         do_sleep(500); //virtual read time
         break;
      case ROOTCERT:
         *l_out = base64decode((unsigned char*) root_cert, p_out);
         do_sleep(500); //virtual read time
         break;
      default: ret = E_SRC_FILETYPE_NOT_SUPPORTED;
   }
   
   return ret;
}



