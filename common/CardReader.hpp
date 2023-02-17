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
    // To ensure transaction begin/end, use the ScopedCardTransaction class
    friend class ScopedCardTransaction;
    virtual long beginTransaction() = 0;
    virtual long endTransaction() = 0;
public:
    CardReader() {};
    virtual ~CardReader() {};
    virtual long connect() = 0;
    virtual long disconnect() = 0;
    virtual bool isPinPad() = 0;
    virtual long apdu(const unsigned char* apdu, size_t l_apdu, unsigned char* out, size_t* l_out, int* sw) = 0;
    virtual long verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], size_t l_pinAPDU, int* sw) = 0;

    std::string name;
    std::string atr;
    int language;
};

/// <summary>
/// Transaction helper class. Try to start a transaction. When out of the calling scope, a successfull transaction is ended.
/// </summary>
class ScopedCardTransaction
{
    std::shared_ptr<CardReader> reader;
    long beginTransactionRC;
public:
    ScopedCardTransaction(const std::shared_ptr<CardReader>& reader)
    {
        this->reader = reader;
        beginTransactionRC = reader->beginTransaction();
    }
    ~ScopedCardTransaction()
    {
        if (beginTransactionRC)
            reader->endTransaction();
    }
    bool TransactionFailed()
    {
        return beginTransactionRC;
    }
};

#endif /* CardReader_hpp */
