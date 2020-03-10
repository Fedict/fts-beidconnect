#ifndef VirtualReader_hpp
#define VirtualReader_hpp

#include "CardReader.hpp"
#include <vector>
#include <memory>
#ifdef WIN32
#include "windows.h"
#endif


class VirtualReader: public CardReader
{
public:
   VirtualReader();
   virtual ~VirtualReader();
   typedef std::shared_ptr<VirtualReader> Ptr;
   static int listReaders(std::vector<CardReader::Ptr> &readers);
   int open() override;
   int close() override;
   int beginTransaction() override { return 0; };
   int endTransaction() override{ return 0; };
   int connect() override;
   int disconnect() override;
   bool isPinPad() override{ return 0; };
   int apdu(const unsigned char *apdu, unsigned int l_apdu, unsigned char *out, int *l_out, int *sw) override{ return -1; };
   int verify_pinpad(unsigned char format, unsigned char PINBlock, unsigned char PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], int l_pinAPDU, int *sw)override { return -1; };
};

#endif /* VirtualReader_hpp */
