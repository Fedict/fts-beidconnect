#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "comm.hpp"
#include "log.hpp"
#include "util.h"

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;

#define WHERE "readMessage()"
int readMessage(std::stringstream &stream)
{
   int length = 0;
   int read_char;
   int i;

   #ifdef _WIN32
      _setmode(_fileno(stdin), _O_BINARY);
      _setmode(_fileno(stdout), _O_BINARY);
   #endif

   for (i = 0; i < 4; i++) {
      while ((read_char = getchar()) == EOF) {
         do_sleep(20);
      }
        length = length | (read_char << i*8);
    }

   if (length < 0)    {
      log_error("%s: length (%d) < 0)", WHERE, length);
      return(E_COMM_PARAM);
   }

   //sanity check
   if (length > 10000) {
      log_error("%s: message length exceeds max size (%d > 10000)", WHERE, length);
      return(E_COMM_PARAM);
   }

   for (i = 0; i < length; i++) {
      read_char = getchar();
      stream << (unsigned char)read_char;
   }

   return 0;
}
#undef WHERE



#define WHERE "sendMessage()"
int sendMessage(const std::string response)
{
   size_t length = response.length();
   //include nullterminator here
   uint32_t len = (uint32_t) length + 1;

   fwrite(&len, 1, 4, stdout);

   //log_info("%s: Response (%i); %s", WHERE, len, response.c_str());
   
   cout << response << "\n";
   // printf("%s\n", pMessage);

    return 0;
}
#undef WHERE
