#pragma once

#include <exception>
#ifndef _WIN32
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#ifndef ERROR_OPERATION_ABORTED
#define ERROR_OPERATION_ABORTED          995L
#endif // !ERROR_OPERATION_ABORTED
#ifndef ERROR_NO_MEDIA_IN_DRIVE
#define ERROR_NO_MEDIA_IN_DRIVE          1112L
#endif // !ERROR_NO_MEDIA_IN_DRIVE
#else
#include <winscard.h>
#endif
#include "x509Util.h"
#include "comm.hpp"
#include "debughelper.hpp"
#include "log.hpp"

enum class SCardException_Code : uint32_t
{
    TransactionFail = 0x00000001,
    Unknown = 0xFFFFFFFF,
};

/// <summary>
/// Exception on the card sub system.
/// Occurs when an error appears when communicating with the card on a SCardxxx function.
/// </summary>
class SCardException : public std::exception
{
private:
    LONG SCardResult = -1;
    SCardException_Code code = SCardException_Code::Unknown;
    std::string stack;
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="SCardResult">Return code of the SCard function</param>
    /// <param name="code">Functionnal code</param>
    SCardException(LONG SCardResult, SCardException_Code code = SCardException_Code::Unknown) : SCardResult(SCardResult), code(code)
    {
        stack = DebugStackTrace::GetStack();
    }
    virtual const char* what() const noexcept
    {
        return "SCard Exception";
    }
    inline const LONG getSCardResult() const
    {
        return SCardResult;
    }
    inline const SCardException_Code getCode() const
    {
        return code;
    }
    inline const std::string GetStack() const
    {
        return stack;
    }
    inline void log() const
    {
        log_error("SCardException SCardResult(%08X) code(%08X) functions(%s)", SCardResult, code, stack.c_str());
    }
    /// <summary>
    /// Map the card SW error to the error string to return to the browser extension
    /// </summary>
    /// <returns>Error string to return to the browser extension</returns>
    const char* result() const
    {
        if (code == SCardException_Code::TransactionFail)
        {
            return BeidConnect_Result::busy;
        }
        // Add LONG cast for MacOS Build
        switch (SCardResult)
        {
        case (LONG)SCARD_E_NOT_READY:
        {
            return BeidConnect_Result::busy;
        }
        case (LONG)SCARD_E_NO_SMARTCARD:
        case (LONG)SCARD_E_COMM_DATA_LOST:
        case (LONG)SCARD_W_REMOVED_CARD:
        case ERROR_NO_MEDIA_IN_DRIVE:
        case ERROR_OPERATION_ABORTED:   // VASCO DIGIPASS 870 return this error when removing the smart card during secure pin entry
        {
            return BeidConnect_Result::no_card;
        }
        default:
            break;
        }
        return BeidConnect_Result::general_error;
    }
    /// <summary>
    /// Return error type to the browser extension
    /// </summary>
    /// <returns>Error type to return to the browser extension</returns>
    const char* resultType() const
    {
        return "SCard";
    }
    /// <summary>
    /// Map the card SW error to the error string to return to the browser extension
    /// </summary>
    /// <returns>Raw Error code to return to the browser extension</returns>
    const uint32_t resultRaw() const
    {
        return SCardResult;
    }
};

enum class CardException_Code : uint32_t
{
    PIN_Incorrect = 0x00000001,
    Unknown = 0xFFFFFFFF,
};

/// <summary>
/// Exception on the card.
/// Occurs when an error is returned by the card. This represents the SW code returned by the card.
/// </summary>
class CardException : public std::exception
{
private:
    uint16_t SW;
    CardException_Code code = CardException_Code::Unknown;
    std::string stack;
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="SW">Return code returned by the smart card</param>
    /// <param name="code">Functionnal code</param>
    CardException(uint16_t SW, CardException_Code code = CardException_Code::Unknown) : SW(SW), code(code)
    {
        stack = DebugStackTrace::GetStack();
    }
    virtual const char* what() const noexcept
    {
        return "Card Exception";
    }
    inline const uint16_t getSW() const
    {
        return SW;
    }
    inline const CardException_Code getCode() const
    {
        return code;
    }
    inline const std::string GetStack() const
    {
        return stack;
    }
    inline void log() const
    {
        log_error("CardException SW(%04X) functions(%s)", SW, stack.c_str());
    }
    /// <summary>
    /// Map the card SW error to the error string to return to the browser extension
    /// </summary>
    /// <returns>Error string to return to the browser extension</returns>
    const char* result() const
    {
        switch (code)
        {
        case CardException_Code::PIN_Incorrect:
            return BeidConnect_Result::pin_incorrect;
        default:
            break;
        }
        switch (SW)
        {
            //case (int)E_SRC_NO_READERS_FOUND:
            //    response.put("result", BeidConnect_Result_no_reader);
            //    break;
            //case (int)E_SRC_NO_CARD:
            //    response.put("result", BeidConnect_Result_no_card);
            //    break;
            //case E_PIN_TOO_SHORT:
            //    response.put("result", BeidConnect_Result_pin_too_short);
            //    break;
            //case E_PIN_TOO_LONG:
            //    response.put("result", BeidConnect_Result_pin_too_long);
            //    break;
        case 0x63C3:
            return BeidConnect_Result::pin_3_attempts_left;
        case 0x63C2:
            return BeidConnect_Result::pin_2_attempts_left;
        case 0x63C1:
            return BeidConnect_Result::pin_1_attempt_left;
        case 0x6983:
            return BeidConnect_Result::card_blocked;
        case 0x6400:    // SPE operation timed out
            return BeidConnect_Result::pin_timeout;
        case 0x6401:    // SPE operation was cancelled by the Cancel button
            return BeidConnect_Result::cancel;
        case 0x6402:    // Modify PIN operation failed because two "new PIN" entries do not match
            return BeidConnect_Result::pin_incorrect;
        case 0x6403:    // User entered too short or too long PIN regarding MIN/MAX PIN Length
            // Note : as this error code is not known by CT-API implementations, it should be mapped to 64 01 on CT - API leve
            return BeidConnect_Result::pin_length;
        case 0x6985:    // E_SRC_COMMAND_NOT_ALLOWED;
            break;
        default:
            break;
        }
        return BeidConnect_Result::general_error;
    }
    /// <summary>
    /// Return error type to the browser extension
    /// </summary>
    /// <returns>Error type to return to the browser extension</returns>
    const char* resultType() const
    {
        return "Card";
    }
    /// <summary>
    /// Map the card SW error to the error string to return to the browser extension
    /// </summary>
    /// <returns>Raw Error code to return to the browser extension</returns>
    const uint32_t resultRaw() const
    {
        return SW;
    }
};

enum class BeidConnectException_Code : uint32_t
{
    SRC_NO_CONTEXT = 0x00000000,
    no_reader = 0x00000001,
    Not_PinPad = 0x00000010,
    SRC_CERT_NOT_FOUND = 0x00000020,
    CERT_Key_Info = 0x00000021,
    E_PIN_TOO_SHORT = 0x00000031,
    E_PIN_TOO_LONG = 0x00000032,
    Unknown = 0xFFFFFFFF,
};

/// <summary>
/// Exception on the beidConnect logic.
/// </summary>
class BeidConnectException : public std::exception
{
private:
    BeidConnectException_Code code = BeidConnectException_Code::Unknown;
    std::string stack;
public:
    BeidConnectException(BeidConnectException_Code code) : code(code)
    {
        stack = DebugStackTrace::GetStack();
    }
    virtual const char* what() const noexcept
    {
        return "BeidConnect Exception";
    }
    const BeidConnectException_Code getCode() const
    {
        return code;
    }
    inline const std::string GetStack() const
    {
        return stack;
    }
    inline void log() const
    {
        log_error("BeidConnectException code(%08X) functions(%s)", code, stack.c_str());
    }
    /// <summary>
    /// Map the card SW error to the error string to return to the browser extension
    /// </summary>
    /// <returns>Error string to return to the browser extension</returns>
    const char* result() const
    {
        switch (code)
        {
        case BeidConnectException_Code::Not_PinPad:
        case BeidConnectException_Code::SRC_CERT_NOT_FOUND:
            return BeidConnect_Result::no_card;
        case BeidConnectException_Code::SRC_NO_CONTEXT:
        case BeidConnectException_Code::no_reader:
            return BeidConnect_Result::no_reader;
        case BeidConnectException_Code::E_PIN_TOO_SHORT:
            return BeidConnect_Result::pin_too_short;
        case BeidConnectException_Code::E_PIN_TOO_LONG:
            return BeidConnect_Result::pin_too_long;
        default:
            break;
        }
        return BeidConnect_Result::general_error;
    }
    /// <summary>
    /// Return error type to the browser extension
    /// </summary>
    /// <returns>Error type to return to the browser extension</returns>
    const char* resultType() const
    {
        return "BeidConnect";
    }
    /// <summary>
    /// Map the card SW error to the error string to return to the browser extension
    /// </summary>
    /// <returns>Raw Error code to return to the browser extension</returns>
    const uint32_t resultRaw() const
    {
        return (uint32_t)code;
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
