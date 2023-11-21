#pragma once

#ifndef ReaderList_hpp
#define ReaderList_hpp

#include "CardReader.hpp"
#include <iostream>
#include <vector>
#include <memory>

#include <stdio.h>

using namespace std;

typedef int type;

class ReaderList
{
public:
   ReaderList(){ listReaders(); };
   virtual ~ReaderList();
   std::shared_ptr<CardReader> getReaderByName(string readername);
   std::shared_ptr<CardReader> getReaderByIndex(unsigned int index);
   std::shared_ptr<CardReader> getFirstReaderWithSupportedCardType(int type[], int size);
   vector<std::shared_ptr<CardReader>> readers;
private:
   void listReaders();
   std::shared_ptr<CardReader> returnFirstReaderWithSupportedCardType(int type[], int size);
};

#endif /* ReaderList_hpp */
