#pragma once

#ifndef VirtualCard_hpp
#define VirtualCard_hpp

#include <stdio.h>
#include <iostream>
#include <memory>
#include "Card.hpp"
#include "CardReader.hpp"
#include "SCardException.h"

class VirtualCard: public Card
{
   public:
      VirtualCard(){};
      VirtualCard(const std::shared_ptr<CardReader>& rdr){ reader = rdr; };
      virtual ~VirtualCard(){};
   
      std::string strType() const override;
      int type() const override;
      void readCertificateChain(std::vector<std::shared_ptr<const CardFile>> &subCerts, std::shared_ptr<const CardFile>& rootCert) override;

      void selectKey(CardKeys type, const std::vector<unsigned char>& cert) override;
      void logon(int l_pin, char *pin) override;
      void logoff() override;
      long sign(const std::vector<unsigned char>& in, int hashAlgo, unsigned char *out, size_t*l_out, int *sw) override;

      std::shared_ptr<const CardFile> getFile(CardFiles fileType) override;
};


#endif /* VirtualCard_hpp */
