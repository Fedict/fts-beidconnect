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
   
      std::string strType() override { return "BEID"; };
      int type() override { return CARD_TYPE_BEID; };
      long readCertificate(int format, int type, std::vector<char> &cert) override;
      long readCertificateChain(int format, unsigned char *cert, int l_cert, std::vector<std::vector<char>>  &subCerts, std::vector<char> &root) override;

      long readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates) override;
      long selectKey(int type, unsigned char* cert = 0, size_t l_cert = 0) override;
      long logon(int l_pin, char *pin) override;
      long logoff() override;
      long sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw) override;

      std::vector<char> getFile(int format, std::string fileType) override;
      std::string* valueForKey(std::string* key) override;
      long selectFile(unsigned char *file, int l_file) override;
      long readFile2(unsigned int offset, int* p_len, unsigned char* p_out) override;

   private:
      int currentSelectedKeyType;
      unsigned int currentSelectedKeyLength;
};


#endif /* BEIDCard_hpp */
