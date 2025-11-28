#include "base/io.h"
#include <windows.h>

namespace pctag {

static std::wstring ToLongPath(const std::wstring& p) {
  if (p.empty()) return p;
  if (p.rfind(L"\\\\?\\", 0) == 0) return p;
  DWORD need = GetFullPathNameW(p.c_str(), 0, nullptr, nullptr);
  std::wstring abs;
  if (need) {
    abs.resize(need - 1);
    GetFullPathNameW(p.c_str(), need, &abs[0], nullptr);
  } else {
    abs = p;
  }
  if (abs.size() >= 2 && abs[1] == L':') {
    return L"\\\\?\\" + abs;
  }
  if (abs.rfind(L"\\\\", 0) == 0) {
    return std::wstring(L"\\\\?\\UNC") + abs.substr(1);
  }
  return abs;
}

bool FileExists(const std::wstring& path) {
  DWORD attr = GetFileAttributesW(ToLongPath(path).c_str());
  return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

std::vector<uint8_t> ReadFileBytes(const std::wstring& path, std::string* err) {
  std::vector<uint8_t> out;
  HANDLE h = CreateFileW(ToLongPath(path).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE) {
    if (err) *err = "Failed to open input file";
    return {};
  }
  LARGE_INTEGER size{};
  if (!GetFileSizeEx(h, &size) || size.QuadPart <= 0) {
    CloseHandle(h);
    if (err) *err = "Invalid file size";
    return {};
  }
  out.resize(static_cast<size_t>(size.QuadPart));
  DWORD read = 0;
  if (!ReadFile(h, out.data(), static_cast<DWORD>(out.size()), &read, nullptr) || read != out.size()) {
    CloseHandle(h);
    if (err) *err = "Failed to read file";
    return {};
  }
  CloseHandle(h);
  return out;
}

bool WriteFileBytes(const std::wstring& path, const std::vector<uint8_t>& data, bool overwrite, std::string* err) {
  DWORD disposition = overwrite ? CREATE_ALWAYS : CREATE_NEW;
  HANDLE h = CreateFileW(ToLongPath(path).c_str(), GENERIC_WRITE, 0, nullptr, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE) {
    if (err) *err = "Failed to open output file";
    return false;
  }
  DWORD written = 0;
  if (!WriteFile(h, data.data(), static_cast<DWORD>(data.size()), &written, nullptr) || written != data.size()) {
    CloseHandle(h);
    if (err) *err = "Failed to write file";
    return false;
  }
  CloseHandle(h);
  return true;
}

bool IsPEFile(const std::vector<uint8_t>& data) {
  if (data.size() < 64) return false;
  if (!(data[0] == 'M' && data[1] == 'Z')) return false;
  return true;
}

}
