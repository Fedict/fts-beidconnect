#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string.h>
#include <regex>
#include "general.h"
#include "log.hpp"
#include "util.h"

#ifdef _WIN32
#include <io.h>
#include <codecvt>
#endif

#ifndef _WIN32
#define TCHAR     unsigned char
#endif

using namespace std;

#ifdef _WIN32
bool writeFile(wstring file, wstring exePath, bool isChrome) {
    wofstream myfile;
    if (isChrome)
    {
        myfile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
    }
    else
    {
        // Since firefox generate an error when parsing the BOM
        myfile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff>));
    }
#else
bool writeFile(string file, string exePath, bool isChrome) {
    ofstream myfile;
#endif
    myfile.open(file, std::ios::out/*std::ofstream::trunc*/);
    if (myfile.is_open()) {
        myfile << "{\n";
        myfile << "  \"name\": \"be.bosa.beidconnect\",\n";
        myfile << "  \"description\": \"Access your eID in webapps\",\n";
        myfile << "  \"path\": \"" << exePath << "\",\n";
        myfile << "  \"type\": \"stdio\",\n";
        if (isChrome) {
            myfile << "  \"allowed_origins\": [\n";
            myfile << "     \"chrome-extension://pencgnkbgaekikmiahiaakjdgaibiipp/\",\n";   // Chrome CRX ID
            myfile << "     \"chrome-extension://ifdkechldgmcamjeenkbcddnjaikdlke/\",\n";
            myfile << "     \"chrome-extension://cjhdnjahbjhicmbmejmcifaaiigbgjjd/\"\n";    // Edge CRX ID
            myfile << "  ]\n";
        }
        else {
            myfile << "  \"allowed_extensions\": [\n";
            myfile << "     \"beidconnect@bosa.be\"\n";
            myfile << "  ]\n";
        }

        myfile << "}\n";
        myfile.close();
        return true;
    }
    return false;
}

int runSetup(int argc, const char* argv[])
{
    log_info("BeidConnect Setup Version %s Build %s", BEIDCONNECT_VERSION, __DATE__);
    //generate a json file that is needed for the Chrome Extension to find the Native host application on Windows
#ifdef _WIN32
    wstring chromeFilePath = L"";
    wstring firefoxFilePath = L"";
    wstring exePath;

    wstring cmdLine = GetCommandLineW();
    size_t pos = cmdLine.find(L"-setup");
    // MSI setup add a '"' (double quote) to the -setup content, need to remove it
    if (cmdLine[pos + 7] == L'"')
    {
        pos++;
    }
    wstring installFolder = cmdLine.substr(pos + 7);
    if (installFolder.back() != L'\\')
    {
        installFolder += L"\\";
    }
    exePath = installFolder + L"beidconnect.exe";

    //escape all \ in json file or exe will not be found on windows
    exePath = std::regex_replace(exePath, std::wregex(L"\\\\"), L"\\\\");

    if (chromeFilePath == L"") {
        chromeFilePath = wstring(installFolder) + L"chrome.json";
    }
    if (firefoxFilePath == L"") {
        firefoxFilePath = wstring(installFolder) + L"firefox.json";
    }
    log_info("BeidConnect Setup exePath  \"%S\"", exePath.c_str());
    log_info("BeidConnect Setup creating \"%S\"", chromeFilePath.c_str());
    log_info("BeidConnect Setup creating \"%S\"", firefoxFilePath.c_str());
#else
    const char* installFolder = NULL;
    string chromeFilePath = "";
    string firefoxFilePath = "";
    string exePath;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-setup") == 0) {
            //beidconnect -setup installFolder [chromeFilePath] [firefoxFilePath]
            installFolder = argv[++i];
        }
        else if (i < argc && chromeFilePath == "") {
            chromeFilePath = string(argv[i]) + "/be.bosa.beidconnect.json";
        }
        else if (i < argc && firefoxFilePath == "") {
            firefoxFilePath = string(argv[i]) + "/be.bosa.beidconnect.json";
        }
    }

    if (installFolder == NULL) {
        return (0);
    }

    //log_info("install folder: <%s>", installFolder);

    exePath = string(installFolder) + "/beidconnect";
    if (chromeFilePath == "") {
        chromeFilePath = string(installFolder) + "/chrome.json";
    }
    if (firefoxFilePath == "") {
        firefoxFilePath = string(installFolder) + "/firefox.json";
    }
    log_info("BeidConnect Setup exePath  \"%s\"", exePath.c_str());
    log_info("BeidConnect Setup creating \"%s\"", chromeFilePath.c_str());
    log_info("BeidConnect Setup creating \"%s\"", firefoxFilePath.c_str());
#endif
    bool chromeFileStatus = writeFile(chromeFilePath, exePath, true);
    if (chromeFileStatus) {
        log_info("BeidConnect Setup Chrome config file creation Succeed");
    }
    else{
        log_info("BeidConnect Setup Chrome config file creation Failed");
    }
    bool firefoxFileStatus = writeFile(firefoxFilePath, exePath, false);
    if (firefoxFileStatus) {
        log_info("BeidConnect Setup Firefox config file creation Succeed");
    }
    else{
        log_info("BeidConnect Setup Firefox config file creation Failed");
    }
    return chromeFileStatus&&firefoxFileStatus?0:-1;
}
