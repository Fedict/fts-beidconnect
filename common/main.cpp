//
//  main.cpp
//
//  Created by Vital on Feb 19, 2020.
//
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.hpp"
#include "log.hpp"
#include "native.hpp"
#include "setup.hpp"

#ifdef _WIN32
//#define CONFIG_FILE "c:\\tmp\\virtualdevice.cfg"
#define LOG_FILE  "C:\\tmp\\eIDLink.log"
#else
//#define CONFIG_FILE "/tmp/virtualdevice.cfg"
#define LOG_FILE  "/tmp/eIDLink.log"
#endif

int main(int argc, const char * argv[]) {
   
   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-service") == 0) {

         log_init(LOG_FILE, 1, 1);
         //runService();
         exit(0);
      }
      else if (strcmp(argv[i], "-test") == 0) {

         log_init(LOG_FILE, 1, 1);
         //runTest(argc, argv);  //run as test and exit
         exit(0);
      }
      else if (strcmp(argv[i], "-setup") == 0) {

	 //generate the necessary json file for use with the Chrome/Firefox
	 //extensions
         log_init(LOG_FILE, 1, 1);
         log_info("eIDLink setup");
         runSetup(argc, argv);
         exit(0);
      }
   }

   log_init(LOG_FILE, 0, 0);
   log_info("**** eIDLink native ****");
   runNative(argc, argv);    //run as chrome extension and exit afterwards
   return 0;
}


