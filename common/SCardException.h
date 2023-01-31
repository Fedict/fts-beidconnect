#pragma once

#include <exception>

enum SCardException_Code
{
    TransactionFail
};

class SCardException : public std::exception
{
private:
    SCardException_Code _code;
public:
    SCardException(SCardException_Code code) : _code(code)
    {
    }
    virtual const char* what() const noexcept
    {
        return "SCard Exception";
    }
    const SCardException_Code code()
    {
        return _code;
    }
    const char* result()
    {
        if (_code == TransactionFail)
        {
            return "busy";
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