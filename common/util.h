#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// For Sleep() - usleep()
#ifdef WIN32
#include "windows.h"
#else
#include <unistd.h>
#endif

void do_sleep(int msec);
int isEqual(const char* str1, int size1, const char* str2, int size2);
void x_bin2hex(const unsigned char *p_bin, int l_bin, char *p_hex);
void x_bin2hex_(const unsigned char *p_bin, int l_bin, char *p_hex);
int hex2bin(const char *hex, int l_bin, char* bin);
char* hex_dump(const unsigned char *p_bin, int l_bin);
char* ascii_dump(const unsigned char *p_bin, int l_bin);
int base64decode_len(const unsigned char *bufcoded);
int base64decode(const unsigned char *bufcoded, unsigned char *bufplain);
int base64encode_len(int len);
int base64encode(const unsigned char *string, int len, unsigned char *encoded);

#endif //
