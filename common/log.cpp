#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "log.hpp"

#define CLEANUP(x)   { err = (x); goto cleanup; }

/******************************************************************************
 *
 * Globals
 *
 ******************************************************************************/
char   g_szLogFile[MAX_PATH]="";
int    g_logStdOut;
int    g_logErrorStdOut;

/******************************************************************************
 *
 * log_init
 *
 ******************************************************************************/
void log_init(const char *pszLogFile, int logStdOut, int logErrorStdOut)
{
  strncpy(g_szLogFile, pszLogFile, sizeof(g_szLogFile));
   g_logStdOut = logStdOut;
   g_logErrorStdOut = logErrorStdOut;
}


/******************************************************************************
 *
 * log_info_ (does not add linefeed at end)
 *
 ******************************************************************************/
void log_info_(const char *string,... )
{
#ifdef LOG_INFO
  static char   buf[0x4000];    
	va_list       args;
  FILE          *fp;
  //EnterCriticalSection(&g_cs);
	if (strcmp(g_szLogFile, "") == 0)
		goto cleanup;
  if ((fp = fopen(g_szLogFile, "a")) == NULL)
    goto cleanup;
  
	va_start(args, string);				              // get args from param-string	
  #ifdef WIN32
	_vsnprintf(buf, sizeof(buf), string, args);	// convert to string	
  #else
   vsnprintf(buf, sizeof(buf), string, args);	// convert to string	
  #endif
	va_end(args);						                    // free arguments

#ifdef WIN32
	 fprintf(fp, "%s", buf);
#else
	 fprintf(fp, "%s", buf);
#endif
  fclose(fp);

#ifdef _DEBUG
   if (g_logStdOut)
      printf("%s", buf);
#endif

cleanup:
  //LeaveCriticalSection(&g_cs);
  return;
#endif
}

/******************************************************************************
 *
 * log_info
 *
 ******************************************************************************/
void log_info(const char *string,... )
{
#ifdef LOG_INFO
  static char   buf[0x4000];    
	va_list       args;
  FILE          *fp;
  //EnterCriticalSection(&g_cs);
	if (strcmp(g_szLogFile, "") == 0)
		goto cleanup;
  if ((fp = fopen(g_szLogFile, "a")) == NULL)
    goto cleanup;
  
	va_start(args, string);				              // get args from param-string	
  #ifdef WIN32
	_vsnprintf(buf, sizeof(buf), string, args);	// convert to string	
  #else
   vsnprintf(buf, sizeof(buf), string, args);	// convert to string	
  #endif
	va_end(args);						                    // free arguments

#ifdef WIN32
	 fprintf(fp, "%s\n", buf);
#else
	 fprintf(fp, "%s\n", buf);
#endif
  fclose(fp);

//#ifdef _DEBUG
   if (g_logStdOut)
      printf("%s\n", buf);
//#endif
   
cleanup:
  //LeaveCriticalSection(&g_cs);
  return;
#endif
}
/******************************************************************************
 *
 * log_error
 *
 ******************************************************************************/
void log_error(const char *string,... )
{
  static char   buf[0x4000];
   char timestring[20];
	va_list       args;
  FILE          *fp;
   time_t rawtime;
   struct tm * timeinfo;
  //EnterCriticalSection(&g_cs);

   time (&rawtime);
   timeinfo = localtime (&rawtime);
   strftime(timestring, 20, "%x-%X", timeinfo);
   
   if (strcmp(g_szLogFile, "") == 0)
		goto cleanup;
  if ((fp = fopen(g_szLogFile, "a")) == NULL)
    goto cleanup;
  
	va_start(args, string);				              // get args from param-string	
  #ifdef WIN32
	_vsnprintf(buf, sizeof(buf), string, args);	// convert to string	
  #else
   vsnprintf(buf, sizeof(buf), string, args);	// convert to string	
  #endif
	va_end(args);						                    // free arguments

#ifdef WIN32
	 fprintf(fp, "%s%s\n", timestring, buf);
#else
	 fprintf(fp, "%s%s\n", timestring, buf);
#endif
  fclose(fp);

#ifdef _DEBUG
   if (g_logErrorStdOut)
      printf("%s\n", buf);
#endif

cleanup:
  //LeaveCriticalSection(&g_cs);
  return;
}
