#ifndef PETAG2_CODEC_METADATA_CODEC_H_
#define PETAG2_CODEC_METADATA_CODEC_H_
#include <string>
#include <vector>
#include <cstdint>

namespace pctag {

std::vector<uint8_t> EncodeMetadata(const std::string& ascii, std::string* err);
bool DecodeMetadata(const std::vector<uint8_t>& tag, std::string* out, std::string* err);
bool IsValidAscii(const std::string& s);

}

#endif  // PETAG2_CODEC_METADATA_CODEC_H_
