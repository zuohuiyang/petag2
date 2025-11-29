#include "base/logging.h"
#include <windows.h>

namespace petag {

static void WriteStdOut(const std::string& s) {
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written = 0;
  WriteFile(h, s.c_str(), static_cast<DWORD>(s.size()), &written, nullptr);
}

static void WriteStdErr(const std::string& s) {
  HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
  DWORD written = 0;
  WriteFile(h, s.c_str(), static_cast<DWORD>(s.size()), &written, nullptr);
}

void LogInfo(const std::string& msg) {
  WriteStdOut(msg + "\r\n");
}

void LogError(const std::string& msg) {
  WriteStdErr(msg + "\r\n");
}

}
