#pragma once

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
        LONG ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &h);
        if (SCARD_S_SUCCESS != ret) {
            log_error("SCardEstablishContext() returned %0x", ret);
            log_error("Smartcard service not running?");
            h = 0;
            throw BeidConnectException(BeidConnectException_Code::SRC_NO_CONTEXT);
        }
    };
    ~SCardCtx() {
        if (h != 0)
        {
            LONG ret = SCardReleaseContext(h);
            if (SCARD_S_SUCCESS != ret)
            {
                log_error("E: Failed SCardReleaseContext");
            }
            else {
                //log_info("SCardReleaseContext ok");
            }
        }
    };
    operator SCARDCONTEXT() { return h; };
private:
    SCARDCONTEXT h = 0;
};

class SCard : public CardReader
{
    void beginTransaction() override;
    void endTransaction() override;
public:
    SCard();
    virtual ~SCard() { disconnect(); };
    static void listReaders(std::vector<std::shared_ptr<CardReader>>& readers);
    long connect() override;
    void disconnect() override;
    bool isPinPad() override;
    CardAPDUResponse apdu(const CardAPDU& apdu) override;
    void verify_pinpad(unsigned char format, unsigned char PINBlock, size_t PINLength, uint16_t PINMaxExtraDigit, const unsigned char pinAPDU[], size_t l_pinAPDU, uint16_t* sw) override;

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
        void clear()
        {
            verify_pin_start = 0;
            verify_pin_finish = 0;
            modify_pin_start = 0;
            modify_pin_finish = 0;
            get_key_pressed = 0;
            verify_pin_direct = 0;
            modify_pin_direct = 0;
            mct_readerdirect = 0;
            mct_universal = 0;
            ifd_pin_prop = 0;
            abort = 0;
        }
    } cmds;
    bool FeaturesRetrieved = false;
    bool m_bCanUsePPDU = false;
    SCARDHANDLE     hCard;
    SCARD_IO_REQUEST  ioSendPci = { 1, sizeof(SCARD_IO_REQUEST) };
    SCARD_IO_REQUEST  ioRecvPci = { 1, sizeof(SCARD_IO_REQUEST) };
};

#endif /* SCard_hpp */
