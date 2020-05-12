#include "log.hpp"
#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include "util.h"
#include <string.h>
#include <regex>

#ifdef _WIN32
#include <io.h>
#endif

#ifndef _WIN32
#define TCHAR     unsigned char
#endif

using namespace std;

int runSetup(int argc, const char * argv[])
{
   //generate a json file that is needed for the Chrome Extension to find the Native host application on Windows
   const char* installFolder = NULL;
   string jsonFilePath = "";
   string exePath;
   const char* type = NULL;
   
   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-setup") == 0) {
         
         //eidlink -setup type installFolder [jsonFilePath]
         type = argv[++i];
         installFolder = argv[++i];
      } else if (i < argc) {
         jsonFilePath = string(argv[i]) + "/be.bosa.eidlink.json";
      }
   }
   
   if (type == NULL || installFolder == NULL) {
      return (0);
   }
   
   //log_info("install folder: <%s>", installFolder);
   
#ifdef _WIN32
   exePath = string(installFolder) + "eidlink.exe";
   
   //escape all \ in json file or exe will not be found on windows
   exePath = std::regex_replace(exePath, std::regex("\\\\"), "\\\\");
   
   if (jsonFilePath == "") {
      jsonFilePath = string(installFolder) + "\\be.bosa.eidlink.json";
   }
#else
   exePath = string(installFolder) + "/eidlink";
   if (jsonFilePath == "") {
      jsonFilePath = string(installFolder) + "/be.bosa.eidlink.json";
   }
#endif
   
   if (strcmp(type, "chrome") != 0 && strcmp(type, "firefox") != 0) {
      log_error("invalid type; expecting 'chrome' or 'firefox'");
   }
   ofstream myfile;
   myfile.open (jsonFilePath, std::ofstream::trunc);
   myfile << "{\n";
   myfile << "  \"name\": \"be.bosa.eidlink\",\n";
   myfile << "  \"description\": \"Access your eID in webapps\",\n";
   myfile << "  \"path\": \"" << exePath << "\",\n";
   myfile << "  \"type\": \"stdio\",\n";
   if (strcmp(type, "chrome") == NULL) {
      myfile << "  \"allowed_origins\": [\n";
      myfile << "     \"chrome-extension://pencgnkbgaekikmiahiaakjdgaibiipp/\"\n";
      myfile << "  ]\n";
   } else {
      myfile << "  \"allowed_extensions\": [\n";
      myfile << "     \"eidlink@bosa.be\"\n";
      myfile << "  ]\n";
   }

   myfile << "}\n";
   myfile.close();
   
   return 0;
}
