#pragma once
#ifdef _WIN32
#include <stdio.h>
#else
#include <unistd.h>
#endif

namespace jd::platform
{
#ifdef _WIN32
    void fillConsole(const char* str = "")
    {
        static const char* CSI = "\33[";
        printf("%s%c%s%c%s", CSI, 'H', CSI, '2J', str);
    }
#else
    void fillConsole(const char* str = "")
    {
        write(1, "\E[H\E[2J", 7);
    }
#endif
}