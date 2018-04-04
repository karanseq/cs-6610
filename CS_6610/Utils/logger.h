#ifndef LOGGER_H_
#define LOGGER_H_

// Library includes
#include <stdarg.h>

namespace engine {
namespace logger {

void Print(const char* i_type, const char* i_format, ...);

}
}

#define LOG(format, ...)                engine::logger::Print("DEBUG: ", (format), __VA_ARGS__)
#define LOG_ERROR(format, ...)          engine::logger::Print("ERROR: ", (format), __VA_ARGS__)

#endif // LOGGER_H_
