#ifndef LOGGER_H
#define LOGGER_H
#include <string>

namespace Polylla {
using LogLevel = enum { INFO };

void log(const std::string& message, LogLevel level = INFO);
}  // namespace Polylla
#endif  // LOGGER_H
