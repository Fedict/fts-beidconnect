#pragma once

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

#define CARD_TYPE_VIRTUALCARD   0
#define CARD_TYPE_BEID			1
#define CARD_TYPE_CARDOS        2
#define CARD_TYPE_CARDOS44      3
#define CARD_TYPE_ADVCARD       4
#define CARD_TYPE_PKCS15        5
#define CARD_TYPE_PKCS11        6
#define CARD_TYPE_ISABEL        7
#define CARD_TYPE_UNKNOWN      -1

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
   virtual void readCertificateChain(std::vector<std::shared_ptr<const CardFile>> &subCerts, std::shared_ptr<const CardFile>& rootCert){ throw NotImplementedException("CardFile readCertificateChain"); };
   virtual void selectKey(CardKeys type, const std::vector<unsigned char>& cert) = 0;
   virtual void logon(int l_pin, char *pin) = 0;
   virtual void logoff() = 0;
   virtual long sign(const std::vector<unsigned char>& in, int hashAlgo, unsigned char *out, size_t*l_out, int *sw) = 0;
   virtual std::shared_ptr<const CardFile> getFile(CardFiles fileType) { throw NotImplementedException("CardFile getFile"); };

   virtual const std::map<std::string, std::string> getCardData() { return std::map<std::string, std::string>(); }
protected:
   std::shared_ptr<class CardReader> reader;

   virtual void selectFile(const unsigned char *file, size_t l_file){ throw NotImplementedException("CardFile selectFile"); };
   virtual std::shared_ptr<const CardFile> readFile(CardFileReadOptimization optimization = CardFileReadOptimization::None) { throw NotImplementedException("CardFile readFile3"); };
};

#endif /* Card_hpp */


