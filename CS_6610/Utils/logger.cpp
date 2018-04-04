#include "logger.h"

// Library includes
#include <stdio.h>
#include <Windows.h>

namespace engine {
namespace logger {

void Print(const char* i_type, const char* i_format, ...)
{
    const size_t len_temp = 256;
    char str_temp[len_temp] = { 0 };

    sprintf_s(str_temp, i_type);
    strcat_s(str_temp, i_format);
    strcat_s(str_temp, "\n");

    const size_t len_output = len_temp + 4096;
    char str_output[len_output] = { 0 };

    va_list args;
    va_start(args, i_format);
    vsprintf_s(str_output, len_output, str_temp, args);
    va_end(args);

    OutputDebugStringA(str_output);
}

}
}

