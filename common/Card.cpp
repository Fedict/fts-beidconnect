#include "Card.hpp"
#include "CardReader.hpp"
#include "log.hpp"
#include "general.h"

long Card::getFile(unsigned char* file, size_t l_file, size_t* l_out, unsigned char* p_out)
{
    long ret = 0;
    ScopedCardTransaction trans(reader);  //begin transaction
    if (trans.TransactionFailed()) {
        log_error("E: get_file(): could not start transaction");
        return E_SRC_START_TRANSACTION;
    }
    else {
        ret = selectFile(file, l_file);
        if (ret == 0)
            ret = readFile2(0, l_out, (unsigned char*)p_out);
    }
    return ret;
}
