#pragma once

#ifndef __LOG_H
#define __LOG_H

#ifndef MAX_PATH
#define MAX_PATH          260
#endif

#ifdef __cplusplus
  extern "C" {
#endif

void log_init(const char *pszLogFile, int logStdOut, int logErrorStdOut);
void log_info_(const char *string,... );
void log_info(const char *string,... );
void log_error(const char *string,... );
 
#ifdef __cplusplus
  }
#endif


#endif
