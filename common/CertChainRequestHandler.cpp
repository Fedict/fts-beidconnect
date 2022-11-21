#include "CertChainRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"
#include "util.h"

using boost::property_tree::ptree;

#define WHERE "CertChainRequestHandler::process()"
std::string CertChainRequestHandler::process()
{
    ptree response;
    int chainFound = 0;
    int countUnsupportedCards = 0;
    int countErrors = 0;

    std::stringstream ss(ssRequest->str());
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    std::string certif = pt.get<std::string>("cert");
    int l_cert = base64decode_len((unsigned char *)certif.c_str());
    unsigned char *cert = (unsigned char *)malloc(l_cert);
    if (cert == 0)
    {
        log_error("%s mem alloc failed for cert (%d)", WHERE, l_cert);
    }
    l_cert = base64decode((unsigned char *)certif.c_str(), cert);

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
        for (int i = 0; i < (int)count; i++)
        {

            std::shared_ptr<CardReader> reader = readerList.getReaderByIndex(i);
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

            // add usercertificates to list
            std::vector<std::vector<char>> subCAs;
            std::vector<char> root;
            status = card->readCertificateChain(FORMAT_RADIX64, cert, l_cert, subCAs, root);
            if (status)
            {
                // countErrors++;
                // search next reader/card
                log_error("%s: E: readCertificateChain() returned %08X", WHERE, status);
                continue;
            }
            chainFound = 1;

            // we found the subca(s) and root
            ptree certificateChain;
            certificateChain.put("rootCA", std::string(root.data(), root.size()));

            ptree subCAList;
            for (auto &cert : subCAs)
            {
                ptree certEntry;
                certEntry.put("", std::string(cert.data(), cert.size()));
                subCAList.push_back(std::make_pair("", certEntry));
            }
            certificateChain.add_child("subCA", subCAList);
            for (auto &cert : subCAs)
            {
                cert.clear();
            }

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

    response.put("operation", "CERTCHAIN");
    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);

    // log_info(streamResponse.str().c_str());
    return streamResponse.str();
}
#undef WHERE
