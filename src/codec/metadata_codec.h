#pragma once
#include <string>
#include <vector>

namespace pctag {

std::vector<uint8_t> EncodeMetadata(const std::string& ascii, std::string* err);
bool DecodeMetadata(const std::vector<uint8_t>& tag, std::string* out, std::string* err);
bool IsValidAscii(const std::string& s);

}

