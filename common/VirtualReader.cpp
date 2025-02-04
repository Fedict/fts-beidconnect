#include "VirtualReader.hpp"

#ifndef _WIN32
#define lstrlen(s) strlen(s)   // non-Windows wintypes.h: LPTSTR == LPSTR == char*
#endif

#ifdef _WIN32
#include "windows.h"
#endif

VirtualReader::VirtualReader()
{
}

VirtualReader::~VirtualReader()
{
}

int VirtualReader::listReaders(std::vector<std::shared_ptr<CardReader>>& readers)
{
   int ret = 0;
   return (ret);
}

long VirtualReader::connect()
{
   long ret = 0;
   return (ret);
}

void VirtualReader::disconnect()
{
}
