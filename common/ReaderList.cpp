#include "ReaderList.hpp"
#include "SCard.hpp"
#include "VirtualReader.hpp"
#include "CardFactory.hpp"
#include <algorithm>

ReaderList::~ReaderList()
{
}

int ReaderList::listReaders()
{
   readers.clear();
   int ret = SCard::listReaders(readers);
   // PKCS11 has its own listreaders function, since they don't pass through winscard lib necessarily
   //  ret = P11Slot::listReaders(readers);
   
#ifdef VIRTUAL_CARDS
   ret = VirtualReader::listReaders(readers);
#endif
   
   //if no readers are found, raise an exception
   return ret;
};

std::shared_ptr<CardReader> ReaderList::getReaderByName(string readername)
{
   if (readers.size() == 0) {
      listReaders();
   }
   for (auto const& reader:readers) {
       if(reader->name.compare(readername) == 0)
          return reader;
   }
   
   return nullptr;
};

std::shared_ptr<CardReader> ReaderList::getReaderByIndex(unsigned int index)
{
   if (readers.size() == 0) {
      listReaders();
   }

   if (index < readers.size())
      return readers[index];
   else
      return nullptr;
};

std::shared_ptr<CardReader> ReaderList::getFirstReaderWithSupportedCardType(int supportedCardTypes[], int size)
{
   int ret;
   std::shared_ptr<CardReader> reader;
   
   readers.clear();

   //we call each type separately here to avoid searching to much, we stop as soon as a supported card has been found
   ret = SCard::listReaders(readers);
   if ( (ret == 0) && ( (reader = returnFirstReaderWithSupportedCardType(supportedCardTypes, size)) ) ) {
      return reader;
   }

//  ret = P11Slot::listReaders(readers);
//   if ( (ret == 0) && ( (reader = returnFirstReaderWithSupportedCardType(supportedCardTypes, size)) ) ) {
//      return reader;
//   }
   
#ifdef VIRTUAL_CARDS
   ret = VirtualReader::listReaders(readers);
   if ( (ret == 0) && ( (reader = returnFirstReaderWithSupportedCardType(supportedCardTypes, size)) ) ) {
      return reader;
   }
#endif
   
   return nullptr;
}

std::shared_ptr<CardReader> ReaderList::returnFirstReaderWithSupportedCardType(int supportedCardTypes[], int size)
{
   for (auto const& reader:readers) {
      std::string strType;
      if (reader->atr == "") {
         continue;
      }

      if (reader->connect())
         continue;

      std::shared_ptr<Card> card = CardFactory::createCard(reader);
      if (card) {
         int *p = std::find(supportedCardTypes, supportedCardTypes+size, card->type());
         if (p != supportedCardTypes+size)
            return reader;
      }
      else {
         continue;
      }
   }
   return nullptr;
}
