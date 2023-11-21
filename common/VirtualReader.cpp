#include "VirtualReader.hpp"
#include "Card.hpp"
#include "CardFactory.hpp"
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "log.hpp"
#include "general.h"
#include "boost/property_tree/ini_parser.hpp"


#ifndef _WIN32
#define lstrlen(s) strlen(s)   // non-Windows wintypes.h: LPTSTR == LPSTR == char*
#endif

#ifdef _WIN32
#include "windows.h"
#endif


VirtualReader::VirtualReader()
{
}

VirtualReader::~VirtualReader()
{
}

#define WHERE "VirtualReader::listReaders()"
int VirtualReader::listReaders(std::vector<std::shared_ptr<CardReader>>& readers)
{
   int ret = 0;
   
   try {
      //read config file for virtual devices
      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini(CONFIG_FILE, pt);
 
      for (auto& section : pt)
      {
         std::shared_ptr<VirtualReader> reader = std::make_shared<VirtualReader>();
         reader->name = section.first;
         //std::cout << '[' << section.first << "]\n";
         reader->atr = "";
         for (auto& key : section.second) {

            if (key.first == "atr") {
               reader->atr = std::string( key.second.get_value<std::string>() );
            }
         }
         readers.push_back(reader);
      }
   }
   catch(...) {
      return 0;
   }
   
   return (ret);
}
#undef WHERE


#define WHERE "VirtualReader::connect()"
long VirtualReader::connect()
{
   long ret = 0;
   return (ret);
}
#undef WHERE


#define WHERE "VirtualReader::disconnect()"
void VirtualReader::disconnect()
{
}
#undef WHERE


