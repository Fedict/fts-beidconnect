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

std::string CertChainRequestHandler::process()
{
    DECLAREFUNCTIONHEADER;
    log_info("----- CertChainRequestHandler -----");
    ptree response;
    try
    {
        bool chainFound = false;
        // IsBusy flag handle the case when at least one card report a busy error and the reference certificate is not found.
        // This may be the case when the busy card contains the certificate.
        bool IsBusy = false;

        std::string certif = ptreeRequest->get<std::string>(BeidConnect_JSON_field::cert);

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
                    continue;
                }

                if (reader->connect())
                {
                    continue;
                }

                std::shared_ptr<Card> card = CardFactory::createCard(reader);
                if (card == nullptr)
                {
                    //countUnsupportedCards++;
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

                }
                catch (SCardException& e)
                {
                    if (e.getCode() == SCardException_Code::TransactionFail) IsBusy = true;
                    continue;
                }
                catch(...)
                {
                    continue;
                }
                chainFound = true;

                card->readCertificateChain(subCAs, rootCert);

                // we found the subca(s) and root
                ptree certificateChain;
                certificateChain.put(BeidConnect_JSON_field::rootCA, rootCert->getBase64());

                ptree subCAList;
                for (auto& cert : subCAs)
                {
                    ptree certEntry;
                    certEntry.put("", cert->getBase64());
                    subCAList.push_back(std::make_pair("", certEntry));
                }
                certificateChain.add_child(BeidConnect_JSON_field::subCA, subCAList);

                response.add_child(BeidConnect_JSON_field::certificateChain, certificateChain);
                response.put(BeidConnect_JSON_field::cardType, card->strType());
                if (reader->isPinPad())
                {
                    response.put(BeidConnect_JSON_field::ReaderType, BeidConnect_ReaderType::pinpad);
                }
                else
                {
                    response.put(BeidConnect_JSON_field::ReaderType, BeidConnect_ReaderType::standard);
                }
                if (TraceInfoInJsonResult)
                {
                    response.put(BeidConnect_JSON_field::ReaderName, reader->name);
                }

                break;
            }

            if (chainFound)
            {
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

    // log_info(streamResponse.str().c_str());
    return streamResponse.str();
}
