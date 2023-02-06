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
      long readCertificateChain(int format, const unsigned char *cert, size_t l_cert, std::vector<std::shared_ptr<const CardFile>> &subCerts, std::vector<char> &root) override;

      long selectKey(CardKeys type, unsigned char* cert = 0, size_t l_cert = 0) override;
      long logon(int l_pin, char *pin) override;
      long logoff() override;
      long sign(const unsigned char* in, size_t l_in, int hashAlgo, unsigned char *out, size_t* l_out, int *sw) override;

      std::shared_ptr<const CardFile> getFile(CardFiles fileType) override;

      //std::string* valueForKey(std::string* key) override;
      long selectFile(const unsigned char *file, size_t l_file) override;
      std::shared_ptr<const CardFile> readFile3(CardFileReadOptimization optimization = CardFileReadOptimization::None) override;

      const std::map<std::string, std::string> getCardData() override;

   private:
      int currentSelectedKeyType;
      size_t currentSelectedKeyLength;
      bool cacheCardDataLoaded = false;
      std::map<std::string, std::string> cacheCardData;
};


#endif /* BEIDCard_hpp */
