#ifndef ___crypto_h____
#define ___crypto_h____

#ifdef __cplusplus
extern "C" {
#endif

   int verifyRSASignature(unsigned char *plain, unsigned int l_plain, const unsigned char* cert, int l_cert, unsigned char *signature, unsigned l_signature);
   int verifyRSASignature2(unsigned char *plain, unsigned int l_plain, const unsigned char* cert, int l_cert, unsigned char *signature, unsigned l_signature);

#ifdef __cplusplus
   }
#endif

#endif
