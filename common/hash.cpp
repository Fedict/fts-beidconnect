#include "hash.h"
#include "string.h"

int hash_length_for_algo(int hashAlgo)
{
	switch (hashAlgo) 
	{
	case DIGEST_ALGO_SHA256:
		return DIGEST_ALGO_SHA256_LEN;
	case DIGEST_ALGO_SHA384:
		return DIGEST_ALGO_SHA384_LEN;
	case DIGEST_ALGO_SHA512:
		return DIGEST_ALGO_SHA512_LEN;
	default:
		return 0;
	}
}

int hash_length_for_algoheader(int hashAlgo)
{
	switch (hashAlgo) 
	{
	case DIGEST_ALGO_SHA256:
		return DIGEST_ALGO_SHA256_HEAD_LEN;
	case DIGEST_ALGO_SHA384:
		return DIGEST_ALGO_SHA384_HEAD_LEN;
	case DIGEST_ALGO_SHA512:
		return DIGEST_ALGO_SHA512_HEAD_LEN;
	default:
		return -1;
	}
}

const char* hash_header_for_algo(int hashAlgo)
{
	switch (hashAlgo) 
	{
		case DIGEST_ALGO_SHA256:
			return DIGEST_ALGO_SHA256_HEAD;
		case DIGEST_ALGO_SHA384:
			return DIGEST_ALGO_SHA384_HEAD;
		case DIGEST_ALGO_SHA512:
			return DIGEST_ALGO_SHA512_HEAD;
		default:
			return 0;
	}
}


int algo2str(const char *hash_algo)
{
   if (!strcmp(hash_algo, "SHA384") || !strcmp(hash_algo, "SHA-384"))
      return DIGEST_ALGO_SHA384;
   if (!strcmp(hash_algo, "SHA512") || !strcmp(hash_algo, "SHA-512"))
      return DIGEST_ALGO_SHA512;
   return DIGEST_ALGO_SHA256;
}
