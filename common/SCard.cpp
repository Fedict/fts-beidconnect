#include "SCard.hpp"
#include "Card.hpp"
#include "CardFactory.hpp"
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "log.hpp"
#include "general.h"
#include "util.h"
#include <thread>
#include "SCardException.h"
#ifdef _DEBUG
#include "test.hpp"
#endif

#define READERS_BUF_SIZE 2000

#ifndef _WIN32
#define lstrlen(s) strlen(s)   // non-Windows wintypes.h: LPTSTR == LPSTR == char*
#endif

/******************************************************************************
 *
 * Local Definitions
 *
 ******************************************************************************/
#define CM_IOCTL_GET_FEATURE_REQUEST          SCARD_CTL_CODE(3400)
#ifdef _WIN32
#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE   SCARD_CTL_CODE(2048)
#else
#define SCARD_CTL_CODE(code) (0x42000000 + (code)) // from reader.h in libpcsclite
#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE   SCARD_CTL_CODE(1)
#endif

#define FEATURE_VERIFY_PIN_START        0x01
#define FEATURE_VERIFY_PIN_FINISH       0x02
#define FEATURE_MODIFY_PIN_START        0x03
#define FEATURE_MODIFY_PIN_FINISH       0x04
#define FEATURE_GET_KEY_PRESSED         0x05
#define FEATURE_VERIFY_PIN_DIRECT       0x06
#define FEATURE_MODIFY_PIN_DIRECT       0x07
#define FEATURE_MCT_READERDIRECT        0x08
#define FEATURE_MCT_UNIVERSAL           0x09
#define FEATURE_IFD_PIN_PROP            0x0A
#define FEATURE_ABORT                   0x0B
#define FEATURE_CCID_ESC_COMMAND        0x13

//see pcsc10_v2.01.6.pdf
#pragma pack(1)

#define PP_APDU_MAX_LEN  40

typedef struct PIN_VERIFY_STRUCTURE
{
   BYTE    bTimeOut;                 // timeout in seconds (00 means use default timeout)
   BYTE    bTimeOut2;                // timeout in seconds after first key stroke
   BYTE    bmFormatString;           // formatting options
   BYTE    bmPINBlockString;         // bits 7-4 bit size of PIN length in APDU, bits 3-0 PIN
   // block size in bytes after justification and formatting
   BYTE    bmPINLengthFormat;        // bits 7-5 RFU, bit 4 set if system units are bytes // clear if system units are bits,
   // bits 3-0 PIN length position in system units
   USHORT  wPINMaxExtraDigit;        // XXYY, where XX is minimum PIN size in digits,
   // YY is maximum
   BYTE    bEntryValidationCondition;// Conditions under which PIN entry should be
   // considered complete
   BYTE    bNumberMessage;           // Number of messages to display for PIN verification
   BYTE    wLangId[2];                  // Language for messages
   BYTE    bMsgIndex;                // Message index (should be 00)
   BYTE    bTeoPrologue[3];          // T=1 I-block prologue field to use (fill with 00)
   BYTE    ulDataLength[4];             // length of Data to be sent to the ICC
   BYTE    abData[PP_APDU_MAX_LEN];                // Data to send to the ICC
} PIN_VERIFY_STRUCTURE, *PPIN_VERIFY_STRUCTURE;

//typedef struct PIN_MODIFY_STRUCTURE
//{
//   BYTE    bTimeOut;                 // timeout in seconds (00 means use default timeout)
//   BYTE    bTimeOut2;                // timeout in seconds after first key stroke
//   BYTE    bmFormatString;           // formatting options USB_CCID_PIN_FORMAT_xxx)
//   BYTE    bmPINBlockString;         // bits 7-4 bit size of PIN length in APDU, bits 3-0 PIN
//   // block size in bytes after justification and formatting
//   BYTE bmPINLengthFormat;           // bits 7-5 RFU, bit 4 set if system units are bytes,
//   // clear if system units are bits
//   // bits 3-0 PIN length position in system units
//   // bits, bits 3-0 PIN length position in system units
//   BYTE bInsertionOffsetOld;         // Insertion position offset in bytes for the current PIN
//   BYTE bInsertionOffsetNew;         // Insertion position offset in bytes for the new PIN
//   USHORT wPINMaxExtraDigit;         // XXYY, where XX is minimum PIN size in digits,
//   // YY is maximum
//   BYTE bConfirmPIN;                 // Flags governing need for confirmation of new PIN
//   BYTE bEntryValidationCondition;   // Conditions under which PIN entry should be
//   // considered complete
//   BYTE bNumberMessage;              // Number of messages to display for PIN verification
//   USHORT wLangId;                   // Language for messages
//   BYTE bMsgIndex1;                  // Index of 1st prompting message
//   BYTE bMsgIndex2;                  // Index of 2d prompting message
//   BYTE bMsgIndex3;                  // Index of 3d prompting message
//   BYTE bTeoPrologue[3];             // T=1 I-block prologue field to use (fill with 00)
//   ULONG ulDataLength;               // length of Data to be sent to the ICC
//   BYTE abData[1];                   // Data to send to the ICC
//} PIN_MODIFY_STRUCTURE , *PPIN_MODIFY_STRUCTURE;


SCard::SCard()
{
   hCard = 0;
}

void SCard::beginTransaction()
{
   LONG ret = 0;
   int maxRetry = 10;
   
#ifdef _DEBUG
   if (unittest_Generate_Exception_Transaction_Fail) {
       throw SCardException(0, SCardException_Code::TransactionFail);
   }
#endif

   while (maxRetry--) {
      ret = SCardBeginTransaction(hCard);
      if (ret == 0) {
         break;
      }
      do_sleep(300);
   }
   if (ret != SCARD_S_SUCCESS) {
      log_error("E: Could not start transaction");
      throw SCardException(ret, SCardException_Code::TransactionFail);
   }
}

void SCard::endTransaction()
{
   LONG ret = SCardEndTransaction(hCard, SCARD_LEAVE_CARD);
   if (ret) {
      log_error("SCardEndTransaction failed (%0x)", ret);
   }
}

#define WHERE "SCard::listReaders()"
void SCard::listReaders(std::vector<std::shared_ptr<CardReader>>& readers)
{
#ifdef _DEBUG
   if (unittest_Generate_Exception_No_Reader) {
       throw BeidConnectException(BeidConnectException_Code::no_reader);
   }
#endif
   LPTSTR            szRdr;
   DWORD             dwI = 0;
   DWORD			dwRdrCount = 0;
   SCARD_READERSTATE readerState[MAX_READERS];
   int n_readers;
   LONG              lReturn;
   unsigned int i;
   shared_ptr<SCardCtx> context = std::make_shared<SCardCtx>();

   log_info("SCardListReaders()");
#ifdef _WIN32
   LPTSTR            szReaders = NULL;
   DWORD             cchReaders = SCARD_AUTOALLOCATE;
   lReturn = SCardListReaders(*context,
                              NULL,
                              (LPTSTR)&szReaders,
                              &cchReaders );
#else
   LPTSTR            szReaders = new char[READERS_BUF_SIZE];
   DWORD             cchReaders = READERS_BUF_SIZE;
   lReturn = SCardListReaders(*context,
                              NULL,
                              (LPTSTR)szReaders,
                              &cchReaders );
#endif
   if ( SCARD_S_SUCCESS != lReturn )
   {
      log_error("%s: E: SCardListReaders returned 0x%08x", WHERE, lReturn);
      throw BeidConnectException(BeidConnectException_Code::no_reader);
   }
   
   // Place the readers into the state array.
   szRdr = szReaders;
   memset(&readerState, 0, MAX_READERS * sizeof(SCARD_READERSTATE));   //if we don't do this, we get a 8010001D on RDP
                                                                       //2019-11-07: added MAX_READERS *, remote readers were not always found
   for ( dwI = 0; dwI < MAX_READERS; dwI++ ) {
      if ( 0 == *szRdr )
         break;
      readerState[dwI].szReader = szRdr;
      readerState[dwI].dwCurrentState = SCARD_STATE_UNAWARE;
      readerState[dwI].dwEventState = SCARD_STATE_UNAWARE;
      szRdr += lstrlen(szRdr) + 1;
   }
   dwRdrCount = dwI;
   n_readers = (int)dwI;
   if (n_readers == 0){
       throw BeidConnectException(BeidConnectException_Code::no_reader);
   }
   
//   // Look through the array of readers.
//   for ( dwI=0; dwI < dwRdrCount; dwI++)
//   { /* first time set to unaware */
//      readerState[dwI].dwCurrentState = readerState[dwI].dwEventState;
//   }
   
   //Wait until there is a change.
   lReturn = SCardGetStatusChange(*context,
                                  INFINITE, // infinite wait
                                  readerState,
                                  dwRdrCount );
   if ( SCARD_S_SUCCESS != lReturn )
   {
      log_error("E: SCardGetStatusChange failed (%lx)", lReturn);
      throw BeidConnectException(BeidConnectException_Code::no_reader);
   }
   /* if something has changed, check it */
   for ( dwI=0; dwI < dwRdrCount; dwI++)
   {
      if (readerState[dwI].dwEventState & SCARD_STATE_CHANGED)
      {
         std::shared_ptr<SCard> reader = std::make_shared<SCard>();
         reader->name = readerState[dwI].szReader;
         reader->context = context;
         
         char atr[1024];
         char s[10];
         memset(atr, 0, sizeof(atr));
         for (i = 0; i < readerState[dwI].cbAtr; i++)
         {
            sprintf(s, "%2.2X",readerState[dwI].rgbAtr[i]);
            strcat(atr ,s);
         }
         
         reader->atr = atr;
         log_info("Reader: %s (%s)", readerState[dwI].szReader, atr);
         readers.push_back(reader);
      }
   }
   
#ifdef _WIN32
   SCardFreeMemory( *context, szReaders );
#else
   delete[] szReaders;
#endif
}
#undef WHERE

#define WHERE "SCard::connect()"
long SCard::connect()
{
   long ret = 0;
   if (atr == "") {
        return E_SRC_NO_CARD;
   }
   
   DWORD ActiveProtocol = SCARD_PROTOCOL_UNDEFINED;
   ret = SCardConnect(*context, name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1| SCARD_PROTOCOL_T0, &hCard, &ActiveProtocol);
   if (ret != SCARD_S_SUCCESS) {
       log_error("%s: E: SCardConnect(%s) returned %08X", WHERE, name.c_str(), ret);
       return (ret);
   }

   ioSendPci.dwProtocol = ActiveProtocol;
   ioRecvPci.dwProtocol = ActiveProtocol;
   
   log_info("I: Card connected (proto:%d)", ActiveProtocol);
   return ret;
}
#undef WHERE

#define WHERE "src_disconnect()"
void SCard::disconnect()
{
   long rv = 0;
   
   if (hCard != 0)
   {
      log_info("I: SCardDisconnect()");
      rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD/*SCARD_RESET_CARD*/);
      if ( rv != SCARD_S_SUCCESS )
      {
         log_error("E: SCardDisconnect: rv = %d\n", rv);
      }
      else
      {
         log_info("************* sleeping 1/2s *************");
         do_sleep(500);
         //log_error("I: Card [%s] disconnected\n", pReader->card.a_cType);
      }
      hCard = 0;
   }
}
#undef WHERE

bool SCard::isPinPad()
{
   long status;

   status = getFeatures();
   if (status != SCARD_S_SUCCESS) {
      log_error("E: getFeatures failed %0x", status);
      return false;
   }
   
   return ((cmds.verify_pin_start != 0) || (cmds.verify_pin_direct != 0));
}

#define WHERE "scard::apdu()"
CardAPDUResponse SCard::apdu(const CardAPDU& apdu)
{
    LONG r = 0;
    unsigned char recv[512];
    DWORD l_recv = 512;

    r = SCardTransmit(hCard, &ioSendPci, apdu.GetAPDU().data(), (DWORD)apdu.GetAPDU().size(), &ioRecvPci, recv, &l_recv);
    if (r != SCARD_S_SUCCESS) {
        log_error("%s: E: Failed SCardTransmit(): %d (0x%0x)", WHERE, r, r);
        throw SCardException(r);
    }
    return CardAPDUResponse(recv, l_recv);
}
#undef WHERE

unsigned int load_int4(unsigned char *x)
{
   return((x[0] << 24) + (x[1] << 16) + (x[2] << 8) + x[3]);
}

unsigned long change_endian(unsigned long a)
{
   return (((a & 0xFF) << 24) + ((a & 0xFF00) << 8) + ((a & 0xFF0000) >> 8) + ((a & 0xFF000000) >> 24));
}

unsigned short load_int2(unsigned char *x)
{
   return (unsigned short)((x[0] << 8) + x[1]);
}

bool SCard::getPPDUFeatures()
{
    unsigned char get_feature_list[] = { 0xFF, 0xC2, 0x01, 0x00, 0x00 };
    //add friendlynames of readers that support PPDU over transmit here
    // List retrieved on eid-mw (18/7/2022)
    if ((name.find("VASCO DIGIPASS 870") == 0) ||
        (name.find("VASCO DIGIPASS 875") == 0) ||
        (name.find("VASCO DIGIPASS 920") == 0) ||
        (name.find("VASCO DIGIPASS 840") == 0) ||
        (name.find("VASCO DIGIPASS 876") == 0) ||
        (name.find("Gemalto ING Shield Pro") == 0) ||
        (name.find("ETSWW eKrypto PINPhab") == 0) ||
        (name.find("ETSWW eKrypto PINPad") == 0) ||
        (name.find("DIOSS pinpad") == 0))
    {
        try
        {
            CardAPDUResponse r = apdu(CardAPDU(&get_feature_list[0], sizeof(get_feature_list)));

            // every byte represents a feature, except the last 2 bytes (SW1, SW2)
            for(DWORD i=0; i < r.getDataLen(); i++)
            {
                switch (r.getDataAtPos(i)) {
                case FEATURE_MODIFY_PIN_START:
                    cmds.modify_pin_start = r.getDataAtPos(i);
                    m_bCanUsePPDU = true;
                    break;
                case FEATURE_MODIFY_PIN_FINISH: cmds.modify_pin_start = r.getDataAtPos(i); break;
                case FEATURE_VERIFY_PIN_START:
                    cmds.verify_pin_start = r.getDataAtPos(i);
                    m_bCanUsePPDU = true;
                    break;
                case FEATURE_VERIFY_PIN_FINISH: cmds.verify_pin_finish = r.getDataAtPos(i); break;
                case FEATURE_VERIFY_PIN_DIRECT:
                    cmds.verify_pin_direct = r.getDataAtPos(i);
                    m_bCanUsePPDU = true;
                    break;
                case FEATURE_MODIFY_PIN_DIRECT:
                    cmds.modify_pin_direct = r.getDataAtPos(i);
                    m_bCanUsePPDU = true;
                    break;
                case FEATURE_GET_KEY_PRESSED:   cmds.get_key_pressed = r.getDataAtPos(i); break;
                case FEATURE_MCT_READERDIRECT:  cmds.mct_readerdirect = r.getDataAtPos(i); break;
                case FEATURE_MCT_UNIVERSAL:     cmds.mct_universal = r.getDataAtPos(i); break;
                case FEATURE_IFD_PIN_PROP:      cmds.ifd_pin_prop = r.getDataAtPos(i); break;
                case FEATURE_ABORT:             cmds.abort = r.getDataAtPos(i); break;
                }
            }
            return true;

        }
        catch(...){}
    }
    return false;
}

#define WHERE "get_features()"
long SCard::getFeatures()
{
    if (!FeaturesRetrieved) {
        if (!getPPDUFeatures()) {
            cmds.clear();
            long           ret;
            unsigned char buf[512];
            DWORD         rcv_len = 512;

            ret = SCardControl(hCard,        //XXXXX get handle without card if no card present!!!!!
                CM_IOCTL_GET_FEATURE_REQUEST,
                NULL,
                0,
                buf,
                sizeof(buf),
                &rcv_len);

            if (ret == SCARD_S_SUCCESS && (rcv_len % 6) == 0 && rcv_len > 0) {

                /* buf is a TLV structure: length always 4 bytes */
                unsigned char* p = buf;
                while (rcv_len >= 6 && p[1] == 4) {
                    switch (p[0]) {
                    case FEATURE_VERIFY_PIN_START:   cmds.verify_pin_start = load_int4(p + 2); break;
                    case FEATURE_VERIFY_PIN_FINISH:  cmds.verify_pin_finish = load_int4(p + 2); break;
                    case FEATURE_MODIFY_PIN_START:   cmds.modify_pin_start = load_int4(p + 2); break;
                    case FEATURE_MODIFY_PIN_FINISH:  cmds.modify_pin_start = load_int4(p + 2); break;
                    case FEATURE_GET_KEY_PRESSED:    cmds.get_key_pressed = load_int4(p + 2); break;
                    case FEATURE_VERIFY_PIN_DIRECT:  cmds.verify_pin_direct = load_int4(p + 2); break;
                    case FEATURE_MODIFY_PIN_DIRECT:  cmds.modify_pin_direct = load_int4(p + 2); break;
                    case FEATURE_MCT_READERDIRECT:   cmds.mct_readerdirect = load_int4(p + 2); break;
                    case FEATURE_MCT_UNIVERSAL:      cmds.mct_universal = load_int4(p + 2); break;
                    case FEATURE_IFD_PIN_PROP:       cmds.ifd_pin_prop = load_int4(p + 2); break;
                    case FEATURE_ABORT:              cmds.abort = load_int4(p + 2); break;
                    }
                    rcv_len -= 6;
                    p += 6;
                }
            }
            else {
                // very likely CCID_IOCTL_GET_FEATURE_REQUEST isn't supported by this reader -> try via PPDU
                // Some readers (like Vasco DP870) don't implement the SCardControl(CM_IOCTL_GET_FEATURE_REQUEST)
                // but instead intercept an SCardTransmit() and return a buffer containing the features -- but with 1 byte
                // for each feature unlike a 6 byte long TLV per feature for the SCardControl() return buffer.
                getPPDUFeatures();
            }
        }
        FeaturesRetrieved = true;
    }

   return SCARD_S_SUCCESS; //we should always return 0 here since scardcontrol might not be supported! ! ! !
}
#undef WHERE

inline void ToUchar4(size_t ulIn, unsigned char* pucOut4)
{
    pucOut4[0] = (unsigned char)(ulIn % 256);
    ulIn /= 256;
    pucOut4[1] = (unsigned char)(ulIn % 256);
    ulIn /= 256;
    pucOut4[2] = (unsigned char)(ulIn % 256);
    pucOut4[3] = (unsigned char)(ulIn / 256);
}

#define WHERE "SCard::verify_pinpad()"
void SCard::verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, uint16_t PINMaxExtraDigit, const unsigned char pinAPDU[], size_t l_pinAPDU, uint16_t* sw)
{
   LONG             ret;
   unsigned char    send_buf[512] = { 0 };
   DWORD            send_buf_len;
   unsigned char    rcv_buf[512] = { 0 };
   DWORD            rcv_len = 512;
   unsigned int     verify_pin_cmd;

   PIN_VERIFY_STRUCTURE pin_verify;

   if (!isPinPad()) {
         log_error("reader is not a pinpad reader");
         throw BeidConnectException(BeidConnectException_Code::Not_PinPad);
   }

#ifdef _DEBUG
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
   if (unittest_Generate_Exception_Src_Command_not_allowed) {
       throw CardException(0x6985);
   }
   if (unittest_Generate_Exception_PinPad_TimeOut) {
       throw CardException(0x6400);
   }
   if (unittest_Generate_Exception_PinPad_Cancel) {
       throw CardException(0x6401);
   }
   if (unittest_Generate_Exception_Other) {
       throw CardException(0x9999);
   }
#endif

   pin_verify.bTimeOut = 0x20;
   pin_verify.bTimeOut2 = 0x30;
   pin_verify.bmFormatString = format;
   pin_verify.bmPINBlockString = PINBlock;
   pin_verify.bmPINLengthFormat = (BYTE)PINLength;
   pin_verify.wPINMaxExtraDigit = PINMaxExtraDigit;//0x0408; //0x0804; //(0x0408 = Min Max  => max min = 0x0804 */
   pin_verify.bEntryValidationCondition = 0x02;	/* 1=max size reached, 2=ok button pressed, 4=timeout */
   pin_verify.bNumberMessage = 0x01;//spr532=>0x00; //0x01;
   USHORT ulLangCode;
   switch (language){
      case LG_DUTCH:
          ulLangCode = 0x0813;// 0x0813 Dutch (Belgium)          XX13  is for dutch (0x0413 Dutch (Netherlands))
         break;
      case LG_FRENCH:
          ulLangCode = 0x040C;// 0x040c French (Standard)        XX0c  is for French
         break;
      case LG_GERMAN:
          ulLangCode = 0x0407;// 0x0407 German (Standard)        XX07  is for German
         break;
      default: /* ENGLISH */
          ulLangCode = 0x0409;// 0x0409 English (United States)  XX09  is for English
   }
   pin_verify.wLangId[0] = (unsigned char)((ulLangCode & 0xff00) / 256);
   pin_verify.wLangId[1] = (unsigned char)(ulLangCode & 0xff);
   pin_verify.bMsgIndex = 0x00;
   pin_verify.bTeoPrologue[0] = 0x00;
   pin_verify.bTeoPrologue[1] = 0x00;
   pin_verify.bTeoPrologue[2] = 0x00;
   ToUchar4(l_pinAPDU, pin_verify.ulDataLength);
   memcpy(pin_verify.abData, pinAPDU, l_pinAPDU);
   send_buf_len = sizeof(PIN_VERIFY_STRUCTURE) + (DWORD)l_pinAPDU - PP_APDU_MAX_LEN;	/* -PP_APDU_MAX_LEN because PIN_VERIFY_STRUCTURE contains the PP_APDU_MAX_LEN byte of abData[] */

   if (!m_bCanUsePPDU) {
       if (cmds.verify_pin_direct != 0) {
           log_info("I: verify_pin_direct SCardControl\n");
           ret = SCardControl(hCard, cmds.verify_pin_direct, &pin_verify, send_buf_len, rcv_buf, sizeof(rcv_buf), &rcv_len);
           if (ret != SCARD_S_SUCCESS) {
               /* In case of a parameter error of the passed structure, the ScardControl may return ERROR_INVALID_PARAMETER (0x57). */
               log_error("E: scardControl returned 0x%08x\n", ret);
               throw SCardException(ret);
           }
       }
       else {
           log_info("I: verify_pin_start SCardControl\n");
           ret = SCardControl(hCard, cmds.verify_pin_start, &pin_verify, send_buf_len, rcv_buf, sizeof(rcv_buf), &rcv_len);
           if (ret != SCARD_S_SUCCESS) {
               /* In case of a parameter error of the passed structure, the ScardControl may return ERROR_INVALID_PARAMETER (0x57). */
               log_error("E: scardControl returned 0x%08x\n", ret);
               throw SCardException(ret);
           }
           bool ScanKeyPressed = true;
           while (ScanKeyPressed) {
               ret = SCardControl(hCard, cmds.get_key_pressed, NULL, 0, rcv_buf, sizeof(rcv_buf), &rcv_len);
               if (ret != SCARD_S_SUCCESS)
               {
                   log_error("E: scardControl returned 0x%08x\n", ret);
                   throw SCardException(ret);
               }
               switch (rcv_buf[0]) {
               case 0x00:
                   // No key 
                   std::this_thread::sleep_for(chrono::milliseconds(200)/*200ms*/);
                   break;
               case 0x0d:
                   log_info("I: get_key_pressed Ok\n", ret);
                   // OK button
                   ScanKeyPressed = false;
                   break;
               case 0x1b:
                   log_info("I: get_key_pressed Cancel\n", ret);
                   // Cancel button
                   ScanKeyPressed = false;
                   break;
               case 0x40:
                   log_info("I: get_key_pressed Aborted/timeout\n", ret);
                   // Aborted/timeout
                   ScanKeyPressed = false;
                   break;
               case 0x2b:
                   log_info("I: get_key_pressed\n", ret);
                   // 0-9
                   break;
               case 0x08:
                   log_info("I: get_key_pressed Backspace\n", ret);
                   // Backspace
                   break;
               case 0x0a:
                   log_info("I: get_key_pressed Clear\n", ret);
                   // Clear
                   break;
               default:
                   log_info("I: get_key_pressed Unknow\n", ret);
                   //printf("Key pressed: 0x%x\n", bRecvBuffer[0]);
                   ;
               }
           }
           ret = SCardControl(hCard, cmds.verify_pin_finish, NULL, 0, rcv_buf, sizeof(rcv_buf), &rcv_len);
           log_error("E: scardControl returned 0x%08x\n", ret);
           if (ret != SCARD_S_SUCCESS)
           {
               log_error("E: scardControl returned 0x%08x\n", ret);
               throw SCardException(ret);
           }
           if ((rcv_len >= 2) /*&&
               ((rcv_buf[rcv_len - 2]) != 90)*/)
           {
               *sw = (uint16_t)((rcv_buf[rcv_len - 2] << 8) + (rcv_buf[rcv_len - 1] & 0xFF));
           }
           if (*sw != 0x9000)
           {
               log_error("E: scardControl verify_pin_finish returned 0x%04x\n", *sw);
               throw CardException(*sw);
           }
       }
       return;
   }
   else
   {
       verify_pin_cmd = cmds.verify_pin_direct;
       if (verify_pin_cmd == 0) {
           verify_pin_cmd = cmds.verify_pin_start;
           log_info("I: verify_pin_start SCardTransmit\n");
       }
       else {
           log_info("I: verify_pin_direct SCardTransmit\n");
       }
       // Convert send_buf into a PPDU and send it with SCardTransmit():  FF C2 01 <tag> <send_len> <send_buf>

       memcpy(&send_buf[5], &pin_verify, send_buf_len);
       //for (int i = (int)send_buf_len; i > 0; i--)
       //  send_buf[5 + i] = ((unsigned char*)& pin_verify)[i];

      send_buf[0] = 0xff; send_buf[1] = 0xc2; send_buf[2] = 0x01; send_buf[3] = (unsigned char)verify_pin_cmd; send_buf[4] = (unsigned char)send_buf_len;
      send_buf_len += 5;

      rcv_len = (DWORD) sizeof(rcv_buf);
      ret = SCardTransmit(hCard, &ioSendPci, send_buf, send_buf_len, &ioRecvPci, rcv_buf, &rcv_len);
      if (ret != SCARD_S_SUCCESS)
      {
         /* In case of a parameter error of the passed structure, the ScardControl may return ERROR_INVALID_PARAMETER (0x57). */
         log_error("E: scardTransmit(pinpad command) returned 0x%08x\n", ret);
         throw SCardException(ret);
      }
   }

   if ((rcv_len >= 2) /*&&
       ((rcv_buf[rcv_len - 2]) != 90)*/)
   {
      *sw = (uint16_t)((rcv_buf[rcv_len - 2] << 8) + (rcv_buf[rcv_len - 1] & 0xFF));
   }

   switch (*sw)
   {
      case 0x9000:
         ret = 0;
         break;
      case 0x63C3:  // E_PIN_3_ATTEMPTS;
      case 0x63C2:  // E_PIN_2_ATTEMPTS;
      case 0x63C1:  // E_PIN_1_ATTEMPT;
      case 0x6983:  // E_PIN_BLOCKED;
      case 0x6400:  // SPE operation timed out // E_PIN_TIMEOUT;
      case 0x6401:  // SPE operation was cancelled by the Cancel button // E_PIN_CANCELLED;
      case 0x6402:  // Modify PIN operation failed because two "new PIN" entries do not match // E_PIN_INCORRECT;
      case 0x6403:  // User entered too short or too long PIN regarding MIN/MAX PIN Length
                    // Note : as this error code is not known by CT-API implementations, it should be mapped to 64 01 on CT - API leve
                    // E_PIN_LENGTH;
          log_error("%s: E: Card returns SW(%04X)", WHERE, *sw);
          throw CardException(*sw);
          break;
      case 0x6B80:  // invalid parameter in passed structure
      case 0x6A86:  // Incorrect value for P2
          log_error("%s: E: Card returns SW(%04X)", WHERE, *sw);
          throw CardException(*sw);
      default: ret = E_PIN_INCORRECT;
          log_error("%s: E: Card returns SW(%04X)", WHERE, *sw);
          throw CardException(*sw, CardException_Code::PIN_Incorrect);
   }
}
#undef WHERE
