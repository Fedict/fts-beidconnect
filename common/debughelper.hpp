//
//  debughelper.hpp
//  BeIDConnect
//
//  Created by Christophe Peerens on 28/03/2024.
//  Copyright © 2024 FOD BOSA. All rights reserved.
//
#pragma once

#ifndef debughelper_hpp
#define debughelper_hpp

#include <stdio.h>
#include <vector>
#include <string>

class DebugStackTrace
{
    static std::vector<const char*> /*DebugStackTrace::*/stack;

public:
    DebugStackTrace(const char* function);
    ~DebugStackTrace();
    
    static std::string GetStack();
    void dump();
};

#define DECLAREFUNCTIONHEADER DebugStackTrace debugStackTrace(__func__);
//#define DECLAREFUNCTIONHEADER DebugStackTrace debugStackTrace(__PRETTY_FUNCTION__);

#endif /* debughelper_hpp */
