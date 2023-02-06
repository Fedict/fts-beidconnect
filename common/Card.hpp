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
	mutable std::string Base64Format;
	mutable std::vector<unsigned char> RawFormat;
public:
	CardFile(const std::string& Base64Format) : Base64Format(Base64Format) {}
	CardFile(const std::vector<unsigned char>& RawFormat) : RawFormat(RawFormat) {}
	CardFile(){}

	inline const std::string& getBase64() const
	{
		if (Base64Format.empty() && RawFormat.size() > 0)
		{
			Base64Format = rawToBase64(RawFormat);
		}
		return Base64Format;
	}
	inline const std::vector<unsigned char>& getRaw() const
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
enum class CardFiles
{
	Id,
	Id_sig,
	Address,
	Address_sig,
	Photo,
	Rrncert,
	Authcert,
	Signcert,
	Cacert,
	Rootcert
};
enum class CardKeys
{
	NonRep,
	Auth
};

class Card
{
public:
   Card() {};
   virtual ~Card(){};

   virtual std::string strType() const { return "NOT DEFINED"; };
   virtual int type() const { return -1; };
   virtual long readCertificateChain(int format, const unsigned char *cert, size_t l_cert, std::vector<std::shared_ptr<const CardFile>> &subCerts, std::vector<char> &root){ throw NotImplementedException("CardFile readCertificateChain"); };
   virtual long selectKey(CardKeys type, unsigned char* cert = 0, size_t l_cert = 0) = 0;
   virtual long logon(int l_pin, char *pin) = 0;
   virtual long logoff() = 0;
   virtual long sign(const unsigned char* in, size_t l_in, int hashAlgo, unsigned char *out, size_t*l_out, int *sw) = 0;
   virtual std::shared_ptr<const CardFile> getFile(CardFiles fileType) { throw NotImplementedException("CardFile getFile"); };

   virtual const std::map<std::string, std::string> getCardData() { return std::map<std::string, std::string>(); }
protected:
   std::shared_ptr<class CardReader> reader;

   virtual long selectFile(const unsigned char *file, size_t l_file){ throw NotImplementedException("CardFile selectFile"); };
   virtual std::shared_ptr<const CardFile> readFile3(CardFileReadOptimization optimization = CardFileReadOptimization::None) { throw NotImplementedException("CardFile readFile3"); };
};

#endif /* Card_hpp */


