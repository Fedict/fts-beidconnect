#ifndef CardReader_hpp
#define CardReader_hpp

#include <iostream>
//#include "general.h"
#include "Card.hpp"
#include <vector>
#include <memory>
#include <fstream>
#include "log.hpp"
#include "CardErrors.h"

#define MAX_ATR_LENGTH			128
#define MAX_LABEL_LENGTH		32
#define MAX_READERS				10


#define FORMAT_HEX				1
#define FORMAT_RADIX64			2

using namespace std;

class CardReader
{
public:
   CardReader(){};
   virtual ~CardReader(){};
   typedef std::shared_ptr <CardReader> Ptr;
   virtual int connect() = 0;
   virtual int disconnect() = 0;
   virtual bool isPinPad() = 0;
   virtual int beginTransaction() = 0;
   virtual int endTransaction() = 0;
   virtual int apdu(const unsigned char *apdu, unsigned int l_apdu, unsigned char *out, int *l_out, int *sw) = 0;
   virtual int verify_pinpad(unsigned char format, unsigned char PINBlock, unsigned char PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], int l_pinAPDU, int *sw) = 0;
 
   std::string name;
   std::string atr;
   int language;
};


#endif /* CardReader_hpp */
