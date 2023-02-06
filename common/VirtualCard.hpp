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
      long readCertificateChain(int format, const unsigned char *cert, size_t l_cert, std::vector<std::shared_ptr<const CardFile>> &subCerts, std::vector<char> &root) override;

      long selectKey(CardKeys type, unsigned char* cert = 0, size_t l_cert = 0) override;
      long logon(int l_pin, char *pin) override;
      long logoff() override;
      long sign(const unsigned char* in, size_t l_in, int hashAlgo, unsigned char *out, size_t*l_out, int *sw) override;

      std::shared_ptr<const CardFile> getFile(CardFiles fileType) override;
};


#endif /* VirtualCard_hpp */
