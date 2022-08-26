#ifndef Card_hpp
#define Card_hpp

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <memory>
#include "log.hpp"
#include "CardErrors.h"

#define FORMAT_HEX				1
#define FORMAT_RADIX64			2
#define FORMAT_RAW            3

#define CARD_TYPE_VIRTUALCARD   0
#define CARD_TYPE_BEID			  1
#define CARD_TYPE_CARDOS        2
#define CARD_TYPE_CARDOS44      3
#define CARD_TYPE_ADVCARD       4
#define CARD_TYPE_PKCS15        5
#define CARD_TYPE_PKCS11        6
#define CARD_TYPE_ISABEL        7
#define CARD_TYPE_UNKNOWN      -1

#define IDFILE             1
#define IDSIGFILE          2
#define ADDRESSFILE        3
#define ADDRESSSIGFILE     4
#define PHOTOFILE          5
#define RRNCERT            6
#define CACERT             7
#define ROOTCERT           8
#define AUTHCERT           9
#define SIGNCERT          10


class Card
{
public:
   Card() {};
   virtual ~Card(){};

   void setAtr(std::string atr);

   virtual int isCardSupported(void) { return 1; };
   virtual std::string strType() { return "NOT DEFINED"; };
   virtual int type() { return -1; };
   virtual long readCertificate(int format, int type, std::vector<char> &cert) = 0;
   virtual long readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates){ return -1; };
   virtual long readCertificateChain(int format, unsigned char *cert, int l_cert, std::vector<std::vector<char>> &subCerts, std::vector<char> &root){ return -1; };
   virtual long selectKey(int type, unsigned char* cert = 0, size_t l_cert = 0) = 0;
   virtual long logon(int l_pin, char *pin) = 0;
   virtual long logoff() = 0;
   virtual long sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw) = 0;
   virtual std::vector<char> getFile(int format, std::string fileType) { return std::vector<char>(); };

   virtual std::string* valueForKey(std::string* key) {return nullptr;};
   virtual long selectFile(unsigned char *file, int l_file){ return -1; };
   virtual long readFile2(unsigned int offset, int* p_len, unsigned char* p_out){ return -1; };

   long getFile(unsigned char *file, int l_file, int* l_out, unsigned char* p_out);
   virtual int list_objects(std::ostringstream& buf) { return (-1); };

   std::shared_ptr<class CardReader> reader;

private:
   std::string     atr;
};

#endif /* Card_hpp */


