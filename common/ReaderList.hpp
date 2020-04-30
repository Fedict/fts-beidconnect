#include "CardReader.hpp"
#include <iostream>
#include <vector>
#include <memory>

#ifndef ReaderList_hpp
#define ReaderList_hpp

#include <stdio.h>

using namespace std;

typedef int type;

class ReaderList
{
public:
   typedef std::unique_ptr<ReaderList> Ptr;
   ReaderList(){};
   virtual ~ReaderList();
   CardReader::Ptr getReaderByName(string readername);
   CardReader::Ptr getReaderByIndex(unsigned int index);
   CardReader::Ptr getFirstReaderWithSupportedCardType(int type[], int size);
   vector<CardReader::Ptr> readers;
private:
   int listReaders();
   CardReader::Ptr returnFirstReaderWithSupportedCardType(int type[], int size);
};

#endif /* ReaderList_hpp */
