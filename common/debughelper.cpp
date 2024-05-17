//
//  debughelper.cpp
//  BeIDConnect
//
//  Created by Christophe Peerens.
//  Copyright © 2024 FOD BOSA. All rights reserved.
//

#include "debughelper.hpp"
#include "log.hpp"

std::vector<const char*> DebugStackTrace::stack;

DebugStackTrace::DebugStackTrace(const char* functionName)
{
    stack.push_back(functionName);
}

DebugStackTrace::~DebugStackTrace()
{
    stack.pop_back();
}

void DebugStackTrace::dump()
{
    log_info(GetStack().c_str());
}

std::string DebugStackTrace::GetStack()
{
    std::string StackPath;
    for(auto i:stack)
    {
        if (StackPath.length() != 0) StackPath += "|";
        StackPath += i;
    }
    
    return StackPath;
}
