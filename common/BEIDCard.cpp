#include "BEIDCard.hpp"
#include "log.hpp"
#include "util.h"
#include "general.h"
#include "hash.h"
#include "asn1.hpp"
#include "x509Util.h"
#include <unordered_map>

#define BEID_READ_BINARY_MAX_LEN     250			 /*maximum length to read with single card command*/

//MSE_SET command
//For Applet 1.7 cards
constexpr unsigned char prepareAuthenticationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x01, 0x84, 0x82 }; //"\x00\x22\x41\xB6\x05\x04\x80\x01\x84\x82"
//#define prepareAuthenticationSHA1Cmd                         "\x00\x22\x41\xB6\x05\x04\x80\x02\x84\x82"
//#define prepareAuthenticationMD5Cmd                          "\x00\x22\x41\xB6\x05\x04\x80\x04\x84\x82"

constexpr unsigned char prepareNonRepudiationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x01, 0x84, 0x83 }; //"\x00\x22\x41\xB6\x05\x04\x80\x01\x84\x83"
//#define prepareNonRepudiationSHA1Cmd                         "\x00\x22\x41\xB6\x05\x04\x80\x02\x84\x83"
//#define prepareNonRepudiationMD5Cmd                          "\x00\x22\x41\xB6\x05\x04\x80\x04\x84\x83"

//MSE_SET command
//For Applet 1.8 cards
constexpr unsigned char applet1_8_prepareAuthenticationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x40, 0x84, 0x82 }; //"\x00\x22\x41\xB6\x05\x04\x80\x40\x84\x82"
constexpr unsigned char applet1_8_prepareNonRepudiationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x40, 0x84, 0x83 }; //"\x00\x22\x41\xB6\x05\x04\x80\x40\x84\x83"

//#define resetPinApdu                              "\x00\x20\x00\x02\x08\x2C\x33\x33\x33\x11\x11\x11\xFF" (13 bytes) New PIN: 1234

//#define unblockCardApdu                           "\x00\x2c\x00\x01\x08\x2c\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
//#define verifyPinApdu                             "\x00\x20\x00\x01\x08\x2f\xff\xff\xff\xff\xff\xff\xff"
#define verifyPinApdu                               "\x00\x20\x00\x01\x08\x20\xff\xff\xff\xff\xff\xff\xff"
#define logoutApdu                                  "\x80\xE6\x00\x00"
//#define beid_apdu_change_pin_1                    "\x00\x24\x00\x01\x08\x2f\xff\xff\xff\xff\xff\xff\xff"
//#define beid_apdu_change_pin_0                    "\x00\x24\x01\x01\x10\x2f\xff\xff\xff\xff\xff\xff\xff\x2f\xff\xff\xff\xff\xff\xff\xff"

#define generateSignatureCmd                        "\x00\x2A\x9E\x9A"
#define retrieveSignatureCmd                        "\x00\xC0\x00\x00\x80"
//#define getResultCmd                              "\x00\xC0\x00\x00\x00"
//#define selectFileCmd                             "\x00\xa4\x02\x00\x02\x3f\x00"
//#define selectSerialCmd                           "\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x32"
//#define selectAuthenticationCertificateCmd        "\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x38"
//#define selectNonRepudiationCertificateCmd        "\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x39"
//#define selectCaCertificateCmd                    "\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x3a"
//#define selectRootCaCertificateCmd                "\x00\xA4\x08\x0C\x06\x3F\x00\xDF\x00\x50\x3b"

//#define select_Belpic_AID                         "\x00\xA4\x04\x0C\x0C\xA0\x00\x00\x01\x77\x50\x4B\x43\x53\x2D\x31\x35\x00"
//#define select_ID_AID                             "\x00\xA4\x04\x0C\x0C\xA0\x00\x00\x01\x77\x49\x64\x46\x69\x6C\x65\x73\x00"

//#define selectBEIDApplet                          "\x00\xA4\x04\x00\x0F\xA0\x00\x00\x00\x30\x29\x05\x70\x00\xAD\x13\x10\x01\x01\xFF"

//#define selectCardMgr                             "\x00\xA4\x04\x00\x07\xA0\x00\x00\x00\x03\x00\x00"
//#define selectMF                                  "\x00\xA4\x02\x0C\x02\x3F\x00"
//#define selectDF00                                "\x00\xA4\x02\x0C\x02\xDF\x00"
//#define selectDF01                                "\x00\xA4\x02\x0C\x02\xDF\x01"
//#define selectIdentityFileCmd                     "\x00\xA4\x02\x0C\x02\x40\x31" /* ends with 00=> Le=0 */
//#define selectIdentityFileSignatureCmd            "\x00\xA4\x02\x0C\x02\x40\x32"
//#define selectAddressFileCmd                      "\x00\xA4\x02\x0C\x02\x40\x33"
//#define selectAddressFileSignatureCmd             "\x00\xA4\x02\x0C\x02\x40\x34"
//#define selectPhotoCommand                        "\x00\xA4\x02\x0C\x02\x40\x35"

constexpr unsigned char beid_apdu_binary[5] = { 0x00, 0xB0, 0x00, 0x00, 0x00 }; //"\x00\xB0\x00\x00\x00"

constexpr unsigned char beid_apdu_GetCardData[5] = { 0x80, 0xE4, 0x00, 0x00, 0x1C };

// Files
constexpr unsigned char id[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x31 }; //"\x3F\x00\xDF\x01\x40\x31";
constexpr unsigned char id_sig[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x32 }; //"\x3F\x00\xDF\x01\x40\x32";
constexpr unsigned char address[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x33 }; //"\x3F\x00\xDF\x01\x40\x33";
constexpr unsigned char address_sig[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x34 }; //"\x3F\x00\xDF\x01\x40\x34";
constexpr unsigned char photo[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x35 }; //"\x3F\x00\xDF\x01\x40\x35";
constexpr unsigned char rrncert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x3C }; //"\x3F\x00\xDF\x00\x50\x3C";
constexpr unsigned char authcert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x38 }; //"\x3F\x00\xDF\x00\x50\x38";
constexpr unsigned char signcert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x39 }; //"\x3F\x00\xDF\x00\x50\x39";
constexpr unsigned char cacert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x3A }; //"\x3F\x00\xDF\x00\x50\x3A";
constexpr unsigned char rootcert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x3B }; //"\x3F\x00\xDF\x00\x50\x3B";

//std::string* BEIDCard::valueForKey(std::string* key)
//{
//   return nullptr;
//}

#define WHERE "BEIDCard::readCertificateChain"
void BEIDCard::readCertificateChain(std::vector<std::shared_ptr<const CardFile>>& subCerts, std::shared_ptr<const CardFile>& rootCert)
{
//    long ret;
//#define X509_ISSUER           "\1\1\4"
//#define X509_SUBJECT          "\1\1\6"
//    ASN1_ITEM subject, issuer;

    //-----------------------------------------------------------------------------
    // CA certificate
    //-----------------------------------------------------------------------------
    //std::shared_ptr<const CardFile> cacert = getFile(CardFiles::Cacert);
    //ret = asn1_get_item(cacert->getRaw().data(), cacert->getRaw().size(), X509_SUBJECT, &subject);
    //if (ret) {
    //    log_error("%s: Could not get subject name from certificate", WHERE);
    //    goto cleanup;
    //}

    //ret = asn1_get_item(cert, l_cert, X509_ISSUER, &issuer);
    //if (ret) {
    //    log_error("%s: Could not get issuer name from certificate", WHERE);
    //    goto cleanup;
    //}

    //if (asn_compare_items(&subject, &issuer) != 0) {
    //    //this is not the issuer we are looking for
    //    log_error("%s E: card does not contain the requested issuer certificate", WHERE);
    //    ret = -1;
    //    goto cleanup;
    //}

    subCerts.push_back(getFile(CardFiles::Cacert));

    //-----------------------------------------------------------------------------
    // root certificate
    //-----------------------------------------------------------------------------
    rootCert = getFile(CardFiles::Rootcert);

//cleanup:
//
//    return (ret);
}
#undef WHERE


#define WHERE "BEIDCard::SelectKey()"
long BEIDCard::selectKey(CardKeys pintype, unsigned char* cert, size_t l_cert)
{
    long ret = 0;
    size_t cmdlen, recvlen;
    unsigned char cmd[512], recv[512];
    int sw;

    std::shared_ptr<const CardFile> cardcert;
    if (pintype == CardKeys::NonRep)
    {
        cardcert = getFile(CardFiles::Signcert);
    }
    else
    {
        cardcert = getFile(CardFiles::Authcert);
    }

    //check if the current card is the one that contains the certificate we want to use for signing
    //in case of multiple readers, we should search until we find the right card
    if (memcmp(cert, cardcert->getRaw().data(), l_cert) != 0) {
        CLEANUP(E_SRC_CERT_NOT_FOUND);
    }

    ret = getKeyInfo(cardcert->getRaw().data(), cardcert->getRaw().size(), &currentSelectedKeyType, &currentSelectedKeyLength);
    if (ret) {
        log_error("E: getKeyInfo(type,size) returned %0X (%d)", ret);
        goto cleanup;
    }

    if (currentSelectedKeyType == X509_KEYTYPE_RSA)
    {
        // Applet 1.7
        if (pintype == CardKeys::NonRep)
        {
            cmdlen = sizeof(prepareNonRepudiationCmd);
            memcpy(cmd, prepareNonRepudiationCmd, cmdlen);
        }
        else
        {
            cmdlen = sizeof(prepareAuthenticationCmd);
            memcpy(cmd, prepareAuthenticationCmd, cmdlen);
        }
    }
    else
    {
        // Applet 1.8
        if (pintype == CardKeys::NonRep)
        {
            cmdlen = sizeof(applet1_8_prepareNonRepudiationCmd);
            memcpy(cmd, applet1_8_prepareNonRepudiationCmd, cmdlen);
        }
        else
        {
            cmdlen = sizeof(applet1_8_prepareAuthenticationCmd);
            memcpy(cmd, applet1_8_prepareAuthenticationCmd, cmdlen);
        }
    }

    recvlen = 255;
    ret = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
    if (ret < 0)
        goto cleanup;

    if ((sw != 0x9000))
    {
        CLEANUP(-1);
    }
cleanup:

    return ret;
}
#undef WHERE

// SizeOfPINLengthInAPDU (bits 7-4) bit size of PIN length in APDU
// PINBlockSize          (bits 3-0) PIN block size in bytes after justification and formatting
#define MAKE_PINBlockString(SizeOfPINLengthInAPDU, PINBlockSize) (((SizeOfPINLengthInAPDU&0xF)<<4)|(PINBlockSize&0xF))
// RFU                   (bits 7-5) RFU
// AreBytes              (bit 4   ) set if system units are bytes, clear if system units are bits 
// PINLengthPosition     (bits 3-0) PIN length position in system units 
#define MAKE_PINLengthFormat(RFU, AreBytes, PINLengthPosition) (((RFU&0x7)<<5)|((AreBytes&0x1)<<4)|(PINLengthPosition&0xF))
// SystemUnits           (bits 7  ) The system units' type indicator: 0=bits, 1=bytes. This bit quantifies the next param
// PINPosition           (bits 6-3) PIN position after format in the APDU command 
// PINJustification      (bit 2   ) PIN justification: 0=Left justify data, 1=Right justify data
// PINFormat             (bits 1-0) PIN Format type: 0=binary, 1=BCD, 2=ASCII
#define MAKE_FormatString(SystemUnits, PINPosition, PINJustification, PINFormat) (((SystemUnits&0x1)<<7)|((PINPosition&0xf)<<3)|((PINJustification&0x1)<<2)|(PINFormat&0x3))

#define WHERE "BEIDCard::logon()"
long BEIDCard::logon(int l_pin, char* pin)
{
    long ret;
    unsigned char recv[1024];
    size_t recvlen = 1024;
    unsigned char cmd[255];
    size_t cmdlen;
    int i;
    int sw = 0;

    ScopedCardTransaction trans(reader);  //begin transaction
    if (trans.TransactionFailed()) {
        log_error("E: Logon: Could not start transaction");
        return E_SRC_START_TRANSACTION;
    }

    //test Pin conditions
    if ((l_pin > 0) && (l_pin < 4))
        return E_PIN_TOO_SHORT;
    if (l_pin > 12)
        return E_PIN_TOO_LONG;

    cmdlen = sizeof(verifyPinApdu) - 1;
    memcpy(cmd, verifyPinApdu, cmdlen);

    //      pin_verify->bmFormatString = 0x89;  //FORMAT_BCD  (Belpic eid card)
    //      pin_verify->bmPINBlockString = 0x047; //0x04;//47
    //      pin_verify->bmPINLengthFormat = 0x04;
    //      pin_verify->wPINMaxExtraDigit = 0x040c; //test apg8201
    //      "\x00\x20\x00\x01\x08\x24\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
    if ((l_pin == 0) && (reader->isPinPad())) {
        ret = reader->verify_pinpad(MAKE_FormatString(1, 1, 0, 1), MAKE_PINBlockString(4, 7), MAKE_PINLengthFormat(0, 0, 4), 0x040c, cmd, cmdlen, &sw);  //TODO this should be handled more abstract since card shouldn't anything about the winswcard constants
        if (ret != 0) {
            log_error("%s: reader->verify_pinpad() returned 0x%0x", WHERE, ret);
        }
        return ret;
    }
    else if (pin != 0 && strlen(pin) >= 4) {
        //BCD encoding
        cmd[5] = 0x20 + l_pin;  /* 0x20 + length of pin */

        for (i = 0; i < l_pin; i++)
        {
            if (i % 2 == 0)
                cmd[6 + i / 2] = 0x0F + (((pin[i] - 48) << 4) & 0xF0);	/* pin[0], pin[1] in nibbles */
            else
                cmd[6 + (i / 2)] = (cmd[6 + (i / 2)] & 0xF0) + ((pin[i] - 48) & 0x0F);	/* pin[0], pin[1] in nibbles */
        }
        ret = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
        if (ret) {
            log_error("%s: reader->apdu(verify) returned 0x%0x", WHERE, ret);
        }
    }
    else {
        ret = -1;
    }

    if (ret < 0)
        return ret;

    switch (sw)
    {
    case 0x9000:
        return 0;
    case 0x63C3:
        return E_PIN_3_ATTEMPTS;
    case 0x63C2:
        return E_PIN_2_ATTEMPTS;
    case 0x63C1:
        return E_PIN_1_ATTEMPT;
    case 0x6983:
        return E_PIN_BLOCKED;
    case 0x6985:
        return E_SRC_COMMAND_NOT_ALLOWED;
    default:
        return E_PIN_INCORRECT;
    }
}
#undef WHERE

#define WHERE "BEIDCard::logoff()"
long BEIDCard::logoff()
{
    long ret;
    unsigned char recv[1024];
    size_t recvlen = 1024;
    unsigned char cmd[255];
    size_t cmdlen;
    int sw = 0;

    ScopedCardTransaction trans(reader);  //begin transaction
    if (trans.TransactionFailed()) {
        log_error("E: Logoff: Could not start transaction");
        return E_SRC_START_TRANSACTION;
    }

    cmdlen = sizeof(logoutApdu) - 1;
    memcpy(cmd, logoutApdu, cmdlen);

    ret = reader->apdu(cmd, cmdlen, recv, &recvlen, &sw);
    if (ret) {
        log_error("%s: reader->apdu(verify) returned 0x%0x", WHERE, ret);
    }

    return (ret);
}
#undef WHERE

#define WHERE "BEIDCard::sign()"
long BEIDCard::sign(const unsigned char* in, size_t l_in, int hashAlgo, unsigned char* out, size_t* l_out, int* sw)
{
    long ret = 0;
    unsigned char recv[1024];
    size_t recvlen = 1024;
    unsigned char cmd[255];
    size_t sign_lg = currentSelectedKeyLength;
    size_t cmdlen;

    if (sign_lg > *l_out) {
        return (-1);
    }

    //begin transaction
    ScopedCardTransaction trans(reader);  //begin transaction
    if (trans.TransactionFailed()) {
        log_error("E: reader->beginTransaction() failed");
        return E_SRC_START_TRANSACTION;
    }

    if ((l_in == 0) || (l_in != hash_length_for_algo(hashAlgo))) {
        log_error("hash input has wrong length for the specified digesting algo");
        return E_DIGEST_LEN;
    }

    memset(cmd, 0xFF, sizeof(cmd));

    /* sign */
    cmdlen = sizeof(generateSignatureCmd) - 1;
    memcpy(cmd, generateSignatureCmd, cmdlen);

    if (currentSelectedKeyType == X509_KEYTYPE_RSA) {
        cmd[cmdlen] = (unsigned char)(hash_length_for_algoheader(hashAlgo) + l_in);
        cmdlen++;
        memcpy(&cmd[cmdlen], hash_header_for_algo(hashAlgo), hash_length_for_algoheader(hashAlgo));
        cmdlen += hash_length_for_algoheader(hashAlgo);
        memcpy(&cmd[cmdlen], in, l_in);
        cmdlen += l_in;
        cmd[cmdlen] = 0x00;      /* append Le */
    }
    else {
        //EC
        cmd[cmdlen] = (unsigned char)(l_in); cmdlen++;
        memcpy(&cmd[cmdlen], in, l_in);
        cmdlen += l_in;
        cmd[cmdlen] = 0;     /* Le should be 0 */
    }

    recvlen = (int)sizeof(recv);

    ret = reader->apdu(cmd, cmdlen, recv, &recvlen, sw);
    if (ret < 0) {
        log_error("%s: reader->apdu returned 0x%0X", WHERE, ret);
        return ret;
    }

    cmdlen = sizeof(retrieveSignatureCmd) - 1;
    memcpy(cmd, retrieveSignatureCmd, cmdlen);

    switch (*sw) {
    case 0x6160:
        cmd[4] = 0x60;
        break;
    case 0x6180:
        cmd[4] = 0x80;
        break;
    case 0x6100:
        cmd[4] = 0x00;
        break;
    case 0x6401:
        return E_PIN_CANCELLED;
    default:
        return -1;
    }

    recvlen = *l_out;

    ret = reader->apdu(cmd, cmdlen, out, &recvlen, sw);
    if ((ret < 0) || (*sw != 0x9000)) {
        log_error("%s: reader->apdu returned 0x%0X (sw=0x%0X)", WHERE, ret, *sw);
        return ret;
    }

    *l_out = (unsigned int)recvlen;

    if (currentSelectedKeyType == X509_KEYTYPE_EC) {
        //signature is a concatenation of 2 values, so we encode these in ASN1 to return the ASN1 format
        ASN1_LIST     asn1_list;
        memset(&asn1_list, 0, sizeof(asn1_list));
        unsigned char part1[256]; //arbitrary length...
        unsigned char part2[256]; //arbitrary length...
        size_t size1 = *l_out / 2;
        size_t size2 = *l_out / 2;
        unsigned char* p = 0;
        memset(part1, 0, sizeof(part1));
        memset(part2, 0, sizeof(part2));
        //normalize as ASN_INTEGER (leading 0 if first byte > 0x7F)
        p = part1;
        if (out[0] > 0x7f) {
            p++;
            size1 += 1;
        }
        memcpy(p, out, *l_out / 2);

        p = part2;
        if (out[*l_out / 2] > 0x7f) {
            p++;
            size2 += 1;
        }
        memcpy(p, &out[*l_out / 2], *l_out / 2);

        asn1_add_item(&asn1_list, ASN_SEQUENCE, 0, 0, 2);
        {
            asn1_add_item(&asn1_list, ASN_INTEGER, part1, size1, 0);
            asn1_add_item(&asn1_list, ASN_INTEGER, part2, size2, 0);
        }

        unsigned char asn1_signature[512];
        size_t l_asn1_signature = 512;
        if ((ret = asn1_encode_list(&asn1_list, asn1_signature, &l_asn1_signature)) != 0)
        {
            log_error("asn1_encode returned 0x%08x (%d)", ret, ret);
            return ret;
        }
        if (l_asn1_signature)
            memcpy(out, asn1_signature, l_asn1_signature);
        *l_out = l_asn1_signature;
    }

    return (ret);
}
#undef WHERE

#define WHERE "BEIDCard::selectFile"
long BEIDCard::selectFile(const unsigned char* file, size_t l_file)
{
    long ret = 0;
    unsigned char apdu[256] = "\x00\xA4\x02\x0C\x00";
    unsigned int l_apdu = 5;
    unsigned char recv[256];
    size_t recvlen = 256;
    int sw = 0;
#if 0
    apdu[4] = l_file;
    memcpy(apdu + l_apdu, file, l_file);

    l_apdu += l_file;

    do_sleep(5);
    ret = reader->apdu(apdu, l_apdu, recv, &recvlen, &sw);
    if (ret < 0) {
        log_error("%s: reader->apdu returned 0x%0X", WHERE, ret);
        return(ret);
    }
    if (sw != 0x9000)
        return(-1);
#else
    apdu[4] = 2;
    l_apdu += 2;
    for (size_t i = 0; i < (l_file / 2); i++) {
        memcpy(&apdu[5], &file[i * 2], 2);
        do_sleep(5);
        ret = reader->apdu(apdu, l_apdu, recv, &recvlen, &sw);
        if (ret < 0) {
            log_error("%s: reader->apdu returned 0x%0X", WHERE, ret);
            return(ret);
        }
        if (sw != 0x9000)
            return(-1);
    }
#endif

    return(ret);
}
#undef WHERE

#define WHERE "BEIDCard::readFile2()"
std::shared_ptr<const CardFile> BEIDCard::readFile3(CardFileReadOptimization optimization)
{
    std::vector<unsigned char> result;
    std::vector<unsigned char> buf(1024);

    size_t p_maxReadlen = 1024 * 1024; // Max file size to read (define upper limite 1MB)

    size_t l_req;
    size_t l_read = 0;

    size_t recvlen = 1024;
    unsigned char cmd[255];

    int sw;

    int cmdlen = sizeof(beid_apdu_binary);
    memcpy(cmd, beid_apdu_binary, cmdlen);

    bool firstRead = true;

    while ((p_maxReadlen - l_read) > 0)                                       /* loop for big datasize     */
    {
        cmd[2] = (unsigned char)((l_read) >> 8);             /* set reading startpoint    */
        cmd[3] = (unsigned char)(l_read);
        l_req = p_maxReadlen - l_read;

        if (l_req > BEID_READ_BINARY_MAX_LEN)                                  /*if more than maximum length*/
            l_req = BEID_READ_BINARY_MAX_LEN;                               /* is requested, than read MAX_LEN  */

        cmd[4] = (unsigned char)(l_req);


        do_sleep(20);  //reading ID fails if we don't wait 1ms here ????

        buf.resize(1024);
        recvlen = buf.size();
        long iReturn = reader->apdu(cmd, cmdlen, buf.data(), &recvlen, &sw);
        if (iReturn) {
            throw CardFileException("APDU");
        }
        buf.resize(recvlen);

        //check if we want to read too much
        if ((sw & 0xFF00) == 0x6c00)
        {
            p_maxReadlen = l_read + (sw & 0xFF);
            continue;
        }
        else if (sw == 0x6B00)
        {
            //we wanted to read data that is not there so we stop reading and return what we have received so far
            break;
        }
        else if (recvlen == 0)
            break;

        //check if we received less than we requested for (DIGIPASS...)
        //we cannot do it this way since if we are on a boundary, this will not work read 250 bytes recv 250 bytes gives 90 00 but following read of 250 bytes will return 6B00,
        //so we need to keep reading and when receivein 6B 00, we stop and return what we have read so far
        result.insert(result.end(), buf.begin(), buf.end());            /* store read bytes          */
        l_read += recvlen;

        if ((int)l_req > recvlen)
            break;

        if (firstRead && optimization == CardFileReadOptimization::DEREncodedCertificate)
        {
            // All certificates stored in the card are DER encoded (not Base 64) and padded to a fixed length.
            // One's the first packet retrieved, we can calculate the "real" length of the certificate.
            // This avoid reading the padding bytes and can reduce the read operations on the card.
            // 
            // The Length field in a TLV triplet identifies the number of bytes encoded in the Value field.
            // The Value field contains the content being sent between computers.
            // If the Value field contains fewer than 128 bytes, the Length field requires only one byte.
            // Bit 7 of the Length field is zero (0) and the remaining bits identify the number of bytes of content being sent.
            // If the Value field contains more than 127 bytes, bit 7 of the Length field is one (1) and the remaining bits identify the number of bytes needed to contain the length.
            p_maxReadlen = (buf[2] << 8) + buf[3];
            if (p_maxReadlen > 0)
            {
                p_maxReadlen += 4;
            }
            firstRead = false;
        }
    }

    return std::make_shared<CardFile>(result);
}
#undef WHERE

std::shared_ptr<const CardFile> BEIDCard::getFile(CardFiles fileType)
{
    std::vector<unsigned char> idFile;
    CardFileReadOptimization optimization = CardFileReadOptimization::None;

    switch (fileType)
    {
    case CardFiles::Id:
    {
        idFile = std::vector<unsigned char>(id, id + sizeof(id));
        break;
    }
    case CardFiles::Id_sig:
    {
        idFile = std::vector<unsigned char>(id_sig, id_sig + sizeof(id_sig));
        break;
    }
    case CardFiles::Address:
    {
        idFile = std::vector<unsigned char>(address, address + sizeof(address));
        break;
    }
    case CardFiles::Address_sig:
    {
        idFile = std::vector<unsigned char>(address_sig, address_sig + sizeof(address_sig));
        break;
    }
    case CardFiles::Photo:
    {
        idFile = std::vector<unsigned char>(photo, photo + sizeof(photo));
        break;
    }
    case CardFiles::Rrncert:
    {
        idFile = std::vector<unsigned char>(rrncert, rrncert + sizeof(rrncert));
        optimization = CardFileReadOptimization::DEREncodedCertificate;
        break;
    }
    case CardFiles::Authcert:
    {
        idFile = std::vector<unsigned char>(authcert, authcert + sizeof(authcert));
        optimization = CardFileReadOptimization::DEREncodedCertificate;
        break;
    }
    case CardFiles::Signcert:
    {
        idFile = std::vector<unsigned char>(signcert, signcert + sizeof(signcert));
        optimization = CardFileReadOptimization::DEREncodedCertificate;
        break;
    }
    case CardFiles::Cacert:
    {
        idFile = std::vector<unsigned char>(cacert, cacert + sizeof(cacert));
        optimization = CardFileReadOptimization::DEREncodedCertificate;
        break;
    }
    case CardFiles::Rootcert:
    {
        idFile = std::vector<unsigned char>(rootcert, rootcert + sizeof(rootcert));
        optimization = CardFileReadOptimization::DEREncodedCertificate;
        break;
    }
    default:
    {
        throw CardFileException("Unknow file requested");
    }
    }
    long ret = 0;
    ScopedCardTransaction trans(reader);  //begin transaction
    ret = selectFile(idFile.data(), idFile.size());
    if (ret == 0) return readFile3(optimization);
    throw CardFileException("File select requested failed");
}

const std::map<std::string, std::string> BEIDCard::getCardData()
{
    if (!cacheCardDataLoaded)
    {
        unsigned char recv[256];
        size_t recvlen =256;
        int sw;
        if (0 == reader->apdu(beid_apdu_GetCardData, sizeof(beid_apdu_GetCardData), recv, &recvlen, &sw))
        {
            if (sw == 0x9000 && recvlen >= 28)
            {
                switch (recv[21])
                {
                case 0x17:cacheCardData["AppletVersion"] = "1.7"; break;
                case 0x18:cacheCardData["AppletVersion"] = "1.8"; break;
                default:cacheCardData["AppletVersion"] = "unknow"; break;
                }
            }
        }
        cacheCardDataLoaded = true;
    }
    return cacheCardData;
}

