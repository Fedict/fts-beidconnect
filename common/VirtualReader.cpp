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

#define WHERE "VirtualReader::open()"
int VirtualReader::open()
{
   int ret = 0;
   return (ret);
};
#undef WHERE


int VirtualReader::close()
{
   return 0;
}

#define WHERE "VirtualReader::listReaders()"
int VirtualReader::listReaders(std::vector<CardReader::Ptr> & readers)
{
   int ret = 0;
   
   try {
      //read config file for virtual devices
      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini(CONFIG_FILE, pt);
 
      for (auto& section : pt)
      {
         VirtualReader::Ptr reader = std::make_shared<VirtualReader>();
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
int VirtualReader::connect()
{
   int ret = 0;
   return (ret);
}
#undef WHERE


#define WHERE "VirtualReader::disconnect()"
int VirtualReader::disconnect()
{
   int rv = 0;
   return (rv);
}
#undef WHERE


