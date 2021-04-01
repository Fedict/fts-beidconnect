#ifndef __hash_h
#define __hash_h

#define E_DIGEST_LEN				0xB0020001


#ifdef __cplusplus
  extern "C" {
#endif

#define DIGEST_ALGO_SHA256          2
#define DIGEST_ALGO_SHA384          3
#define DIGEST_ALGO_SHA512          4

#define DIGEST_ALGO_SHA256_HEAD     "\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20"
#define DIGEST_ALGO_SHA384_HEAD     "\x30\x41\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x02\x05\x00\x04\x30"
#define DIGEST_ALGO_SHA512_HEAD     "\x30\x51\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03\x05\x00\x04\x40"

#define DIGEST_ALGO_SHA256_HEAD_LEN    19
#define DIGEST_ALGO_SHA384_HEAD_LEN    19
#define DIGEST_ALGO_SHA512_HEAD_LEN    19

#define DIGEST_ALGO_SHA256_LEN      32
#define DIGEST_ALGO_SHA384_LEN      48
#define DIGEST_ALGO_SHA512_LEN      64

int hash_length_for_algo(int hashAlgo);
const char* hash_header_for_algo(int hashAlgo);
int hash_length_for_algoheader(int hashAlgo);
int algo2str(const char *hash_algo);

     
#ifdef __cplusplus
  }
#endif

#endif
