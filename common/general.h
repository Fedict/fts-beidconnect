#if !defined(__GENERAL_INC__)
#define __GENERAL_INC__

#define RET_OK						0
#define E_ALLOC_ERR             ((unsigned int)0xB0010003)

#ifdef __cplusplus
  extern "C" {
#endif

/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/

#define BEIDCONNECT_VERSION	"2.10"

#define CLEANUP(a) { ret = a; goto cleanup; }
     
#ifdef _WIN32
#define CONFIG_FILE "c:\\tmp\\virtualdevice.cfg"
#define LOG_FILE  "C:\\tmp\\beidconnect.log"
#define TEST_FILE  "C:\\tmp\\beidconnecttestdb.json"
#else
#define CONFIG_FILE "/tmp/virtualdevice.cfg"
#define LOG_FILE  "/tmp/beidconnect.log"
#define TEST_FILE  "/tmp/beidconnecttestdb.json"
#endif
     
#define MAX_READER_NAME			  128
#define MAX_CARDTYPE_ID         20
#define MAX_ID_FILE_SIZE        20000
#define READER_TYPE_STANDARD    1
#define READER_TYPE_PINPAD      2

#define CERT_TYPE_NONREP		1
#define CERT_TYPE_AUTH			2
#define CERT_TYPE_CA          3
#define CERT_TYPE_ROOT			4
#define CERT_TYPE_RRN			5
#define CERT_TYPE_USERCERTS   6
#define CERT_TYPE_CHAIN       7

#define ID_FLAG_INCLUDE_ID          1
#define ID_FLAG_INCLUDE_ADDR        2
#define ID_FLAG_INCLUDE_PHOTO       4
#define ID_FLAG_INCLUDE_INTEGRITY   8
#define ID_FLAG_INCLUDE_CERTS       16
#define ID_FLAG_INCLUDE_AUTH_CERT   32
#define ID_FLAG_INCLUDE_SIGN_CERT   64
#define ID_FLAG_INCLUDE_CACERTS     128
#define ID_FLAG_INCLUDE_ROOTCERT    256

#define LG_DUTCH                  1
#define LG_FRENCH                 2
#define LG_ENGLISH                3
#define LG_GERMAN                 4
          
#ifdef __cplusplus
  }
#endif

#endif
