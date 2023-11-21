#include "InfoRequestHandler.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include "Card.hpp"
#include "CardFactory.hpp"
#include "ReaderList.hpp"
#include "CardReader.hpp"

using boost::property_tree::ptree;

std::string InfoRequestHandler::process()
{
    ptree response;

    int supportedCardTypes[] = {CARD_TYPE_BEID, /*CARD_TYPE_PKCS15, CARD_TYPE_PKCS11 */};
    std::shared_ptr<Card> card;
    ReaderList readerList;
    std::shared_ptr<CardReader> reader = readerList.getFirstReaderWithSupportedCardType(supportedCardTypes, sizeof(supportedCardTypes) / sizeof(int));
    if (reader == nullptr)
    {
        if (readerList.readers.size() == 0)
        {
            response.put("result", "no_reader");
        }
        else
        {
            response.put("result", "no_card");
        }
    }
    else
    {
        if ((reader->isPinPad()))
        {
            response.put("reader", "pinpad");
        }
        else
        {
            response.put("reader", "standard");
        }
        response.put("report", reader->name);

        card = CardFactory::createCard(reader);
        if (card == nullptr)
        {
            response.put("result", "no_card");
            response.put("report", "card_type_unsupported");
        }
        else
        {
            response.put("cardtype", card->strType());
            response.put("result", "OK");
        }
        reader->disconnect();
    }

    post_process(response);
    std::stringstream streamResponse;
    boost::property_tree::write_json(streamResponse, response, false);
    return streamResponse.str();
}
