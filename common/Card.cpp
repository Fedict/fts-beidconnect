#include "Card.hpp"
#include "CardReader.hpp"
#include "log.hpp"
#include "general.h"



void Card::setAtr(std::string atr)
{
   this->atr = atr;
}

int Card::getFile(unsigned char *file, int l_file, int* l_out, unsigned char* p_out)
{
int ret = 0;
int begintransaction = 1;

ret = reader->beginTransaction();
if (ret) {
   log_error("E: get_file(): could not start transaction");
   begintransaction = 0;
   CLEANUP(E_SRC_START_TRANSACTION);
}
else {
   ret = selectFile(file, l_file);
   if (ret == 0)
      ret = readFile2(0, l_out, (unsigned char*) p_out);
}

cleanup:
   if (begintransaction)
      reader->endTransaction();

return ret;
}


