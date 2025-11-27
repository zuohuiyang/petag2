#pragma once
#include <string>
#include <vector>

namespace pctag {

std::vector<uint8_t> ReadFileBytes(const std::wstring& path, std::string* err);
bool WriteFileBytes(const std::wstring& path, const std::vector<uint8_t>& data, bool overwrite, std::string* err);
bool FileExists(const std::wstring& path);
bool IsPEFile(const std::vector<uint8_t>& data);

}

