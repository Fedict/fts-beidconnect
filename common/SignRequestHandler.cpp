//
//  SignRequestHandler.cpp
//  eIDLink
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#include "SignRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "util.h"

using boost::property_tree::ptree;

#define WHERE "SignRequestHandler::process()"
std::string SignRequestHandler::process()
{
   ptree response;
   
   Card::Ptr card;
   ReaderList readerList;
   int countUnsupportedCards = 0;
   int countErrors = 0;
   
   std::stringstream ss(ssRequest->str());
   boost::property_tree::ptree pt;
   boost::property_tree::read_json(ss, pt);
   std::string certif = pt.get<std::string>("cert");
   int l_cert = base64decode_len((unsigned char*) certif.c_str());
   unsigned char *cert = (unsigned char*) malloc(l_cert);
   if (cert == 0) {
      log_error("%s mem alloc failed for cert (%d)", WHERE, l_cert);
   }
   l_cert = base64decode((unsigned char*)certif.c_str(), cert);
   
   CardReader::Ptr reader = readerList.getReaderByIndex(0);
//   size_t count = readerList.readers.size();
   if (reader == nullptr) {
      response.put("result", "no_reader");
   }
   else {
      int status = 0;
      ptree readerInfos;
      //for (int i = 0; i < (int)count; i++) {
      for (auto& reader:readerList.readers) {
            //reader = readerList.getReaderByIndex(i);
         if (reader->atr == "") {
            continue;
         }
         
         status = reader->connect();
         if (status)
         {
            countErrors++;
            log_error("%s: E: reader->connect(%s) returned %08X", WHERE, reader->name.c_str(), status);
            continue;
         }
         
         Card::Ptr card = CardFactory::createCard(reader);
         if (card == nullptr) {
            countUnsupportedCards++;
            continue; //card not supported in this reader, try next reader
         }

         
      
         break;
      }
      
//      if (chainFound > 0) {
//         response.put("result", "OK");
//      }
//      else {
//         //errors and unsupported cards result in no_card
//         response.put("result", "no_card");
//
//         if (countUnsupportedCards > 0) {
//            response.put("report", "card_type_unsupported");
//         }
//      }
   }
   
   if (cert) {
      free (cert);
   }
   post_process(response);
   std::stringstream streamResponse;
   boost::property_tree::write_json(streamResponse, response, false);
   
   //log_info(streamResponse.str().c_str());
   return streamResponse.str();
}
#undef WHERE
