#ifndef PETAG2_BASE_LOGGING_H_
#define PETAG2_BASE_LOGGING_H_
#include <string>

namespace petag {

void LogInfo(const std::string& msg);
void LogError(const std::string& msg);

}

#endif  // PETAG2_BASE_LOGGING_H_
