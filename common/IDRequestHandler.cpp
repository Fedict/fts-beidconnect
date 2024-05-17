#include "IDRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "general.h"
#include "SCardException.h"

using boost::property_tree::ptree;

std::string IDRequestHandler::process()
{
    DECLAREFUNCTIONHEADER;
    log_info("----- IDRequestHandler -----");

    ptree response;
    int countSupportedCards = 0;
    int countUnsupportedCards = 0;

    try
    {
        ReaderList readerList;
        size_t count = readerList.readers.size();

        int idflags = stoi(ptreeRequest->get<std::string>("idflags"));

        if (count == 0)
        {
            response.put(BeidConnect_JSON_field::result, BeidConnect_Result::no_reader);
        }
        else
        {
            ptree readerInfos;
            for (int i = 0; i < (int)count; i++)
            {

                std::shared_ptr<CardReader> reader = readerList.getReaderByIndex(i);
                if (reader->atr == "")
                {
                    continue;
                }

                if (reader->connect())
                {
                    continue;
                }

                std::shared_ptr<Card> card = CardFactory::createCard(reader);
                if (card == nullptr)
                {
                    countUnsupportedCards++;
                    continue; // card not supported in this reader, try next reader
                }

                countSupportedCards++;
                ptree readerInfo;
                readerInfo.put(BeidConnect_JSON_field::ReaderName, reader->name);
                if (reader->isPinPad())
                {
                    readerInfo.put(BeidConnect_JSON_field::ReaderType, BeidConnect_ReaderType::pinpad);
                }
                else
                {
                    readerInfo.put(BeidConnect_JSON_field::ReaderType, BeidConnect_ReaderType::standard);
                }
                readerInfo.put(BeidConnect_JSON_field::cardType, card->strType());

                if (idflags & ID_FLAG_INCLUDE_ID)
                {
                    readerInfo.put("id", card->getFile(CardFiles::Id)->getBase64());
                }
                if (idflags & ID_FLAG_INCLUDE_ADDR)
                {
                    readerInfo.put("addr", card->getFile(CardFiles::Address)->getBase64());
                }
                if (idflags & ID_FLAG_INCLUDE_PHOTO)
                {
                    readerInfo.put("photo", card->getFile(CardFiles::Photo)->getBase64());
                }
                if (idflags & ID_FLAG_INCLUDE_INTEGRITY)
                {
                    readerInfo.put("idsig", card->getFile(CardFiles::Id_sig)->getBase64());
                    readerInfo.put("addrsig", card->getFile(CardFiles::Address_sig)->getBase64());
                    readerInfo.put("rrncert", card->getFile(CardFiles::Rrncert)->getBase64());
                }
                if ((idflags & ID_FLAG_INCLUDE_AUTH_CERT) || (idflags & ID_FLAG_INCLUDE_CERTS))
                {
                    readerInfo.put("authcert", card->getFile(CardFiles::Authcert)->getBase64());
                }
                if ((idflags & ID_FLAG_INCLUDE_SIGN_CERT) || (idflags & ID_FLAG_INCLUDE_CERTS))
                {
                    readerInfo.put("signcert", card->getFile(CardFiles::Signcert)->getBase64());
                }
                if ((idflags & ID_FLAG_INCLUDE_CACERTS) || (idflags & ID_FLAG_INCLUDE_CERTS))
                {
                    readerInfo.put("cacert", card->getFile(CardFiles::Cacert)->getBase64());
                }
                if ((idflags & ID_FLAG_INCLUDE_ROOTCERT) || (idflags & ID_FLAG_INCLUDE_CERTS))
                {
                    readerInfo.put("rootcert", card->getFile(CardFiles::Rootcert)->getBase64());
                }

                readerInfos.push_back(std::make_pair("", readerInfo));
            }
            response.add_child(BeidConnect_JSON_field::Readers, readerInfos);

            if (countSupportedCards > 0)
            {
                response.put(BeidConnect_JSON_field::result, BeidConnect_Result::OK);
            }
            else
            {
                // errors and unsupported cards result in nop_card
                response.put(BeidConnect_JSON_field::result, BeidConnect_Result::no_card);

                if (countUnsupportedCards > 0)
                {
                    response.put(BeidConnect_JSON_field::report, "card_type_unsupported");
                }
            }
        }
    }
    catch (SCardException& e)
    {
        e.log();
        response.put(BeidConnect_JSON_field::result, e.result());
    }
    catch (CardException& e)
    {
        e.log();
        response.put(BeidConnect_JSON_field::result, e.result());
    }
    catch (BeidConnectException& e)
    {
        e.log();
        response.put(BeidConnect_JSON_field::result, e.result());
    }
    catch (...)
    {
        log_error("%s: E: Exception", __func__);
        response.put(BeidConnect_JSON_field::result, BeidConnect_Result::general_error);
    }
    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);
    return streamResponse.str();
}
