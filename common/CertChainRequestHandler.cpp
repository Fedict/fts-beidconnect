#include "CertChainRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "util.h"
#include "SCardException.h"

using boost::property_tree::ptree;

#define WHERE "CertChainRequestHandler::process()"
std::string CertChainRequestHandler::process()
{
    ptree response;
    int chainFound = 0;
    int countUnsupportedCards = 0;
    int countErrors = 0;

    try
    {
        std::string certif = ptreeRequest->get<std::string>("cert");
        int l_cert = base64decode_len(certif);
        unsigned char* cert = (unsigned char*)malloc(l_cert);
        if (cert == 0)
        {
            log_error("%s mem alloc failed for cert (%d)", WHERE, l_cert);
        }
        l_cert = base64decode(certif, cert);

        ReaderList readerList;
        size_t count = readerList.readers.size();
        if (count == 0)
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
                    // card not supported in this reader, try next reader
                    continue;
                }

                // add usercertificates to list
                std::vector<std::shared_ptr<const CardFile>> subCAs;
                std::shared_ptr<const CardFile> rootCert;
                try
                {
                    std::shared_ptr<const CardFile> cardSigningCert = card->getFile(CardFiles::Signcert);
                    if (cardSigningCert->getBase64() != certif)
                    {
                        std::shared_ptr<const CardFile> cardAuthCert = card->getFile(CardFiles::Authcert);
                        if (cardAuthCert->getBase64() != certif)
                        {
                            // none of the certificate of the card conrespond to the certificate received as parameter
                            continue;
                        }
                    }

                    card->readCertificateChain(subCAs, rootCert);
                }
                catch(...)
                {
                    continue;
                }
                chainFound = 1;

                // we found the subca(s) and root
                ptree certificateChain;
                certificateChain.put("rootCA", rootCert->getBase64());

                ptree subCAList;
                for (auto& cert : subCAs)
                {
                    ptree certEntry;
                    certEntry.put("", cert->getBase64());
                    subCAList.push_back(std::make_pair("", certEntry));
                }
                certificateChain.add_child("subCA", subCAList);

                response.add_child("certificateChain", certificateChain);
                response.put("cardType", card->strType());
                if (reader->isPinPad())
                {
                    response.put("ReaderType", "pinpad");
                }
                else
                {
                    response.put("ReaderType", "standard");
                }
                if (TraceInfoInJsonResult)
                {
                    response.put("ReaderName", reader->name);
                }

                break;
            }

            if (chainFound > 0)
            {
                response.put("result", "OK");
            }
            else
            {
                // errors and unsupported cards result in no_card
                response.put("result", "no_card");

                if (countUnsupportedCards > 0)
                {
                    response.put("report", "card_type_unsupported");
                }
            }
        }

        if (cert)
        {
            free(cert);
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

    // log_info(streamResponse.str().c_str());
    return streamResponse.str();
}
#undef WHERE
