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
      BEIDCard(CardReader::Ptr rdr){ reader = rdr; };
      virtual ~BEIDCard(){};
   
      std::string strType() override { return "BEID"; };
      int type() override { return CARD_TYPE_BEID; };
      int readCertificate(int format, int type, std::vector<char> &cert) override;
      int readCertificateChain(int format, unsigned char *cert, int l_cert, std::vector<std::vector<char>>  &subCerts, std::vector<char> &root) override;

      int readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates) override;
      int selectKey(int type, unsigned char* cert = 0, int l_cert = 0) override;
      int logon(int l_pin, char *pin) override;
      int logoff() override;
      int sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw) override;

      std::vector<char> getFile(int format, std::string fileType) override;
      std::string* valueForKey(std::string* key) override;
      int selectFile(unsigned char *file, int l_file) override;
      int readFile2(unsigned int offset, int* p_len, unsigned char* p_out) override;

   private:
      int currentSelectedKeyType;
      int currentSelectedKeyLength;
};


#endif /* BEIDCard_hpp */
