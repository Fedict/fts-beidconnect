#pragma once

#ifndef COMMON_CRYPTO_H
#define COMMON_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

   int verifySignature(unsigned char *plain, unsigned int l_plain, const char *hashalg, const unsigned char *cert, int l_cert, unsigned char *signature, unsigned l_signature);

#ifdef __cplusplus
   }
#endif

#endif // COMMON_CRYPTO_H
