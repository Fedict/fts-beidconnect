#ifndef SCard_hpp
#define SCard_hpp

#ifndef _WIN32
#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>
#else
#include <winscard.h>
#endif

#include "CardReader.hpp"
#include <vector>
#include <memory>

class SCardCtx
{
public:
   SCardCtx() {
      h = 0;
      valid = false;
      LONG ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &h);
      if (ret) {
         log_error("SCardEstablishContext() returned %0x", ret);
         log_error("Smartcard service not running?");
      }
      valid = true;
   };
   ~SCardCtx() {
      LONG lReturn = SCardReleaseContext(h);
      if ( SCARD_S_SUCCESS != lReturn )
      {
         log_error("E: Failed SCardReleaseContext");
      }
      else {
         //log_info("SCardReleaseContext ok");
      }
   };
   operator SCARDCONTEXT() { return h; };
   bool valid;
private:
   SCARDCONTEXT  h;
};

class SCard: public CardReader
{
   long beginTransaction() override;
   long endTransaction() override;
public:
   SCard();
   virtual ~SCard() { disconnect(); };
   static int listReaders(std::vector<std::shared_ptr<CardReader>> &readers);
   long connect() override;
   long disconnect() override;
   bool isPinPad() override;
   long apdu(const unsigned char *apdu, size_t l_apdu, unsigned char *out, size_t*l_out, int *sw) override;
   CardAPDUResponse apdu2(const CardAPDU& apdu) override;
   long verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], size_t l_pinAPDU, int *sw) override;

   shared_ptr<SCardCtx> context;
   
private:
   long getFeatures();
   bool getPPDUFeatures();
   class commands
   {
      public:
         unsigned int verify_pin_start = 0;
         unsigned int verify_pin_finish = 0;
         unsigned int modify_pin_start = 0;
         unsigned int modify_pin_finish = 0;
         unsigned int get_key_pressed = 0;
         unsigned int verify_pin_direct = 0;
         unsigned int modify_pin_direct = 0;
         unsigned int mct_readerdirect = 0;
         unsigned int mct_universal = 0;
         unsigned int ifd_pin_prop = 0;
         unsigned int abort = 0;
   } cmds;
   bool FeaturesRetrieved = false;
   bool m_bCanUsePPDU = false;
   SCARDHANDLE     hCard;
   SCARD_IO_REQUEST  ioSendPci = { 1, sizeof(SCARD_IO_REQUEST) };
   SCARD_IO_REQUEST  ioRecvPci = { 1, sizeof(SCARD_IO_REQUEST) };
};

#endif /* SCard_hpp */
