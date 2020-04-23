#ifndef ___crypto_h____
#define ___crypto_h____

#ifdef __cplusplus
extern "C" {
#endif

   int verifySignature(unsigned char *plain, unsigned int l_plain, const char *hashalg, const unsigned char *cert, int l_cert, unsigned char *signature, unsigned l_signature);

#ifdef __cplusplus
   }
#endif

#endif
