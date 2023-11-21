#pragma once

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
   void beginTransaction() override {};
   void endTransaction() override {};
public:
   VirtualReader();
   virtual ~VirtualReader();
   static int listReaders(std::vector<std::shared_ptr<CardReader>>& readers);
   long connect() override;
   void disconnect() override;
   bool isPinPad() override{ return 0; };
   CardAPDUResponse apdu(const CardAPDU& apdu) override { throw NotImplementedException("VirtualReader apdu2"); };
   void verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, uint16_t PINMaxExtraDigit, const unsigned char pinAPDU[], size_t l_pinAPDU, uint16_t*sw) override { throw NotImplementedException("VirtualReader verify_pinpad"); };
};

#endif /* VirtualReader_hpp */
