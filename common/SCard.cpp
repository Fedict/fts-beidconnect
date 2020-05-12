#include "SCard.hpp"
#include "Card.hpp"
#include "CardFactory.hpp"
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "log.hpp"
#include "general.h"
#include "util.h"

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
   USHORT  wLangId;                  // Language for messages
   BYTE    bMsgIndex;                // Message index (should be 00)
   BYTE    bTeoPrologue[3];          // T=1 I-block prologue field to use (fill with 00)
   ULONG   ulDataLength;             // length of Data to be sent to the ICC
   BYTE    abData[1];                // Data to send to the ICC
} PIN_VERIFY_STRUCTURE, *PPIN_VERIFY_STRUCTURE;



typedef struct PIN_MODIFY_STRUCTURE
{
   BYTE    bTimeOut;                 // timeout in seconds (00 means use default timeout)
   BYTE    bTimeOut2;                // timeout in seconds after first key stroke
   BYTE    bmFormatString;           // formatting options USB_CCID_PIN_FORMAT_xxx)
   BYTE    bmPINBlockString;         // bits 7-4 bit size of PIN length in APDU, bits 3-0 PIN
   // block size in bytes after justification and formatting
   BYTE bmPINLengthFormat;           // bits 7-5 RFU, bit 4 set if system units are bytes,
   // clear if system units are bits
   // bits 3-0 PIN length position in system units
   // bits, bits 3-0 PIN length position in system units
   BYTE bInsertionOffsetOld;         // Insertion position offset in bytes for the current PIN
   BYTE bInsertionOffsetNew;         // Insertion position offset in bytes for the new PIN
   USHORT wPINMaxExtraDigit;         // XXYY, where XX is minimum PIN size in digits,
   // YY is maximum
   BYTE bConfirmPIN;                 // Flags governing need for confirmation of new PIN
   BYTE bEntryValidationCondition;   // Conditions under which PIN entry should be
   // considered complete
   BYTE bNumberMessage;              // Number of messages to display for PIN verification
   USHORT wLangId;                   // Language for messages
   BYTE bMsgIndex1;                  // Index of 1st prompting message
   BYTE bMsgIndex2;                  // Index of 2d prompting message
   BYTE bMsgIndex3;                  // Index of 3d prompting message
   BYTE bTeoPrologue[3];             // T=1 I-block prologue field to use (fill with 00)
   ULONG ulDataLength;               // length of Data to be sent to the ICC
   BYTE abData[1];                   // Data to send to the ICC
} PIN_MODIFY_STRUCTURE , *PPIN_MODIFY_STRUCTURE;


SCard::SCard()
{
   hCard = 0;
}

int SCard::beginTransaction()
{
   int ret = 0;
   int maxRetry = 10;
   
   while (maxRetry--) {
      ret = SCardBeginTransaction(hCard);
      if (ret == 0) {
         break;
      }
      do_sleep(300);
   }
   if (ret) {
      log_error("E: Could not start transaction");
      return(E_SRC_START_TRANSACTION);
   }
   return ret;
}

int SCard::endTransaction()
{
   int ret = SCardEndTransaction(hCard, SCARD_LEAVE_CARD);
   if (ret) {
      log_error("SCardEndTransaction failed (%0x)", ret);
   }
   return 0;
}


#define WHERE "SCard::listReaders()"
int SCard::listReaders(std::vector<CardReader::Ptr> & readers)
{
   LPTSTR            szRdr;
   DWORD             dwI = 0;
   DWORD			dwRdrCount = 0;
   SCARD_READERSTATE readerState[MAX_READERS];
   int n_readers;
   LONG              lReturn;
   unsigned int i;
   int ret = 0;
   SCardCtx::Ptr context = std::make_shared<SCardCtx>();

   if (!context->valid) {
      return E_SRC_NO_CONTEXT;
   }
   
   log_info("SCardListReaders()");
#ifdef _WIN32
   LPTSTR            szReaders = NULL;
   DWORD             cchReaders = SCARD_AUTOALLOCATE;
   lReturn = SCardListReaders(context->hSC(),
                              NULL,
                              (LPTSTR)&szReaders,
                              &cchReaders );
#else
   LPTSTR            szReaders = new char[READERS_BUF_SIZE];
   DWORD             cchReaders = READERS_BUF_SIZE;
   lReturn = SCardListReaders(context->hSC(),
                              NULL,
                              (LPTSTR)szReaders,
                              &cchReaders );
#endif
   if ( SCARD_S_SUCCESS != lReturn )
   {
      log_error("%s: E: SCardListReaders returned 0x%08x", WHERE, lReturn);
      return(E_SRC_NO_READERS_FOUND);
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
      return(E_SRC_NO_READERS_FOUND);
   }
   
//   // Look through the array of readers.
//   for ( dwI=0; dwI < dwRdrCount; dwI++)
//   { /* first time set to unaware */
//      readerState[dwI].dwCurrentState = readerState[dwI].dwEventState;
//   }
   
   //Wait until there is a change.
   lReturn = SCardGetStatusChange(context->hSC(),
                                  INFINITE, // infinite wait
                                  readerState,
                                  dwRdrCount );
   if ( SCARD_S_SUCCESS != lReturn )
   {
      log_error("E: SCardGetStatusChange failed (%lx)", lReturn);
      return(E_SRC_NO_READERS_FOUND);
   }
   /* if something has changed, check it */
   for ( dwI=0; dwI < dwRdrCount; dwI++)
   {
      if (readerState[dwI].dwEventState & SCARD_STATE_CHANGED)
      {
         SCard::Ptr reader = std::make_shared<SCard>();
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
   SCardFreeMemory( context->hSC(), szReaders );
#else
   delete[] szReaders;
#endif
   
   return (ret);
}
#undef WHERE


#define WHERE "SCard::connect()"
int SCard::connect()
{
   int ret = 0;
   DWORD dwProto = -1;
   SCARDCONTEXT hSC = context->hSC();// SingletonSCard::getInstance().hSC;
   if (atr == "") {
        return E_SRC_NO_CARD;
     }
   
   ret = SCardConnect(hSC,
                      name.c_str(),
                      SCARD_SHARE_SHARED,
                      SCARD_PROTOCOL_T1,
                      &hCard,
                      &dwProto);
   if (ret != SCARD_S_SUCCESS) {
      
      log_info("E: SCardConnect(T1) returned 0x%08x", ret);
      ret = SCardConnect(hSC, name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, &hCard, &dwProto);
      if (ret != SCARD_S_SUCCESS) {
         log_error("E: SCardConnect(T0) returned 0x%08x", ret);
         return (ret);
      }
   }
   
   log_info("I: Card connected (proto:%d)", dwProto);
      switch ( dwProto )
   {
      case SCARD_PROTOCOL_T0:
         cardIsT1 = 0;
         break;
         
      case SCARD_PROTOCOL_T1:
         cardIsT1 = 1;
         break;
         
      default:
         
         break;
   }
   
   return (ret);
}
#undef WHERE

#define WHERE "src_disconnect()"
int SCard::disconnect()
{
   int rv = 0;
   
   if (hCard != 0)
   {
      log_info("I: SCardDisconnect()");
      rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD/*SCARD_RESET_CARD*/);
      if ( rv != SCARD_S_SUCCESS )
      {
         log_error("E: SCardDisconnect: rv = %d\n", rv);
         return (rv);
      }
      else
      {
         log_info("************* sleeping 1/2s *************");
         do_sleep(500);
         //log_error("I: Card [%s] disconnected\n", pReader->card.a_cType);
      }
      hCard = 0;
   }
   
   return (rv);
}
#undef WHERE


bool SCard::isPinPad()
{
   int status;
   bool ret;

   status = getFeatures();
   if (status) {
      log_error("E: getFeatures failed %0x", status);
      CLEANUP(false);
   }
   
   if ((cmds.verify_pin_start != 0) || (cmds.verify_pin_direct != 0)) {
      CLEANUP(true);
   }
   else {
      CLEANUP(false);
   }
   
cleanup:
   
   return ret;
}


#define WHERE "scard::apdu()"
int SCard::apdu(const unsigned char *apdu, unsigned int l_apdu, unsigned char *out, int *l_out, int *sw)
{
   int ret = 0;
   unsigned char recv[512];
   DWORD l_recv = 512;
   
   memset(out, 0, *l_out);
   
   ret = SCardTransmit(hCard,
                       ((cardIsT1 == 1)? SCARD_PCI_T1 : SCARD_PCI_T0),
                       apdu, l_apdu, NULL, recv, &l_recv);
   if ( ret != SCARD_S_SUCCESS ) {
      log_error("%s: E: Failed SCardTransmit(): %d (0x%0x)", WHERE, ret, ret);
      return(ret);
   }
   
   if (*l_out >= (int) l_recv)
   {
      if (l_recv >= 2)
         *l_out = l_recv - 2;
      else
         *l_out = l_recv;
   }
   
   if (*l_out > 0)
      memcpy(out, recv, *l_out);
   
   *sw = (recv[l_recv - 2] << 8) + recv[l_recv - 1];
   
   return (ret);
}
#undef WHERE


/**
 * For these pinpad reader, an SCardTransmit(FEATURES) is done when an SCardControl() fails on Windows.
 * It should NOT be done for other readers who can/will get stuck when send the SCardTransmit(FEATURES).
 * Below is the currently known list of such pinpad readers:
 */
const char *PINPAD_READERS_WITH_TRANSMIT[] = {
   "DIGIPASS 870",
   "DIGIPASS 875",
   "DIGIPASS 920",
   // Add similar pinpad readers here
   NULL
};

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
   return((x[0] << 8) + x[1]);
}

/** Check if pReader matches with any of the pinpad readers listed in the PINPAD_READERS_WITH_TRANSMIT table */
int SCard::isPinpadWithTransmit(const char *readerName)
{
   const char **row = PINPAD_READERS_WITH_TRANSMIT;
   
   for (; NULL != *row; row++) {
      if (NULL != strstr(readerName, *row)) {
         log_info("Special pinpad reader that works with an SCardTransmit()");
         return 1; // reader found in the PINPAD_READERS_WITH_TRANSMIT table
      }
   }
   
   return 0; // not found
}

#define WHERE "get_features()"
int SCard::getFeatures()
{
   int           ret;
   unsigned char buf[512];
   unsigned char *p;
   DWORD         rcv_len = 512;
   
#ifdef _WIN32
   int      sw;
   unsigned char get_feature_list[] = "\xFF\xC2\x01\x00\x00";
   
   if (isPinpadWithTransmit(name.c_str())) {
      // On Windows, some readers (like Vasco DP870) don't implement the SCardControl(CM_IOCTL_GET_FEATURE_REQUEST)
      // but instead intercept an SCardTransmit() and return a buffer containing the features -- but with 1 byte
      // for each feature unlike a 6 byte long TLV per feature for the SCardControl() return buffer.
      ret = apdu(get_feature_list, sizeof(get_feature_list) - 1, buf, (int*)&rcv_len, &sw);
      if (ret) {
         log_error("SCardTransmit(get_features) failed on reader that should support this (0x%0X)", ret);
         return(ret);
      }
      else {
         // every byte represents a feature, except the last 2 bytes (SW1, SW2)
         p = buf;
         while (rcv_len > 0) {
            switch (p[0]) {
               case FEATURE_VERIFY_PIN_START:   cmds.verify_pin_start = *p; break;
               case FEATURE_VERIFY_PIN_FINISH:  cmds.verify_pin_finish = *p; break;
               case FEATURE_MODIFY_PIN_START:   cmds.modify_pin_start = *p; break;
               case FEATURE_MODIFY_PIN_FINISH:  cmds.modify_pin_start = *p; break;
               case FEATURE_GET_KEY_PRESSED:    cmds.get_key_pressed = *p; break;
               case FEATURE_VERIFY_PIN_DIRECT:  cmds.verify_pin_direct = *p; break;
               case FEATURE_MODIFY_PIN_DIRECT:  cmds.modify_pin_direct = *p; break;
               case FEATURE_MCT_READERDIRECT:   cmds.mct_readerdirect = *p; break;
               case FEATURE_MCT_UNIVERSAL:      cmds.mct_universal = *p; break;
               case FEATURE_IFD_PIN_PROP:       cmds.ifd_pin_prop = *p; break;
               case FEATURE_ABORT:              cmds.abort = *p; break;
               case 0x80:
                  //pReader->cmd.fedictVersion          = load_int4(p+2)+1; break;  //I count 1 up to make sure we don't have a 0 here
                  break;
               default:;
            }
            
            rcv_len--;
            p++;
         } //end while
         return (ret);
      }
      return (ret);
   }
#endif  //end #ifdef WIN32
   
   
   ret = SCardControl(hCard,        //XXXXX get handle without card if no card present!!!!!
                      CM_IOCTL_GET_FEATURE_REQUEST,
                      NULL,
                      0,
                      buf,
                      sizeof(buf),
                      &rcv_len);
   
   if (ret == 0) {
      
      /* buf is a TLV structure: length always 4 bytes */
      p = buf;
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
            case 0x80:
               //pReader->cmd.fedictVersion          = load_int4(p+2)+1; break;  //I count 1 up to make sure we don't have a 0 here
               break;
            default:;
         } //end switch
         
         rcv_len -= 6;
         p += 6;
      } //end while
   } //end if
   
   return 0;
   //return (ret); //we should always return 0 here since scardcontrol might not be supported! ! ! !
}
#undef WHERE



#define WHERE "SCard::verify_pinpad()"
int SCard::verify_pinpad(unsigned char format, unsigned char PINBlock, unsigned char PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], int l_pinAPDU, int *sw)
{
   int           ret;
   unsigned char send_buf[512];
   unsigned char rcv_buf[512];
   unsigned int  send_len;
   DWORD         rcv_len = 512;
   unsigned int verify_pin_cmd;
   int len = 0;

   PIN_VERIFY_STRUCTURE *pin_verify;

   if (!isPinPad()) {
         log_error("reader is not a pinpad reader");
         return(1);
   }
   
   verify_pin_cmd = cmds.verify_pin_direct;
   if (verify_pin_cmd == 0)
      verify_pin_cmd = cmds.verify_pin_start;
   
   pin_verify = (PIN_VERIFY_STRUCTURE *)&send_buf[0];

   pin_verify->bmFormatString = format;      // 0x82;  //82=ascii
   pin_verify->bmPINBlockString = PINBlock;  //0x08; //0x04;
   pin_verify->bmPINLengthFormat = PINLength;//0x00;

   pin_verify->wPINMaxExtraDigit = PINMaxExtraDigit;//0x0408; //0x0804; //(0x0408 = Min Max  => max min = 0x0804 */

   memcpy(pin_verify->abData, pinAPDU, l_pinAPDU);
   len = pin_verify->ulDataLength = l_pinAPDU;

   pin_verify->bEntryValidationCondition = 0x06;	/* 1=max size reached, 2=ok button pressed, 4=timeout */
   pin_verify->bTimeOut = 0x20;
   pin_verify->bTimeOut2 = 0x00;

   pin_verify->bNumberMessage = 0x01;//spr532=>0x00; //0x01;
   
   switch (language){
      case LG_DUTCH:
         pin_verify->wLangId = 0x0813;
         break;
      case LG_FRENCH:
         pin_verify->wLangId = 0x040C;
         break;
      case LG_GERMAN:
         pin_verify->wLangId = 0x0407;
         break;
      default: /* ENGLISH */
         pin_verify->wLangId = 0x0409;
   }
   
   pin_verify->bMsgIndex = 0x00;
   pin_verify->bTeoPrologue[0] = 0x00;
   pin_verify->bTeoPrologue[1] = 0x00;
   pin_verify->bTeoPrologue[2] = 0x00;

   //	0x0409 English (United States)    XX09   is for English
   //  0x040c French (Standard)             XX0c  is for French
   //  0x0407 German (Standard)         XX07  is for German

   //  0x0813 Dutch (Belgium)          XX13   13 is for dutch
   //  0x0413 Dutch (Netherlands)

   send_len = sizeof(PIN_VERIFY_STRUCTURE) + len - 1;	/* -1 because PIN_VERIFY_STRUCTURE contains the first byte of abData[] */

   if (!isPinpadWithTransmit(name.c_str())) {

      if ((ret = SCardControl(hCard,
                              verify_pin_cmd,
                              send_buf,
                              send_len,
                              rcv_buf,
                              sizeof(rcv_buf),
                              &rcv_len)) != 0) {
//         /* In case of a parameter error of the passed structure, the ScardControl may return ERROR_INVALID_PARAMETER (0x57). */
         log_error("E: scardControl returned 0x%08x\n", ret);
         return(ret);
      }
   }
   else {

      // Convert send_buf into a PPDU and send it with SCardTransmit():  FF C2 01 <tag> <send_len> <send_buf>
      unsigned char tag = (unsigned char)verify_pin_cmd;

      for (int i = (int)send_len; i > 0; i--)
         send_buf[5 + i] = send_buf[i];

      send_buf[0] = 0xff; send_buf[1] = 0xc2; send_buf[2] = 0x01; send_buf[3] = tag; send_buf[4] = (unsigned char)send_len;
      send_len += 5;

      rcv_len = (DWORD) sizeof(rcv_buf);
      if ((ret = SCardTransmit(hCard,
                               ((cardIsT1) ? SCARD_PCI_T1 : SCARD_PCI_T0),
                               send_buf,
                               send_len,
                               NULL,
                               rcv_buf,
                               &rcv_len)) != 0)
      {
         /* In case of a parameter error of the passed structure, the ScardControl may return ERROR_INVALID_PARAMETER (0x57). */
         log_error("E: scardTransmit(pinpad command) returned 0x%08x\n", ret);
         return(ret);
      }
   }

   if ((rcv_len == 2) &&
       ((rcv_buf[0]) != 90))
   {
      *sw = (rcv_buf[0] << 8) + (rcv_buf[1] & 0xFF);
   }

   switch (*sw)
   {
      case 0x9000:
         ret = 0;
         break;
      case 0x63C3:
         ret = E_PIN_3_ATTEMPTS;
         break;
      case 0x63C2:
         ret = E_PIN_2_ATTEMPTS;
         break;
      case 0x63C1:
         ret = E_PIN_1_ATTEMPT;
         break;
      case 0x6983:
         ret = E_PIN_BLOCKED;
         break;
      case 0x6400:
         ret = E_PIN_TIMEOUT;
         break;
      case 0x6401:
         ret = E_PIN_CANCELLED;
         break;
      case 0x6403:
         ret = E_PIN_LENGTH;
         break;
      default: ret = E_PIN_INCORRECT;
   }

//cleanup:

   return (ret);
}
#undef WHERE




