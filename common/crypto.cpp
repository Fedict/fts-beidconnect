/*
*   some crypto functions
*/
#include <stdio.h>
#include <stdlib.h>

#include <openssl/pem.h>
#include <openssl/conf.h>
#include "crypto.h"
#include "log.hpp"
#include "util.h"

#define CLEANUP(i) {iReturn=(i);goto cleanup;}

#ifdef _WIN32
#include "shlobj.h"
#undef X509_NAME
#undef X509_EXTENSIONS
#undef X509_CERT_PAIR
#undef PKCS7_ISSUER_AND_SERIAL
#undef OCSP_RESPONSE
#endif


#include <openssl/x509v3.h>
#define OPENSSL_NO_ENGINE 1
#ifndef OPENSSL_NO_ENGINE
# include <openssl/engine.h>
#endif

#ifdef __APPLE__
#define TCHAR     unsigned char
//#define MAX_PATH  512
#endif



int verifyRSASignature(unsigned char *plain, unsigned int l_plain, const unsigned char* cert, int l_cert, unsigned char *signature, unsigned l_signature)
{
   int ret = 0;
   int result;
   X509 *x509 = NULL;
   EVP_PKEY *evp_pubkey;
   RSA *rsa_pubkey;
   
   if(!d2i_X509(&x509, &cert, l_cert)) {
      goto cleanup;
   }
   
   evp_pubkey  = X509_get_pubkey(x509);
   rsa_pubkey  = EVP_PKEY_get1_RSA(evp_pubkey);
   
   result      = RSA_verify(NID_sha256, plain, l_plain, signature, l_signature, rsa_pubkey);
   
   if (result == 1) {
	   log_info("Signature is valid");
   }
   else {
	   log_error("Signature is invalid");
      ret = -1;
   }
   
   RSA_free(rsa_pubkey);
   EVP_PKEY_free(evp_pubkey);
   X509_free(x509);

cleanup:
   ;// Cleanup
   return (ret);
}


int verifyRSASignature2(unsigned char *plain, unsigned int l_plain, const unsigned char* cert, int l_cert, unsigned char *signature, unsigned l_signature)
{
   X509 *x509 = NULL;
   EVP_PKEY *evp_pubkey;
   RSA *rsa_pubkey;
   unsigned char to[512];
   int l_to = 512;
   char *plainhex = 0;
   char *hex = 0;
   
   if(!d2i_X509(&x509, &cert, l_cert)) {
      goto cleanup;
   }
   
   evp_pubkey  = X509_get_pubkey(x509);
   rsa_pubkey  = EVP_PKEY_get1_RSA(evp_pubkey);
   
   l_to = RSA_public_decrypt(l_signature, signature, to, rsa_pubkey, RSA_NO_PADDING);
   if (l_to < 0)
      goto cleanup;
   
   if (l_to < 1000)
      hex = (char*) malloc(l_to);
 
   plainhex = (char*) malloc(l_plain * 2);
   
   x_bin2hex(plain, l_plain, plainhex);
   x_bin2hex(to, l_to, hex);
   
   log_info("plain:               %s", plainhex);
   log_info("decrypted signature: %s", hex);

   RSA_free(rsa_pubkey);
   EVP_PKEY_free(evp_pubkey);
   X509_free(x509);
   
cleanup:
   // Cleanup
   ;//RSA_free(rsa_pubkey);
   return 0;
}

