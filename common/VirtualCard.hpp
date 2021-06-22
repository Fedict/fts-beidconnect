#ifndef VirtualCard_hpp
#define VirtualCard_hpp

#include <stdio.h>
#include <iostream>
#include <memory>
#include "Card.hpp"
#include "CardReader.hpp"

class VirtualCard: public Card
{
   public:
      VirtualCard(){};
      VirtualCard(CardReader::Ptr rdr){ reader = rdr; };
      virtual ~VirtualCard(){};
   
      std::string strType() override;
      int type() override;
      int readCertificate(int format, int type, std::vector<char> &cert) override;
      int readCertificateChain(int format, unsigned char *cert, int l_cert, std::vector<std::vector<char>>  &subCerts, std::vector<char> &root) override;

      int readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates) override;
      int selectKey(int type, unsigned char* cert = 0, int l_cert = 0) override;
      int logon(int l_pin, char *pin) override;
      int logoff() override;
      int sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw) override;
      std::vector<char> getFile(int format, std::string fileType) override;
};


#endif /* VirtualCard_hpp */
