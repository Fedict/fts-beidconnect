#include "UserCertsRequestHandler.hpp"
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

#define WHERE "UserCertsRequestHandler::process()"
std::string UserCertsRequestHandler::process()
{
    ptree response;

    try
    {
        ReaderList readerList;
        int countSupportedCards = 0;
        int countUnsupportedCards = 0;
        int countErrors = 0;
        int certType = 0;

        if (ptreeRequest->get_optional<std::string>(BeidConnect_JSON_field::keyusage).is_initialized())
        {
            std::string keyusage = ptreeRequest->get<std::string>(BeidConnect_JSON_field::keyusage);
            if (keyusage.compare(BeidConnect_Keyusage::NONREPUDIATION) == 0)
            {
                certType = CERT_TYPE_NONREP;
            }
            else if (keyusage.compare(BeidConnect_Keyusage::DIGITALSIGNATURE) == 0)
            {
                certType = CERT_TYPE_AUTH;
            }
            // else 0, return all usercerts
        }

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
                    continue;
                }

                if (reader->connect())
                {
                    countErrors++;
                    continue;
                }

                std::shared_ptr<Card> card = CardFactory::createCard(reader);
                if (card == nullptr)
                {
                    countUnsupportedCards++;
                    continue; // card not supported in this reader, try next reader
                }

                std::vector<std::shared_ptr<const CardFile>> certificates;
                if (certType == 0 || certType == CERT_TYPE_NONREP)
                {
                    certificates.push_back(card->getFile(CardFiles::Signcert));
                }
                if (certType == 0 || certType == CERT_TYPE_AUTH)
                {
                    certificates.push_back(card->getFile(CardFiles::Authcert));
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
                ptree certList;
                for (auto& cert : certificates)
                {
                    ptree certEntry;
                    certEntry.put("", std::string(cert->getBase64().data(), cert->getBase64().size()));
                    certList.push_back(std::make_pair("", certEntry));
                }
                readerInfo.add_child(BeidConnect_JSON_field::certificates, certList);
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
        log_error("%s: E: SCardException SCardResult(%08X) code(%08X)", WHERE, e.getSCardResult(), e.getCode());
        response.put(BeidConnect_JSON_field::result, e.result());
    }
    catch (CardException& e)
    {
        log_error("%s: E: CardException SW(%04X)", WHERE, e.getSW());
        response.put(BeidConnect_JSON_field::result, e.result());
    }
    catch (BeidConnectException& e)
    {
        log_error("%s: E: BeidConnectException code(%08X)", WHERE, e.getCode());
        response.put(BeidConnect_JSON_field::result, e.result());
    }
    catch (...)
    {
        log_error("%s: E: Exception", WHERE);
        response.put(BeidConnect_JSON_field::result, BeidConnect_Result::general_error);
    }
    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);
    return streamResponse.str();
}
#undef WHERE
