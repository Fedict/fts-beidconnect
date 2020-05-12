/*
*   some crypto functions
*/
#include "crypto.h"
#include "log.hpp"
#include "util.h"

#ifdef _WIN32
#include "shlobj.h"
#undef X509_NAME
#undef X509_EXTENSIONS
#undef X509_CERT_PAIR
#undef PKCS7_ISSUER_AND_SERIAL
#undef OCSP_RESPONSE
#endif

#if WITH_OPENSSL
#include <openssl/pem.h>
#include <openssl/conf.h>

#define LOG_FAIL_IF(cond, msg, rv) if((cond)) { log_error(msg); ret = rv; goto cleanup; }

#include <openssl/x509v3.h>
#define OPENSSL_NO_ENGINE 1
#ifndef OPENSSL_NO_ENGINE
# include <openssl/engine.h>
#endif

#ifdef __APPLE__
#define TCHAR     unsigned char
#endif

int verifySignature(unsigned char *plain, unsigned int l_plain, const char *hashalg, const unsigned char *cert, int l_cert, unsigned char *signature, unsigned l_signature)
{
   X509 *x509 = NULL;
   EVP_PKEY *pkey = NULL;
   EVP_MD_CTX *mdctx = NULL;
   EVP_PKEY_CTX *pctx = NULL;
   int ret = 0;
   const EVP_MD *md = EVP_get_digestbyname(hashalg);

   LOG_FAIL_IF((d2i_X509(&x509, &cert, l_cert) == NULL), "Certificate cannot be parsed", -1);
   pkey = X509_get0_pubkey(x509);
   LOG_FAIL_IF((pkey == NULL), "Could not parse public key from certificate", -1);
   mdctx = EVP_MD_CTX_new();
   LOG_FAIL_IF((EVP_DigestVerifyInit(mdctx, &pctx, md, NULL, pkey) != 1), "Could not initialize verification context", -1);
   LOG_FAIL_IF((EVP_DigestVerifyUpdate(mdctx, plain, l_plain) != 1), "Could not read signed data", -1);
   LOG_FAIL_IF((EVP_DigestVerifyFinal(mdctx, signature, l_signature) != 1), "Could not verify signature", -1);
cleanup:
   // pctx is owned by mdctx; and since we used X509_get0_pubkey, pkey is owned by x509
   if(mdctx) {
      EVP_MD_CTX_free(mdctx);
   }
   X509_free(x509);
   return ret;
}
#else
#include "x509Util.h"
int verifySignature(unsigned char *plain, unsigned int l_plain, const char *hashalg, const unsigned char *cert, int l_cert, unsigned char *signature, unsigned l_signature)
{
   unsigned int length = getRSAKeyLength((const char*)cert, l_cert);

   if((length != 0) && (length != l_signature)) {
      log_error("Signature length does not match key length");
      return -1;
   }
   return 0;
}
#endif
