//
//  SignRequestHandler.cpp
//  BeIDConnect
//
//  Created by Vital Schonkeren on 02/03/2020.
//
#include "SignRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "util.h"
#include "general.h"
#include "hash.h"
#include "crypto.h"

using boost::property_tree::ptree;

#define WHERE "SignRequestHandler::process()"
std::string SignRequestHandler::process()
{
   ptree response;
   long lasterror = 0;
   
   ReaderList readerList;
   int algo = 0;
   unsigned char signature[512];
   unsigned int l_signature = 512;
   bool loggedON = false;
   
   std::stringstream ss(ssRequest->str());
   boost::property_tree::ptree pt;
   boost::property_tree::read_json(ss, pt);
   std::string certif = pt.get<std::string>("cert");
   std::string operation = pt.get<std::string>("operation");
   std::string pin = pt.get<std::string>("pin");
   std::string digest = pt.get<std::string>("digest");
   std::string digestAlgo = pt.get<std::string>("algo");
   
   int l_cert = base64decode_len((unsigned char*) certif.c_str());
   unsigned char *cert = (unsigned char*) malloc(l_cert);
   if (cert == 0) {
      log_error("%s mem alloc failed for cert (%d)", WHERE, l_cert);
   }
   l_cert = base64decode((unsigned char*)certif.c_str(), cert);
   
   int l_hash = base64decode_len((unsigned char*) digest.c_str());
   unsigned char *hash = (unsigned char*) malloc(l_hash);
   if (hash == 0) {
      log_error("%s mem alloc failed for digest (%d)", WHERE, l_hash);
   }
   l_hash = base64decode((unsigned char*)digest.c_str(), hash);
   
   std::shared_ptr<CardReader> reader = readerList.getReaderByIndex(0);
   if (reader == nullptr) {
      lasterror = E_SRC_NO_READERS_FOUND;
   }
   else {
      ptree readerInfos;
      for (auto& reader:readerList.readers) {
         
         if (reader->atr == "") {
            lasterror = E_SRC_NO_CARD;
            continue;
         }
         
         lasterror = reader->connect();
         if (lasterror)
         {
            log_error("%s: E: reader->connect(%s) returned %08X", WHERE, reader->name.c_str(), lasterror);
            continue;
         }
         
         std::shared_ptr<Card> card = CardFactory::createCard(reader);
         if (card == nullptr) {
            lasterror = CARD_TYPE_UNKNOWN;
            continue; //card not supported in this reader, try next reader
         }
         
         if (operation == "SIGN") {
            lasterror = card->selectKey(CERT_TYPE_NONREP, cert, l_cert);
         }
         else {
            lasterror = card->selectKey(CERT_TYPE_AUTH, cert, l_cert);
         }
         if (lasterror) {
            log_error("%s: E: card->selectKey returned %d (0x%0X)", WHERE, lasterror, lasterror);
            reader->disconnect();
            continue; //try next reader to find chain
         }
  
         if (pin == "null") {
            pin = "";
         }
         //if we get here, we successfully selected the key or found the signing certificate
         lasterror = card->logon((int)pin.size(), (char*) pin.c_str());
         if (lasterror) {
            log_info("%s: E: card->logon returned %d (0x%0X)", WHERE, lasterror, lasterror);
            break;
         }
         loggedON = true;
         
         algo = algo2str((char*)digestAlgo.c_str());
         
         int sw = 0;
         lasterror = card->sign(hash, l_hash, algo, signature, &l_signature, &sw);
         if (lasterror)
         {
            log_error( "%s: E: card->sign returned %08X", WHERE, lasterror);
            if (loggedON) {
               card->logoff();
            }
            break;
         }
         
         if (loggedON) {
            card->logoff();
         }
         
         //verify the signature if we have a certificate
         if ((l_cert > 0) && (cert != 0)) {
            lasterror = verifySignature(hash, l_hash, "sha256", cert, l_cert, signature, l_signature);
            if (lasterror) {
               //since we verify the signature based on the certificate that was used to do the signature,
               //there can only be two reasons to fail a verification:
               //1. or the certificate linked to the private key on the card does not belong to the private key
               //2. or the reader calculates a faux signature, maybe because the reader does not support extended APDU commands
               //where the signature is incomplete!
               lasterror = E_SRC_SIGNATURE_FAILED;
            }
         }
         break;
      }
   }
      
   switch (lasterror) {
      case (int) E_SRC_NO_READERS_FOUND:
         response.put("result", "no_reader");
         break;
      case (int) E_SRC_SIGNATURE_FAILED:
         response.put("result", "signature_failed");
         break;
      case (int) E_SRC_NO_CARD:
         response.put("result", "no_card");
         break;
      case E_PIN_CANCELLED:
         response.put("result", "cancel");
         break;
      case E_PIN_BLOCKED:
         response.put("result", "card_blocked");
         break;
      case E_PIN_LENGTH:
         response.put("result", "pin_length");
         break;
      case E_PIN_TOO_SHORT:
         response.put("result", "pin_too_short");
         break;
      case E_PIN_TOO_LONG:
         response.put("result", "pin_too_long");
         break;
      case E_PIN_TIMEOUT:
         response.put("result", "pin_timeout");
         break;
      case E_PIN_INCORRECT:
         response.put("result", "pin_incorrect");
         break;
      case E_PIN_3_ATTEMPTS:
         response.put("result", "pin_3_attempts_left");
         break;
      case E_PIN_2_ATTEMPTS:
         response.put("result", "pin_2_attempts_left");
         break;
      case E_PIN_1_ATTEMPT:
         response.put("result", "pin_1_attempt_left");
         break;
      case 0:
      {
         int l_signB64 =  base64encode_len(l_signature);
         unsigned char* signB64 = (unsigned char*) malloc(l_signB64+1);
         if (signB64 == NULL) {
            log_error("%s mem alloc failed for digest (%d)", WHERE, l_hash);
            //lasterror = E_ALLOC;
         }
         base64encode(signature, l_signature, signB64);
         signB64[l_signB64] = 0;

         response.put("signature", signB64);
         response.put("result", "OK");
         break;
      }
      default:
         log_error("%s E: signPKCS1() returned %0X", WHERE, lasterror);
         response.put("result", "general_error");
         
         std::string report = str(boost::format("signPKCS1 returned 0x%0X") % lasterror);
         response.put("report", report);
   }
   
   
   if (cert) {
      free (cert);
   }
   if (hash) {
      free (hash);
   }
   
   post_process(response);
   std::stringstream streamResponse;
   boost::property_tree::write_json(streamResponse, response, false);
   
   //log_info(streamResponse.str().c_str());
   return streamResponse.str();
}
#undef WHERE
