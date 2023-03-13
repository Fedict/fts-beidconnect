#pragma once

#ifndef __COMM_H
#define __COMM_H

// json field
namespace BeidConnect_JSON_field
{
    constexpr char result[] = "result";
    constexpr char report[] = "report";
    constexpr char version[] = "version";
    constexpr char Readers[] = "Readers";
    constexpr char ReaderName[] = "ReaderName";
    constexpr char ReaderType[] = "ReaderType";
    constexpr char cardType[] = "cardType";
    constexpr char keyusage[] = "keyusage";
    constexpr char certificates[] = "certificates";
    constexpr char certificateChain[] = "certificateChain";
    constexpr char rootCA[] = "rootCA";
    constexpr char subCA[] = "subCA";
    constexpr char cert[] = "cert";
    constexpr char signature[] = "signature";
    constexpr char operation[] = "operation";
    constexpr char correlationId[] = "correlationId";
    constexpr char pin[] = "pin";
    /// <summary>
    /// Digest to sign
    /// </summary>
    constexpr char digest[] = "digest";
    constexpr char algo[] = "algo";
};

// "operation" json field possible values :
// Those are the "operation"s that can be requested by the browser extension
namespace BeidConnect_operation
{
    constexpr char VERSION[] = "VERSION";
    constexpr char ID[] = "ID";
    constexpr char USERCERTS[] = "USERCERTS";
    constexpr char CERTCHAIN[] = "CERTCHAIN";
    constexpr char SIGN[] = "SIGN";
    constexpr char AUTH[] = "AUTH";
}

// "ReaderType" json field possible values :
namespace BeidConnect_ReaderType
{
    constexpr char pinpad[] = "pinpad";
    constexpr char standard[] = "standard";
}

// "keyusage" json field possible values :
namespace BeidConnect_Keyusage
{
    constexpr char NONREPUDIATION[] = "NONREPUDIATION";
    constexpr char DIGITALSIGNATURE[] = "DIGITALSIGNATURE";
}

// "result" json field possible values : Error statuses that BeidConnect can return in the 
namespace BeidConnect_Result
{
    constexpr char OK[] = "OK";
    constexpr char general_error[] = "general_error";
    constexpr char busy[] = "busy";
    constexpr char no_card[] = "no_card";
    constexpr char no_reader[] = "no_reader";
    constexpr char pin_incorrect[] = "pin_incorrect";
    constexpr char pin_too_short[] = "pin_too_short";
    constexpr char pin_too_long[] = "pin_too_long";
    constexpr char pin_3_attempts_left[] = "pin_3_attempts_left";
    constexpr char pin_2_attempts_left[] = "pin_2_attempts_left";
    constexpr char pin_1_attempt_left[] = "pin_1_attempt_left";
    constexpr char card_blocked[] = "card_blocked";
    constexpr char pin_timeout[] = "pin_timeout";
    constexpr char cancel[] = "cancel";
    constexpr char pin_length[] = "pin_length";
    //constexpr char card_type_unsupported = "card_type_unsupported";
}

enum class BECOM_Code : uint32_t
{
    OK = 0x00000000,
    E_COMM_PARAM = 0x00000001,
    E_COMM_ENDREQUEST = 0x00000002,
};

BECOM_Code readMessage(std::stringstream &stream);
void sendMessage(const std::string response);

#endif //__COMM_H

