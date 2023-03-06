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

class CardAPDU
{
    std::vector<unsigned char> APDU;
public:
    CardAPDU(unsigned char* data, size_t len, size_t lenBuffer = -1)
    {
        if (lenBuffer == -1) lenBuffer = len;
        APDU.resize(lenBuffer);
        APDU.assign(data, data + len);
    }
    CardAPDU(std::initializer_list<unsigned char> data, size_t lenBuffer = -1) : APDU(data)
    {
        if (lenBuffer != -1) APDU.resize(lenBuffer);;
    }
    inline const std::vector<unsigned char>& GetAPDU() const
    {
        return APDU;
    }
    inline void patch(size_t pos, unsigned char value)
    {
        APDU[pos] = value;
    }
};
class CardAPDUResponse
{
    std::vector<unsigned char> result;
    uint16_t SW;
public:
    CardAPDUResponse(unsigned char* data, size_t len)
    {
        setCardResponse(data, len);
    }
    inline void setCardResponse(unsigned char* data, size_t len)
    {
        size_t lenData = len;
        if (lenData >= 2) lenData -= 2;

        if (lenData > 0) {
            result.resize(lenData);
            result.assign(data, data + lenData);
        }
        SW = (data[len - 2] << 8) + data[len - 1];
    }
    inline void setSW(uint16_t SW)
    {
        this->SW = SW;
    }
    inline uint16_t getSW() const
    {
        return SW;
    }
};

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
    virtual CardAPDUResponse apdu2(const CardAPDU& apdu) = 0;
    virtual long verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], size_t l_pinAPDU, int* sw) = 0;

    std::string name;
    std::string atr;
    int language = 0;
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
