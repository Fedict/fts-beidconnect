#ifndef Card_hpp
#define Card_hpp

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <memory>
#include <map>
#include "log.hpp"
#include "CardErrors.h"
#include "SCardException.h"
#include "util.h"

#define FORMAT_HEX				1
#define FORMAT_RADIX64			2
#define FORMAT_RAW            3

#define CARD_TYPE_VIRTUALCARD   0
#define CARD_TYPE_BEID			1
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

class CardFile
{
	std::string Base64Format;
	std::vector<unsigned char> RawFormat;
public:
	CardFile(const std::string& Base64Format) : Base64Format(Base64Format) {}
	CardFile(const std::vector<unsigned char>& RawFormat) : RawFormat(RawFormat) {}

	inline std::string getBase64()
	{
		if (Base64Format.empty() && RawFormat.size() > 0)
		{
			Base64Format = rawToBase64(RawFormat);
		}
		return Base64Format;
	}
	inline const std::vector<unsigned char> getRaw()
	{
		if (RawFormat.size() == 0 && !Base64Format.empty() )
		{
			RawFormat = base64ToRaw(Base64Format);
		}
		return RawFormat;
	}
};

enum class CardFileReadOptimization
{
	None,
	DEREncodedCertificate
};

class Card
{
public:
   Card() {};
   virtual ~Card(){};

   virtual std::string strType() { return "NOT DEFINED"; };
   virtual int type() { return -1; };
   virtual long readCertificate(int format, int type, std::vector<char> &cert) = 0;
   virtual long readUserCertificates(int format, int certType, std::vector<std::vector<char>> &certificates){ throw NotImplementedException("CardFile readUserCertificates"); };
   virtual long readCertificateChain(int format, unsigned char *cert, size_t l_cert, std::vector<std::vector<char>> &subCerts, std::vector<char> &root){ throw NotImplementedException("CardFile readCertificateChain"); };
   virtual long selectKey(int type, unsigned char* cert = 0, size_t l_cert = 0) = 0;
   virtual long logon(int l_pin, char *pin) = 0;
   virtual long logoff() = 0;
   virtual long sign(unsigned char* in, unsigned int l_in, int hashAlgo, unsigned char *out, unsigned int *l_out, int *sw) = 0;
   virtual CardFile getFile(const std::string& fileType) { throw NotImplementedException("CardFile getFile"); };
   virtual CardFile getFile3(const std::string& fileType) { throw NotImplementedException("CardFile getFile3"); };

   virtual long selectFile(unsigned char *file, size_t l_file){ throw NotImplementedException("CardFile selectFile"); };
   virtual long readFile2(unsigned int offset, size_t* p_len, unsigned char* p_out){ throw NotImplementedException("CardFile readFile2"); };
   virtual CardFile readFile3(CardFileReadOptimization optimization = CardFileReadOptimization::None) { throw NotImplementedException("CardFile readFile3"); };

   virtual const std::map<std::string, std::string> getCardData() { return std::map<std::string, std::string>(); }

   long getFile(unsigned char *file, size_t l_file, size_t* l_out, unsigned char* p_out);
protected:
   std::shared_ptr<class CardReader> reader;
};

#endif /* Card_hpp */


