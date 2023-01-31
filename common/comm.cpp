#include <iostream>
#include <sstream>
#include "comm.hpp"
#include "log.hpp"
#include "util.h"

#ifdef _WIN32
#include <io.h>
#endif

using namespace std;

#define WHERE "readMessage()"
int readMessage(std::stringstream &stream)
{
    int length = 0;
    int read_char;
    int i;

    for (i = 0; i < 4; i++)
    {
        while ((read_char = getchar()) == EOF)
        {
            // Check reason
            if (std::feof(stdin))
            {
                //log_info("stdin EOF");
            }
            else if (std::ferror(stdin))
            {
                log_info("stdin error");
                return (E_COMM_PARAM);
            }
            do_sleep(20);
        }
        length = length | (read_char << i * 8);
    }

    // Some info on internet defines a 0 or -1 is send by the browser to gracefully stop the native Host
    // Never go it, but we can still handle it in case of
    // NB: official doc does not mention this.
    if (length == 0)
    {
        log_info("%s: Resquest Len (%i)", WHERE, length);
        return E_COMM_ENDREQUEST;
    }
    else if (length < 0)
    {
        log_info("%s: Resquest Len (%i)", WHERE, length);
        return (E_COMM_PARAM);
    }

    // sanity check
    if (length > 10000)
    {
        log_error("%s: message length exceeds max size (%d > 10000)", WHERE, length);
        return (E_COMM_PARAM);
    }

    for (i = 0; i < length; i++)
    {
        read_char = getchar();
        if (read_char != '\0')
            stream << (unsigned char)read_char;
    }

    // Remove PIN from dump
    std::string dumpstr = stream.str();
    size_t len = dumpstr.length();
    size_t pos = dumpstr.find("\"pin\":\"");
    if (pos != string::npos)
    {
        pos += 7;
        while (dumpstr[pos] != '\"' && pos < len)
        {
            dumpstr[pos] = '*';
            pos++;
        }
    }
    log_info("%s: Resquest (%i): '%s'", WHERE, length, dumpstr.c_str());

    return 0;
}
#undef WHERE

#define WHERE "sendMessage()"
int sendMessage(const std::string response)
{
    std::string response2 = response.substr(0, response.length() - 1);
    size_t length = response2.length();
    // include nullterminator here
    uint32_t len = (uint32_t)length /*+ 1*/;

    fwrite(&len, 1, 4, stdout);

    log_info("%s: Response (%i): '%s'", WHERE, len, response2.c_str());

    cout << response2 << std::flush;

    return 0;
}
#undef WHERE
