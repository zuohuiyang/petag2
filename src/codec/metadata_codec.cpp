#include "codec/metadata_codec.h"
#include <cstdint>

namespace pctag {

static uint32_t Crc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int j = 0; j < 8; ++j) {
      uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

bool IsValidAscii(const std::string& s) {
  if (s.size() < 2 || s.size() > 30) return false;
  for (char c : s) {
    unsigned char uc = static_cast<unsigned char>(c);
    if (uc < 0x20 || uc > 0x7E) return false;
  }
  return true;
}

std::vector<uint8_t> EncodeMetadata(const std::string& ascii, std::string* err) {
  if (!IsValidAscii(ascii)) {
    if (err) *err = "Invalid metadata: ASCII 2-30 chars required";
    return {};
  }
  std::vector<uint8_t> out;
  out.reserve(1 + 1 + ascii.size() + 4);
  out.push_back(0x01);
  out.push_back(static_cast<uint8_t>(ascii.size()));
  out.insert(out.end(), ascii.begin(), ascii.end());
  uint32_t crc = Crc32(reinterpret_cast<const uint8_t*>(ascii.data()), ascii.size());
  out.push_back(static_cast<uint8_t>(crc & 0xFF));
  out.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>((crc >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((crc >> 24) & 0xFF));
  return out;
}

bool DecodeMetadata(const std::vector<uint8_t>& tag, std::string* out, std::string* err) {
  if (tag.size() < 1 + 1 + 4) {
    if (err) *err = "Tag too short";
    return false;
  }
  if (tag[0] != 0x01) {
    if (err) *err = "Unsupported version";
    return false;
  }
  uint8_t len = tag[1];
  if (tag.size() != 1 + 1 + len + 4) {
    if (err) *err = "Tag length mismatch";
    return false;
  }
  std::string payload(reinterpret_cast<const char*>(&tag[2]), reinterpret_cast<const char*>(&tag[2 + len]));
  if (!IsValidAscii(payload)) {
    if (err) *err = "Invalid ASCII payload";
    return false;
  }
  uint32_t crc = static_cast<uint32_t>(tag[2 + len]) |
                 (static_cast<uint32_t>(tag[2 + len + 1]) << 8) |
                 (static_cast<uint32_t>(tag[2 + len + 2]) << 16) |
                 (static_cast<uint32_t>(tag[2 + len + 3]) << 24);
  uint32_t calc = Crc32(reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
  if (crc != calc) {
    if (err) *err = "CRC32 mismatch";
    return false;
  }
  if (out) *out = payload;
  return true;
}

}

