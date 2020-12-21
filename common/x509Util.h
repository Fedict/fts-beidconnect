#ifndef x509Util_h
#define x509Util_h


#include "time.h"

#ifdef _WIN32
#define snprintf _snprintf
#else
#define strnicmp strncasecmp
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LEN_DATE  14

//X509 asn1 paths for certificate elements
#define X509_ISSUER           "\1\1\4"
#define X509_SUBJECT          "\1\1\6"
#define X509_VALID_UNTIL      "\1\1\5\2"
#define X509_KEYINFO          "\1\1\7"

#define asc2uchar(a) (a-48)  // "7" -> 55
#define uchar2asc(a) (a+48)  // 55 -> "7"

//X509 extension KeyUsage flags
#define KEYUSAGE_DIGITAL_SIGNATURE    1
#define KEYUSAGE_NONREPUDIATION       2

enum X509_KEYTYPE {
   X509_KEYTYPE_RSA,
   X509_KEYTYPE_EC
};

char* getCertSubjectName(unsigned char* cert, unsigned int l_cert);
int isEndEntity(char* cert, unsigned int l_cert);
unsigned int getKeyUsage(char* cert, unsigned int l_cert);
int getKeyInfo(unsigned char *cert, unsigned int l_cert, int *keyType, int *keySize);
//unsigned int getRSAKeyLength(const char* cert, unsigned int l_cert);
char* getValidUntil(char* cert, unsigned int l_cert);
int isTimeBeforeNow (int len, char *p_atime);

#ifdef __cplusplus
   }
#endif

#endif /* x509Util_h */
