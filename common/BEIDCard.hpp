#pragma once

#ifndef BEIDCard_hpp
#define BEIDCard_hpp

#include <stdio.h>
#include <iostream>
#include <memory>
#include "Card.hpp"
#include "CardReader.hpp"

class BEIDCard: public Card
{
   public:
      BEIDCard(){};
      BEIDCard(std::shared_ptr<CardReader> rdr){ reader = rdr; };
      virtual ~BEIDCard(){};
   
      std::string strType() const override { return "BEID"; };
      int type() const override { return CARD_TYPE_BEID; };
      void readCertificateChain(std::vector<std::shared_ptr<const CardFile>> &subCerts, std::shared_ptr<const CardFile>& rootCert) override;

      void selectKey(CardKeys type, const std::vector<unsigned char>& cert) override;
      void logon(int l_pin, char *pin) override;
      void logoff() override;
      long sign(const std::vector<unsigned char>& in, int hashAlgo, unsigned char *out, size_t* l_out, int *sw) override;

      std::shared_ptr<const CardFile> getFile(CardFiles fileType) override;

      void selectFile(const unsigned char *file, size_t l_file) override;
      std::shared_ptr<const CardFile> readFile(CardFileReadOptimization optimization = CardFileReadOptimization::None) override;

      const std::map<std::string, std::string> getCardData() override;

   private:
      int currentSelectedKeyType = X509_KEYTYPE_RSA;
      size_t currentSelectedKeyLength = 0;
      bool cacheCardDataLoaded = false;
      std::map<std::string, std::string> cacheCardData;
};


#endif /* BEIDCard_hpp */
