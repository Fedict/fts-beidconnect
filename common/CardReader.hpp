#pragma once

#ifndef CardReader_hpp
#define CardReader_hpp

#include <iostream>
#include "general.h"
#include "Card.hpp"
#include <vector>
#include <memory>
#include <fstream>
#include "log.hpp"
#include "CardErrors.h"
#include "debughelper.hpp"
#ifdef _DEBUG
#include "assert.h"
#endif

#define MAX_ATR_LENGTH			128
#define MAX_LABEL_LENGTH		32
#define MAX_READERS				10

using namespace std;

/// <summary>
/// Encapsulate an APDU to transmit to the smart card.
/// </summary>
class CardAPDU
{
    std::vector<unsigned char> APDU;
public:
    CardAPDU(){}
    CardAPDU(const unsigned char* data, size_t len, size_t lenBuffer = SIZE_MAX)
    {
        if (lenBuffer == SIZE_MAX) lenBuffer = len;
        APDU.resize(lenBuffer);
        APDU.assign(data, data + len);
    }
    CardAPDU(std::initializer_list<unsigned char> data, size_t lenBuffer = SIZE_MAX) : APDU(data)
    {
        if (lenBuffer != SIZE_MAX) APDU.resize(lenBuffer);;
    }
    /// <summary>
    /// Get the ADPU bytes.
    /// </summary>
    /// <returns>A const vector containig the APDU bytes</returns>
    inline const std::vector<unsigned char>& GetAPDU() const
    {
        return APDU;
    }
    /// <summary>
    /// Modify a byte at a specific position.
    /// </summary>
    /// <param name="pos">The position where to change the byte.</param>
    /// <param name="value">The new value.</param>
    inline void patchAt(size_t pos, unsigned char value)
    {
        APDU[pos] = value;
    }
    /// <summary>
    /// Get a value at a specific position
    /// </summary>
    /// <param name="pos">The position where to retribe the byte value.</param>
    /// <returns>The byte value.</returns>
    inline unsigned char getAt(size_t pos)
    {
        return APDU[pos];
    }
    /// <summary>
    /// Append a byte value at the end of the ADPU.
    /// </summary>
    /// <param name="value">The value to append.</param>
    inline void append(unsigned char value)
    {
        APDU.push_back(value);
    }
    /// <summary>
    /// Append an array of byte value at the end of the ADPU.
    /// </summary>
    /// <param name="value">The array to append.</param>
    /// <param name="len">The length of the array to append.</param>
    inline void append(const unsigned char* value, size_t len)
    {
        APDU.insert(APDU.end(), value, value+ len);
    }
};

/// <summary>
/// Encapsulate a response regarding an transmited APDU to the smart card.
/// It may contain the SW and the result data.
/// </summary>
class CardAPDUResponse
{
    std::vector<unsigned char> result;
    uint16_t SW = 0;
public:
    CardAPDUResponse(){}
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
        SW = (uint16_t)((data[len - 2] << 8) + data[len - 1]);
    }
    inline void setSW(uint16_t SW)
    {
        this->SW = SW;
    }
    inline uint16_t getSW() const
    {
        return SW;
    }
    inline size_t getDataLen() const
    {
        return result.size();
    }
    inline unsigned char getDataAtPos(size_t pos) const
    {
        return result[pos];
    }
    inline const std::vector<unsigned char>& getData() const
    {
        return result;
    }
};

class CardReader
{
    // To ensure transaction begin/end, use the ScopedCardTransaction class
    friend class ScopedCardTransaction;
    virtual void beginTransaction() = 0;
    virtual void endTransaction() = 0;
public:
    CardReader() {};
    virtual ~CardReader() {};
    virtual long connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isPinPad() = 0;
    virtual CardAPDUResponse apdu(const CardAPDU& apdu) = 0;
    virtual void verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, uint16_t PINMaxExtraDigit, const unsigned char pinAPDU[], size_t l_pinAPDU, uint16_t* sw) = 0;

    std::string name;
    std::string atr;
    int language = 0;
};

/// <summary>
/// Transaction helper class. Try to start a transaction. When out of the calling scope, a successfull transaction is ended.
/// </summary>
class ScopedCardTransaction
{
    DebugStackTrace debugStackTrace;
    std::shared_ptr<CardReader> reader;
    bool TransactionInProgress = false;
#ifdef _DEBUG
    static int Counter;
#endif
public:
    ScopedCardTransaction(const std::shared_ptr<CardReader>& reader) : debugStackTrace("T")
    {
        this->reader = reader;
        reader->beginTransaction();
        TransactionInProgress = true;
#ifdef _DEBUG
        ++Counter;
        assert(Counter == 1);
#endif
    }
    ~ScopedCardTransaction()
    {
        if (TransactionInProgress) {
            reader->endTransaction();
#ifdef _DEBUG
            Counter--;
#endif
        }
    }
};

#endif /* CardReader_hpp */
