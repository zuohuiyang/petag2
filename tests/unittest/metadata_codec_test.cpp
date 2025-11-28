#include "codec/metadata_codec.h"
#include <cassert>
#include <string>

int main() {
  std::string err;
  std::vector<uint8_t> enc = pctag::EncodeMetadata("{test_chan:123}", &err);
  assert(!enc.empty());
  std::string out;
  bool ok = pctag::DecodeMetadata(enc, &out, &err);
  assert(ok);
  assert(out == "{test_chan:123}");
  assert(!pctag::IsValidAscii(""));
  assert(pctag::IsValidAscii(std::string(255, 'A')));
  assert(!pctag::IsValidAscii(std::string(256, 'A')));
  return 0;
}
