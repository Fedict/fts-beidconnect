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

void writeFile(string file, string exePath, bool isChrome) {
    ofstream myfile;
    myfile.open(file, std::ofstream::trunc);
    myfile << "{\n";
    myfile << "  \"name\": \"be.bosa.beidconnect\",\n";
    myfile << "  \"description\": \"Access your eID in webapps\",\n";
    myfile << "  \"path\": \"" << exePath << "\",\n";
    myfile << "  \"type\": \"stdio\",\n";
    if (isChrome) {
        myfile << "  \"allowed_origins\": [\n";
        myfile << "     \"chrome-extension://pencgnkbgaekikmiahiaakjdgaibiipp/\"\n";
        myfile << "  ]\n";
    } else {
        myfile << "  \"allowed_extensions\": [\n";
        myfile << "     \"beidconnect@bosa.be\"\n";
        myfile << "  ]\n";
    }

    myfile << "}\n";
    myfile.close();
}

int runSetup(int argc, const char * argv[])
{
   //generate a json file that is needed for the Chrome Extension to find the Native host application on Windows
   const char* installFolder = NULL;
   string chromeFilePath = "";
   string firefoxFilePath = "";
   string exePath;
   
   for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-setup") == 0) {
         
         //beidconnect -setup installFolder [chromeFilePath] [firefoxFilePath]
         installFolder = argv[++i];
      } else if (i < argc && chromeFilePath=="") {
         chromeFilePath = string(argv[i]) + "/be.bosa.beidconnect.json";
      } else if (i < argc && firefoxFilePath == "") {
          firefoxFilePath = string(argv[i]) + "/be.bosa.beidconnect.json";
      }
   }
   
   if (installFolder == NULL) {
      return (0);
   }
   
   //log_info("install folder: <%s>", installFolder);
   
#ifdef _WIN32
   exePath = string(installFolder) + "\\beidconnect.exe";
   
   //escape all \ in json file or exe will not be found on windows
   exePath = std::regex_replace(exePath, std::regex("\\\\"), "\\\\");
   
   if (chromeFilePath == "") {
      chromeFilePath = string(installFolder) + "\\chrome.json";
   }
   if (firefoxFilePath == "") {
       firefoxFilePath = string(installFolder) + "\\firefox.json";
   }
#else
   exePath = string(installFolder) + "/beidconnect";
   if (chromeFilePath == "") {
      chromeFilePath = string(installFolder) + "/chrome.json";
   }
   if (firefoxFilePath == "") {
      firefoxFilePath = string(installFolder) + "/firefox.json";
   }
#endif
   //log_info("creating %s", chromeFilePath.c_str());
   writeFile(chromeFilePath, exePath, true);
   //log_info("creating %s", firefoxFilePath.c_str());
   writeFile(firefoxFilePath, exePath, false);
   return 0;
}
