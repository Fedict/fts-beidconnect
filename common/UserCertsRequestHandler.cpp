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

        if (ptreeRequest->get_optional<std::string>("keyusage").is_initialized())
        {
            std::string keyusage = ptreeRequest->get<std::string>("keyusage");
            if (keyusage.compare("NONREPUDIATION") == 0)
            {
                certType = CERT_TYPE_NONREP;
            }
            else if (keyusage.compare("DIGITALSIGNATURE") == 0)
            {
                certType = CERT_TYPE_AUTH;
            }
            // else 0, return all usercerts
        }

        if (readerList.readers.size() == 0)
        {
            response.put("result", "no_reader");
        }
        else
        {
            long status = 0;
            ptree readerInfos;
            for (auto& reader : readerList.readers)
            {
                if (reader->atr == "")
                {
                    continue;
                }

                status = reader->connect();
                if (status)
                {
                    countErrors++;
                    log_error("%s: E: reader->connect(%s) returned %08X", WHERE, reader->name.c_str(), status);
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
                readerInfo.put("ReaderName", reader->name);
                if (reader->isPinPad())
                {
                    readerInfo.put("ReaderType", "pinpad");
                }
                else
                {
                    readerInfo.put("ReaderType", "standard");
                }
                readerInfo.put("cardType", card->strType());
                ptree certList;
                for (auto& cert : certificates)
                {
                    ptree certEntry;
                    certEntry.put("", std::string(cert->getBase64().data(), cert->getBase64().size()));
                    certList.push_back(std::make_pair("", certEntry));
                }
                readerInfo.add_child("certificates", certList);
                readerInfos.push_back(std::make_pair("", readerInfo));
            }
            response.add_child("Readers", readerInfos);

            if (countSupportedCards > 0)
            {
                response.put("result", "OK");
            }
            else
            {
                // errors and unsupported cards result in nop_card
                response.put("result", "no_card");

                if (countUnsupportedCards > 0)
                {
                    response.put("report", "card_type_unsupported");
                }
            }
        }
    }
    catch (SCardException& e)
    {
        response.put("result", e.result());
    }
    catch (...)
    {
        response.put("result", "general_error");
    }
    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);
    return streamResponse.str();
}
#undef WHERE
