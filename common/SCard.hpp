#ifndef SCard_hpp
#define SCard_hpp

#ifndef WIN32
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
   typedef shared_ptr<SCardCtx> Ptr;
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
   SCARDCONTEXT hSC() { return h; };
   bool valid;
private:
   SCARDCONTEXT  h;
};


class SCard: public CardReader
{
public:
   SCard();
   virtual ~SCard(){};
   typedef std::shared_ptr<SCard> Ptr;
   static int listReaders(std::vector<CardReader::Ptr> &readers);
   int open() override;
   int close() override;
   int beginTransaction() override;
   int endTransaction() override;
   int connect() override;
   int disconnect() override;
   bool isPinPad() override;
   int apdu(const unsigned char *apdu, unsigned int l_apdu, unsigned char *out, int *l_out, int *sw) override;
   int verify_pinpad(unsigned char format, unsigned char PINBlock, unsigned char PINLength, unsigned int PINMaxExtraDigit, unsigned char pinAPDU[], int l_pinAPDU, int *sw) override;

   SCardCtx::Ptr context;
   
private:
   int isPinpadWithTransmit(const char *readerName);
   int getFeatures();
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
   SCARDHANDLE     hCard;
   int cardIsT1;
};

#endif /* SCard_hpp */
