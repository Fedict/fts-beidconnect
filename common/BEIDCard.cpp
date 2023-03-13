#include <unordered_map>
#include "BEIDCard.hpp"
#include "log.hpp"
#include "util.h"
#include "general.h"
#include "hash.h"
#include "asn1.hpp"
#include "x509Util.h"
#ifdef _DEBUG
#include "test.hpp"
#endif

#define BEID_READ_BINARY_MAX_LEN     250			 /*maximum length to read with single card command*/

//MSE_SET command
//For Applet 1.7 cards
constexpr unsigned char prepareAuthenticationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x01, 0x84, 0x82 };
//#define prepareAuthenticationSHA1Cmd                         "\x00\x22\x41\xB6\x05\x04\x80\x02\x84\x82"
//#define prepareAuthenticationMD5Cmd                          "\x00\x22\x41\xB6\x05\x04\x80\x04\x84\x82"

constexpr unsigned char prepareNonRepudiationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x01, 0x84, 0x83 };
//#define prepareNonRepudiationSHA1Cmd                         "\x00\x22\x41\xB6\x05\x04\x80\x02\x84\x83"
//#define prepareNonRepudiationMD5Cmd                          "\x00\x22\x41\xB6\x05\x04\x80\x04\x84\x83"

//MSE_SET command
//For Applet 1.8 cards
constexpr unsigned char applet1_8_prepareAuthenticationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x40, 0x84, 0x82 };
constexpr unsigned char applet1_8_prepareNonRepudiationCmd[10] = { 0x00, 0x22, 0x41, 0xB6, 0x05, 0x04, 0x80, 0x40, 0x84, 0x83 };

//#define resetPinApdu                              "\x00\x20\x00\x02\x08\x2C\x33\x33\x33\x11\x11\x11\xFF" (13 bytes) New PIN: 1234

//#define unblockCardApdu                           "\x00\x2c\x00\x01\x08\x2c\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
//#define verifyPinApdu                             "\x00\x20\x00\x01\x08\x2f\xff\xff\xff\xff\xff\xff\xff"
constexpr unsigned char verifyPinApdu[] = { 0x00,0x20,0x00,0x01,0x08,0x20,0xff,0xff,0xff,0xff,0xff,0xff,0xff };
constexpr unsigned char logoutApdu[] = { 0x80, 0xE6, 0x00, 0x00 };
//#define beid_apdu_change_pin_1                    "\x00\x24\x00\x01\x08\x2f\xff\xff\xff\xff\xff\xff\xff"
//#define beid_apdu_change_pin_0                    "\x00\x24\x01\x01\x10\x2f\xff\xff\xff\xff\xff\xff\xff\x2f\xff\xff\xff\xff\xff\xff\xff"

constexpr unsigned char generateSignatureCmd[] = { 0x00,0x2A,0x9E,0x9A };
constexpr unsigned char retrieveSignatureCmd[] = { 0x00,0xC0,0x00,0x00,0x80 };
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

constexpr unsigned char beid_apdu_binary[5] = { 0x00, 0xB0, 0x00, 0x00, 0x00 };

constexpr unsigned char beid_apdu_GetCardData[5] = { 0x80, 0xE4, 0x00, 0x00, 0x1C };

// Files
constexpr unsigned char id[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x31 };
constexpr unsigned char id_sig[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x32 };
constexpr unsigned char address[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x33 };
constexpr unsigned char address_sig[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x34 };
constexpr unsigned char photo[6] = { 0x3F, 0x00, 0xDF, 0x01, 0x40, 0x35 };
constexpr unsigned char rrncert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x3C };
constexpr unsigned char authcert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x38 };
constexpr unsigned char signcert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x39 };
constexpr unsigned char cacert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x3A };
constexpr unsigned char rootcert[6] = { 0x3F, 0x00, 0xDF, 0x00, 0x50, 0x3B };

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
void BEIDCard::selectKey(CardKeys pintype, const std::vector<unsigned char>& cert)
{
    long ret = 0;

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
    if (cert != cardcert->getRaw()) {
        throw BeidConnectException(BeidConnectException_Code::SRC_CERT_NOT_FOUND);
    }

    ret = getKeyInfo(cardcert->getRaw().data(), cardcert->getRaw().size(), &currentSelectedKeyType, &currentSelectedKeyLength);
    if (ret) {
        log_error("E: getKeyInfo(type,size) returned %0X (%d)", ret);
        throw BeidConnectException(BeidConnectException_Code::CERT_Key_Info);
    }

    CardAPDUResponse cr;
    if (currentSelectedKeyType == X509_KEYTYPE_RSA)
    {
        // Applet 1.7
        if (pintype == CardKeys::NonRep)
        {
            cr = reader->apdu(CardAPDU(&prepareNonRepudiationCmd[0], sizeof(prepareNonRepudiationCmd)));
        }
        else
        {
            cr = reader->apdu(CardAPDU(&prepareAuthenticationCmd[0], sizeof(prepareAuthenticationCmd)));
        }
    }
    else
    {
        // Applet 1.8
        if (pintype == CardKeys::NonRep)
        {
            cr = reader->apdu(CardAPDU(&applet1_8_prepareNonRepudiationCmd[0], sizeof(applet1_8_prepareNonRepudiationCmd)));
        }
        else
        {
            cr = reader->apdu(CardAPDU(&applet1_8_prepareAuthenticationCmd[0], sizeof(applet1_8_prepareAuthenticationCmd)));
        }
    }

    if (cr.getSW() != 0x9000) throw CardException(cr.getSW());
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
void BEIDCard::logon(int l_pin, char* pin)
{
    uint16_t sw = 0;

#ifdef _DEBUG
    if (unittest_Generate_Exception_Other) {
        throw CardException(0x9999);
    }
    if (unittest_Generate_Exception_Pin_Blocked) {
        throw CardException(0x6983);
    }
    if (unittest_Generate_Exception_Pin_3_attempts) {
        throw CardException(0x63C3);
    }
    if (unittest_Generate_Exception_Pin_2_attempts) {
        throw CardException(0x63C2);
    }
    if (unittest_Generate_Exception_Pin_1_attempt) {
        throw CardException(0x63C1);
    }
    if (unittest_Generate_Exception_PIN_TOO_SHORT) {
        throw BeidConnectException(BeidConnectException_Code::E_PIN_TOO_SHORT);
    }
    if (unittest_Generate_Exception_PIN_TOO_LONG) {
        throw BeidConnectException(BeidConnectException_Code::E_PIN_TOO_LONG);
    }
    if (unittest_Generate_Exception_PIN_Incorrect) {
        throw CardException(0, CardException_Code::PIN_Incorrect);
    }
#endif

    ScopedCardTransaction trans(reader);  //begin transaction

    //test Pin conditions
    if ((l_pin > 0) && (l_pin < 4))
        throw BeidConnectException(BeidConnectException_Code::E_PIN_TOO_SHORT);
    if (l_pin > 12)
        throw BeidConnectException(BeidConnectException_Code::E_PIN_TOO_LONG);

    CardAPDU cmd(&verifyPinApdu[0], sizeof(verifyPinApdu));
    CardAPDUResponse resp;

    //      pin_verify->bmFormatString = 0x89;  //FORMAT_BCD  (Belpic eid card)
    //      pin_verify->bmPINBlockString = 0x047; //0x04;//47
    //      pin_verify->bmPINLengthFormat = 0x04;
    //      pin_verify->wPINMaxExtraDigit = 0x040c; //test apg8201
    //      "\x00\x20\x00\x01\x08\x24\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
    if ((l_pin == 0) && (reader->isPinPad())) {
        reader->verify_pinpad(MAKE_FormatString(1, 1, 0, 1), MAKE_PINBlockString(4, 7), MAKE_PINLengthFormat(0, 0, 4), 0x040c, cmd.GetAPDU().data(), cmd.GetAPDU().size(), &sw);  //TODO this should be handled more abstract since card shouldn't anything about the winswcard constants
        return;
    }
    else if (pin != 0 && strlen(pin) >= 4) {
        //BCD encoding
        cmd.patchAt(5, (unsigned char)(0x20 + l_pin));  /* 0x20 + length of pin */

        for (int i = 0; i < l_pin; i++)
        {
            if (i % 2 == 0)
                cmd.patchAt(6 + i / 2, (unsigned char)(0x0F + (((pin[i] - 48) << 4) & 0xF0)));	/* pin[0], pin[1] in nibbles */
            else
                cmd.patchAt(6 + (i / 2), (cmd.getAt(6 + (i / 2)) & 0xF0) + ((pin[i] - 48) & 0x0F));	/* pin[0], pin[1] in nibbles */
        }
        resp = reader->apdu(cmd);
    }
    else {
        throw BeidConnectException(BeidConnectException_Code::E_PIN_TOO_SHORT);
    }

    switch (resp.getSW())
    {
    case 0x9000:
        return;
    case 0x63C3: // E_PIN_3_ATTEMPTS;
    case 0x63C2: // E_PIN_2_ATTEMPTS;
    case 0x63C1: // E_PIN_1_ATTEMPT;
    case 0x6983: // E_PIN_BLOCKED;
    case 0x6985: // E_SRC_COMMAND_NOT_ALLOWED;
        log_error("%s: E: Card returns SW(%04X)", WHERE, resp.getSW());
        throw CardException(resp.getSW());
    default:
        throw CardException(resp.getSW(), CardException_Code::PIN_Incorrect);
    }
}
#undef WHERE

#define WHERE "BEIDCard::logoff()"
void BEIDCard::logoff()
{
    ScopedCardTransaction trans(reader);  //begin transaction
    reader->apdu(CardAPDU(&logoutApdu[0], sizeof(logoutApdu)));
}
#undef WHERE

#define WHERE "BEIDCard::sign()"
long BEIDCard::sign(const std::vector<unsigned char>& in, int hashAlgo, unsigned char* out, size_t* l_out, int* sw)
{
    long ret = 0;
    size_t sign_lg = currentSelectedKeyLength;

    if (sign_lg > *l_out) {
        return (-1);
    }

    //begin transaction
    ScopedCardTransaction trans(reader);  //begin transaction

    if ((in.size() == 0) || (in.size() != hash_length_for_algo(hashAlgo))) {
        log_error("hash input has wrong length for the specified digesting algo");
        return E_DIGEST_LEN;
    }

    /* sign */
    CardAPDU cmdGenSign(&generateSignatureCmd[0], sizeof(generateSignatureCmd));
    if (currentSelectedKeyType == X509_KEYTYPE_RSA) {
        // RSA
        cmdGenSign.append((unsigned char)(hash_length_for_algoheader(hashAlgo) + in.size()));
        cmdGenSign.append(hash_header_for_algo(hashAlgo), hash_length_for_algoheader(hashAlgo));
        cmdGenSign.append(in.data(), in.size());
    }
    else {
        //EC
        cmdGenSign.append((unsigned char)in.size());
        cmdGenSign.append(in.data(), in.size());
    }
    CardAPDUResponse responseGenSign = reader->apdu(cmdGenSign);

    CardAPDU cmdRetrieveSignature(&retrieveSignatureCmd[0], sizeof(retrieveSignatureCmd));
    switch (responseGenSign.getSW()) {
    case 0x6160:
        cmdRetrieveSignature.patchAt(4, 0x60);
        break;
    case 0x6180:
        cmdRetrieveSignature.patchAt(4, 0x80);
        break;
    case 0x6100:
        cmdRetrieveSignature.patchAt(4, 0x00);
        break;
    case 0x6401:
        log_error("%s: E: Card returns SW(%04X)", WHERE, responseGenSign.getSW());
        throw CardException(responseGenSign.getSW());
    default:
        return -1;
    }
    CardAPDUResponse responseRetrieveSignature = reader->apdu(cmdRetrieveSignature);
    *l_out = responseRetrieveSignature.getDataLen();
    memcpy(out, responseRetrieveSignature.getData().data(), responseRetrieveSignature.getDataLen());

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
void BEIDCard::selectFile(const unsigned char* file, size_t l_file)
{
    CardAPDU apdu({ 0x00,0xA4,0x02,0x0C,0x00 }, 7);
    apdu.patchAt(4, 2);
    for (size_t i = 0; i < (l_file / 2); i++) {
        apdu.patchAt(5, file[i * 2]);
        apdu.patchAt(6, file[(i * 2)+1]);
        do_sleep(5);
        CardAPDUResponse cr = reader->apdu(apdu);
        if (cr.getSW() != 0x9000)
        {
            log_error("%s: E: Card returns SW(%04X)", WHERE, cr.getSW());
            throw CardException(cr.getSW());
        }
    }
}
#undef WHERE

#define WHERE "BEIDCard::readFile()"
std::shared_ptr<const CardFile> BEIDCard::readFile(CardFileReadOptimization optimization)
{
    std::vector<unsigned char> result;
    size_t p_maxReadlen = 1024 * 1024; // Max file size to read (define upper limite 1MB)
    size_t l_req;
    size_t l_read = 0;
    bool firstRead = true;
    CardAPDU cmd(&beid_apdu_binary[0], sizeof(beid_apdu_binary));

    while ((p_maxReadlen - l_read) > 0)                                       /* loop for big datasize     */
    {
        cmd.patchAt(2, (unsigned char)((l_read) >> 8));             /* set reading startpoint    */
        cmd.patchAt(3, (unsigned char)(l_read));
        l_req = p_maxReadlen - l_read;

        if (l_req > BEID_READ_BINARY_MAX_LEN)                                  /*if more than maximum length*/
            l_req = BEID_READ_BINARY_MAX_LEN;                               /* is requested, than read MAX_LEN  */

        cmd.patchAt(4, (unsigned char)(l_req));

        do_sleep(20);  //reading ID fails if we don't wait 1ms here ????

        CardAPDUResponse r = reader->apdu(cmd);

        //check if we want to read too much
        if ((r.getSW() & 0xFF00) == 0x6c00)
        {
            p_maxReadlen = l_read + (r.getSW() & 0xFF);
            continue;
        }
        else if (r.getSW() == 0x6B00)
        {
            //we wanted to read data that is not there so we stop reading and return what we have received so far
            break;
        }
        else if (r.getDataLen() == 0)
            break;

        //check if we received less than we requested for (DIGIPASS...)
        //we cannot do it this way since if we are on a boundary, this will not work read 250 bytes recv 250 bytes gives 90 00 but following read of 250 bytes will return 6B00,
        //so we need to keep reading and when receivein 6B 00, we stop and return what we have read so far
        result.insert(result.end(), r.getData().begin(), r.getData().end());            /* store read bytes          */
        l_read += r.getDataLen();

        if (l_req > r.getDataLen())
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
            p_maxReadlen = (r.getData()[2] << 8) + r.getData()[3];
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

#ifdef _DEBUG
    if (unittest_Generate_Exception_Removed_Card) {
        throw SCardException(SCARD_W_REMOVED_CARD);
    }
#endif

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
    ScopedCardTransaction trans(reader);  //begin transaction
    selectFile(idFile.data(), idFile.size());
    return readFile(optimization);
}

const std::map<std::string, std::string> BEIDCard::getCardData()
{
    if (!cacheCardDataLoaded)
    {
        try
        {
            CardAPDUResponse r = reader->apdu(CardAPDU(&beid_apdu_GetCardData[0], sizeof(beid_apdu_GetCardData)));
            if (r.getSW() == 0x9000 && r.getDataLen() >= 28)
            {
                switch (r.getDataAtPos(21))
                {
                case 0x17:cacheCardData["AppletVersion"] = "1.7"; break;
                case 0x18:cacheCardData["AppletVersion"] = "1.8"; break;
                default:cacheCardData["AppletVersion"] = "unknow"; break;
                }
            }
        }
        catch (...) {}
        cacheCardDataLoaded = true;
    }
    return cacheCardData;
}
