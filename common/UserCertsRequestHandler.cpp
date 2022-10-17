//
//  UserCertsRequestHandler.cpp
//  BeIDConnect
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#include "UserCertsRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "general.h"

using boost::property_tree::ptree;

#define WHERE "UserCertsRequestHandler::process()"
std::string UserCertsRequestHandler::process()
{
   ptree response;
   
   ReaderList readerList;
   size_t count = readerList.readers.size();
   int countSupportedCards = 0;
   int countUnsupportedCards = 0;
   int countErrors = 0;
   int certType = 0;
  
   std::stringstream ss(ssRequest->str());
   boost::property_tree::ptree pt;

   boost::property_tree::read_json(ss, pt);
   if (pt.get_optional<std::string>("keyusage").is_initialized()) {
      std::string keyusage = pt.get<std::string>("keyusage");
      if (keyusage.compare("NONREPUDIATION") == 0) {
         certType = CERT_TYPE_NONREP;
      }
      else if (keyusage.compare("DIGITALSIGNATURE") == 0) {
         certType = CERT_TYPE_AUTH;
      }
      //else 0, return all usercerts
   }

   if (count == 0) {
      response.put("result", "no_reader");
   }
   else {
      long status = 0;
      ptree readerInfos;
      for (int i = 0; i < (int)count; i++) {
         
         std::shared_ptr<CardReader> reader = readerList.getReaderByIndex(i);
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
         
         std::shared_ptr<Card> card = CardFactory::createCard(reader);
         if (card == nullptr) {
            countUnsupportedCards++;
            continue; //card not supported in this reader, try next reader
         }
         
         //add usercertificates to list
         std::vector<std::vector<char>> certificates;
         status = card->readUserCertificates(FORMAT_RADIX64, certType, certificates);
         if (status) {
            countErrors++;
            log_error( "%s: E: card->readUserCertificates() returned %08X", WHERE, status);
            continue;
         }
         countSupportedCards++;
         ptree readerInfo;
         readerInfo.put("ReaderName", reader->name);
         if (reader->isPinPad()) {
            readerInfo.put("ReaderType", "pinpad");
         }
         else {
            readerInfo.put("ReaderType", "standard");
         }
         readerInfo.put("cardType", card->strType());
         ptree certList;
         for (auto& cert:certificates) {
            ptree certEntry;
            certEntry.put("", std::string(cert.data(), cert.size()));
            certList.push_back(std::make_pair("", certEntry));
         }
         readerInfo.add_child("certificates", certList);
         readerInfos.push_back(std::make_pair("", readerInfo));
         for (auto& cert:certificates) {
            cert.clear();
         }
      }
      response.add_child("Readers", readerInfos);
      
      if (countSupportedCards > 0) {
         response.put("result", "OK");
      }
      else {
         //errors and unsupported cards result in nop_card
         response.put("result", "no_card");
         
         if (countUnsupportedCards > 0) {
            response.put("report", "card_type_unsupported");
         }
      }
   }
   
   post_process(response);
   std::stringstream streamResponse;
   boost::property_tree::write_json(streamResponse, response, false);
   return streamResponse.str();
}
#undef WHERE
