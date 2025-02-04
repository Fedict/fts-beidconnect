#include "SignRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "util.h"
#include "general.h"
#include "hash.h"
#include "crypto.h"
#include "SCardException.h"

using boost::property_tree::ptree;

std::string SignRequestHandler::process()
{
    DECLAREFUNCTIONHEADER;
    log_info("----- SignRequestHandler -----");
    ptree response;
    try
    {
        long lasterror = 0;
        unsigned char signature[512];
        size_t l_signature = 512;
        bool signatureDone = false;
        // IsBusy flag handle the case when at least one card report a busy error and the reference certificate is not found.
        // This may be the case when the busy card contains the certificate.
        bool IsBusy = false;

        std::string certif = ptreeRequest->get<std::string>(BeidConnect_JSON_field::cert);
        std::string operation = ptreeRequest->get<std::string>(BeidConnect_JSON_field::operation);
        std::string pin = ptreeRequest->get<std::string>(BeidConnect_JSON_field::pin);
        std::string digest = ptreeRequest->get<std::string>(BeidConnect_JSON_field::digest);
        std::string digestAlgo = ptreeRequest->get<std::string>(BeidConnect_JSON_field::algo);

        std::vector<unsigned char> cert(base64decode_len(certif));
        cert.resize(base64decode(certif, cert.data()));

        std::vector<unsigned char> hash(base64decode_len(digest));
        hash.resize(base64decode(digest, hash.data()));

        ReaderList readerList;
        if (readerList.readers.size() == 0)
        {
            response.put(BeidConnect_JSON_field::result, BeidConnect_Result::no_reader);
        }
        else
        {
            ptree readerInfos;
            for (auto& reader : readerList.readers)
            {
                if (reader->atr == "")
                {
                    lasterror = E_SRC_NO_CARD;
                    continue;
                }

                lasterror = reader->connect();
                if (lasterror)
                {
                    continue;
                }

                std::shared_ptr<Card> card = CardFactory::createCard(reader);
                if (card == nullptr)
                {
                    lasterror = CARD_TYPE_UNKNOWN;
                    continue; // card not supported in this reader, try next reader
                }

                try
                {
                    card->selectKey((operation == BeidConnect_operation::SIGN) ? CardKeys::NonRep : CardKeys::Auth, cert);
                }
                catch (SCardException& e)
                {
                    if (e.getCode() == SCardException_Code::TransactionFail) IsBusy = true;
                    lasterror = E_SRC_NO_CARD;
                    continue;
                }
                catch (...)
                {
                    lasterror = E_SRC_NO_CARD;
                    reader->disconnect();
                    continue; // try next reader to find chain
                }

                if (lasterror)
                {
                    log_error("%s: E: card->selectKey returned %d (0x%0X)", __func__, lasterror, lasterror);
                    reader->disconnect();
                    continue; // try next reader to find chain
                }

                if (pin == "null")
                {
                    pin = "";
                }

                signatureDone = true;

                // if we get here, we successfully selected the key or found the signing certificate
                card->logon((int)pin.size(), (char*)pin.c_str());

                int sw = 0;
                lasterror = card->sign(hash, algo2str((char*)digestAlgo.c_str()), signature, &l_signature, &sw);
                if (lasterror)
                {
                    log_error("%s: E: card->sign returned %08X", __func__, lasterror);
                    try
                    {
                        card->logoff();
                    }
                    catch (...) {}
                    break;
                }

                if (TraceInfoInJsonResult)
                {
                    response.put(BeidConnect_JSON_field::ReaderName, reader->name);
                }

                try
                {
                    card->logoff();
                }
                catch (...) {}

                // verify the signature if we have a certificate
                //if ((l_cert > 0) && (cert != 0))
                //{
                //    lasterror = verifySignature(hash, l_hash, "sha256", cert, l_cert, signature, l_signature);
                //    if (lasterror)
                //    {
                //        // since we verify the signature based on the certificate that was used to do the signature,
                //        // there can only be two reasons to fail a verification:
                //        // 1. or the certificate linked to the private key on the card does not belong to the private key
                //        // 2. or the reader calculates a faux signature, maybe because the reader does not support extended APDU commands
                //        // where the signature is incomplete!
                //        lasterror = E_SRC_SIGNATURE_FAILED;
                //    }
                //}
                break;
            }

            if (signatureDone)
            {
                response.put(BeidConnect_JSON_field::signature, rawToBase64(std::vector<unsigned char>(signature, signature + l_signature)));
                response.put(BeidConnect_JSON_field::result, BeidConnect_Result::OK);
            }
            else if (IsBusy)
            {
                response.put(BeidConnect_JSON_field::result, BeidConnect_Result::busy);
            }
            else
            {
                // errors and unsupported cards result in no_card
                response.put(BeidConnect_JSON_field::result, BeidConnect_Result::no_card);
                //if (countUnsupportedCards > 0)
                //{
                //    response.put(BeidConnect_JSON_field::report, BeidConnect_Result::card_type_unsupported);
                //}
            }
        }
    }
    catch (SCardException& e)
    {
        e.log();
        response.put(BeidConnect_JSON_field::result, e.result());
        response.put(BeidConnect_JSON_field::resultType, e.resultType());
        response.put(BeidConnect_JSON_field::resultRaw, e.resultRaw());
    }
    catch (CardException& e)
    {
        e.log();
        response.put(BeidConnect_JSON_field::result, e.result());
        response.put(BeidConnect_JSON_field::resultType, e.resultType());
        response.put(BeidConnect_JSON_field::resultRaw, e.resultRaw());
    }
    catch (BeidConnectException& e)
    {
        e.log();
        response.put(BeidConnect_JSON_field::result, e.result());
        response.put(BeidConnect_JSON_field::resultType, e.resultType());
        response.put(BeidConnect_JSON_field::resultRaw, e.resultRaw());
    }
    catch (...)
    {
        log_error("%s: E: Exception", __func__);
        response.put(BeidConnect_JSON_field::result, BeidConnect_Result::general_error);
        response.put(BeidConnect_JSON_field::resultType, BeidConnect_Result::general_error);
        response.put(BeidConnect_JSON_field::resultRaw, 0xFFFFFFFF);
    }
    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);

    // log_info(streamResponse.str().c_str());
    return streamResponse.str();
}
