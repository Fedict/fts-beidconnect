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
   
      std::string strType() override;
      int type() override;
      long readCertificate(int format, int type, std::vector<char> &cert) override;
      long readCertificateChain(int format, unsigned char *cert, size_t l_cert, std::vector<std::vector<char>>  &subCerts, std::vector<char> &root) override;

      long readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates) override;
      long selectKey(int type, unsigned char* cert = 0, size_t l_cert = 0) override;
      long logon(int l_pin, char *pin) override;
      long logoff() override;
      long sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw) override;
      CardFile getFile(const std::string& fileType) override;
};


#endif /* VirtualCard_hpp */
