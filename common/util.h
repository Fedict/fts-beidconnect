#pragma once

#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <stdlib.h>

// For Sleep() - usleep()
#ifdef _WIN32
#include "windows.h"
#else
#include <unistd.h>
#endif

void do_sleep(int msec);
int isEqual(const char* str1, int size1, const char* str2, int size2);
//void x_bin2hex(const unsigned char *p_bin, int l_bin, char *p_hex);
//void x_bin2hex_(const unsigned char *p_bin, int l_bin, char *p_hex);
//char* hex_dump(const unsigned char *p_bin, int l_bin);
//char* ascii_dump(const unsigned char *p_bin, int l_bin);
size_t base64decode_len(const std::string& bufcoded);
size_t base64decode(const std::string& bufcoded, unsigned char *bufplain);
size_t base64encode_len(size_t size_t);
size_t base64encode(const unsigned char *string, size_t len, unsigned char *encoded);

std::string rawToBase64(const std::vector<unsigned char>& raw);
const std::vector<unsigned char> base64ToRaw(const std::string& bufcoded);

#endif //
