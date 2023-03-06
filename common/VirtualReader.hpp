#ifndef VirtualReader_hpp
#define VirtualReader_hpp

#include "CardReader.hpp"
#include <vector>
#include <memory>
#ifdef _WIN32
#include "windows.h"
#endif


class VirtualReader: public CardReader
{
   long beginTransaction() override { return 0; };
   long endTransaction() override{ return 0; };
public:
   VirtualReader();
   virtual ~VirtualReader();
   static int listReaders(std::vector<std::shared_ptr<CardReader>>& readers);
   long connect() override;
   long disconnect() override;
   bool isPinPad() override{ return 0; };
   long apdu(const unsigned char* apdu, size_t l_apdu, unsigned char* out, size_t* l_out, int* sw) override { throw NotImplementedException("VirtualReader apdu"); };
   CardAPDUResponse apdu2(const CardAPDU& apdu) override { throw NotImplementedException("VirtualReader apdu2"); };
   long verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], size_t l_pinAPDU, int *sw)override { throw NotImplementedException("VirtualReader verify_pinpad"); };
};

#endif /* VirtualReader_hpp */
