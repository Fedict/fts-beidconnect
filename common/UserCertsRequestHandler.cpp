//
//  VersionRequestHandler.cpp
//  eIDLink
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

using boost::property_tree::ptree;

std::string UserCertsRequestHandler::process()
{
   std::stringstream response;
   ptree pt;
      
   int supportedCardTypes[] = { CARD_TYPE_BEID, /*CARD_TYPE_PKCS15, CARD_TYPE_PKCS11 */};
   Card::Ptr card;
   ReaderList readerList;
   CardReader::Ptr reader = readerList.getFirstReaderWithSupportedCardType(supportedCardTypes, sizeof(supportedCardTypes)/sizeof(int));
   if (reader == nullptr) {
      if (readerList.readers.size() == 0) {
         pt.put("result", "no_reader");
      }
      else {
         pt.put("result", "no_card");
      }
   }
   else {
      pt.put("report", reader->name);
      
      card = CardFactory::createCard(reader);
      if (card == nullptr) {
         pt.put("result", "no_card");
         pt.put("report", "card_type_unsupported");
      }
      else {
         pt.put("cardtype", card->strType());
         pt.put("result","OK");
      }
      reader->disconnect();
   }
   
   //read certificates and return in a json array
   
   
   boost::property_tree::write_json(response, pt, false);
   return response.str();
}
