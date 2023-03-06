#pragma once

#include <exception>
#ifndef _WIN32
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#else
#include <winscard.h>
#endif
#include "x509Util.h"

enum SCardException_Code
{
    TransactionFail,
    UnknownCode
};

/// <summary>
/// Exception on the card sub system.
/// Occurs when an error appears when communicating with the card.
/// </summary>
class SCardException : public std::exception
{
private:
    SCardException_Code code = SCardException_Code::UnknownCode;
    LONG SCardResult = -1;
public:
    SCardException(SCardException_Code code) : code(code)
    {
    }
    SCardException(LONG SCardResult) : SCardResult(SCardResult)
    {
    }
    virtual const char* what() const noexcept
    {
        return "SCard Exception";
    }
    const SCardException_Code getCode()
    {
        return code;
    }
    const char* result()
    {
        switch (SCardResult)
        {
        case SCARD_E_NO_SMARTCARD:
        {
            return "no_card";
        }
        }
        if (code == TransactionFail)
        {
            return "busy";
        }
        return "general_error";
    }
};

/// <summary>
/// Exception on the card.
/// Occurs when an error is returned by the card. This represents the SW code returned by the card.
/// </summary>
class CardException : public std::exception
{
private:
    uint16_t SW;
public:
    CardException(uint16_t SW) : SW(SW)
    {
    }
    virtual const char* what() const noexcept
    {
        return "Card Exception";
    }
    const char* result()
    {
        switch (SW)
        {
        case 0x6401:
        {
            return "no_card";
        }
        //case 0x63C3:
        //    return E_PIN_3_ATTEMPTS;
        //case 0x63C2:
        //    return E_PIN_2_ATTEMPTS;
        //case 0x63C1:
        //    return E_PIN_1_ATTEMPT;
        //case 0x6983:
        //    return E_PIN_BLOCKED;
        //case 0x6985:
        //    return E_SRC_COMMAND_NOT_ALLOWED;

        }
        return "general_error";
    }
};

class NotImplementedException : public std::exception
{
public:
    NotImplementedException(const std::string& _Message) : std::exception(/*_Message*/)
    {
    }
    virtual const char* what() const noexcept { return "Function not yet implemented."; }
};

class CardFileException : public std::exception
{
public:
    CardFileException(const std::string& _Message) : std::exception(/*_Message*/)
    {
    }
    virtual char const* what() const noexcept { return "Wrong Card File."; }
};