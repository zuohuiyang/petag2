#include <string>
#include <vector>
#include <iostream>
#include <windows.h>

#include "base/io.h"
#include "base/logging.h"
#include "codec/metadata_codec.h"
#include "chrome/updater/certificate_tag.h"

using updater::tagging::CreatePEBinary;
using updater::tagging::BinaryInterface;

static void PrintHelp() {
  pctag::LogInfo("Usage:");
  pctag::LogInfo("  petag2 -i|--insert <input> <output> <metadata>");
  pctag::LogInfo("  petag2 -e|--read <input>");
  pctag::LogInfo("Options:");
  pctag::LogInfo("  --help    Show this help message");
}

static std::wstring Utf8ToWide(const std::string& s) {
  int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
  std::wstring w; w.resize(n);
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
  return w;
}

static bool NormalizeWinCertificateLength(std::vector<uint8_t>& bytes) {
  if (bytes.size() < sizeof(IMAGE_DOS_HEADER)) return false;
  auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
  if (dos->e_magic != 0x5A4D) return false;
  size_t pe_off = static_cast<size_t>(dos->e_lfanew);
  if (pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) > bytes.size()) return false;
  size_t opt_off = pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
  if (opt_off + sizeof(IMAGE_OPTIONAL_HEADER32) > bytes.size()) return false;
  uint16_t opt_magic = *reinterpret_cast<const uint16_t*>(bytes.data() + opt_off);
  uint32_t cert_va = 0, cert_sz = 0;
  if (opt_magic == 0x10b) {
    auto opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(bytes.data() + opt_off);
    cert_va = opt->DataDirectory[4].VirtualAddress;
    cert_sz = opt->DataDirectory[4].Size;
  } else if (opt_magic == 0x20b) {
    auto opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(bytes.data() + opt_off);
    cert_va = opt->DataDirectory[4].VirtualAddress;
    cert_sz = opt->DataDirectory[4].Size;
  } else {
    return false;
  }
  if (cert_va == 0 || cert_sz < 8 || (size_t)cert_va + cert_sz > bytes.size()) return false;
  uint32_t* len_ptr = reinterpret_cast<uint32_t*>(&bytes[cert_va]);
  if (*len_ptr == cert_sz) return false;
  *len_ptr = cert_sz;
  return true;
}

int main(int argc, char** argv) {
  if (argc < 2) { PrintHelp(); return 1; }
  std::string cmd = argv[1];
  if (cmd == "--help" || cmd == "-h") { PrintHelp(); return 0; }

  if (cmd == "-e" || cmd == "--read") {
    if (argc != 3) { pctag::LogError("Invalid arguments for read"); return 1; }
    std::wstring in = Utf8ToWide(argv[2]);
    std::string err;
    auto bytes = pctag::ReadFileBytes(in, &err);
    if (bytes.empty()) { pctag::LogError(err); return 1; }
    if (!pctag::IsPEFile(bytes)) { pctag::LogError("Input is not a PE file"); return 1; }
    auto bin = CreatePEBinary(base::span<const uint8_t>(bytes.data(), bytes.size()));
    if (!bin) { pctag::LogError("Failed to parse signed PE or not signed"); return 1; }
    auto t = bin->tag();
    if (!t || t->empty()) { pctag::LogInfo("No metadata tag found"); return 0; }
    std::string meta;
    if (!pctag::DecodeMetadata(*t, &meta, &err)) { pctag::LogError(err); return 1; }
    pctag::LogInfo("Metadata: " + meta);
    return 0;
  }

  

  if (cmd == "-i" || cmd == "--insert") {
    if (argc != 5) { pctag::LogError("Invalid arguments for insert"); PrintHelp(); return 1; }
    std::wstring in = Utf8ToWide(argv[2]);
    std::wstring out = Utf8ToWide(argv[3]);
    std::string meta_arg = argv[4];
    if (in == out) { pctag::LogError("Output must be a different file"); return 1; }
    std::string err;
    auto bytes = pctag::ReadFileBytes(in, &err);
    if (bytes.empty()) { pctag::LogError(err); return 1; }
    if (!pctag::IsPEFile(bytes)) { pctag::LogError("Input is not a PE file"); return 1; }
    auto bin = CreatePEBinary(base::span<const uint8_t>(bytes.data(), bytes.size()));
    if (!bin) {
      bool fixed = NormalizeWinCertificateLength(bytes);
      if (fixed) {
        bin = CreatePEBinary(base::span<const uint8_t>(bytes.data(), bytes.size()));
      }
    }
    if (!bin) { pctag::LogError("Failed to parse signed PE or not signed"); return 1; }
    auto tag_bytes = pctag::EncodeMetadata(meta_arg, &err);
    if (tag_bytes.empty()) { pctag::LogError(err); return 1; }
    auto updated = bin->SetTag(base::span<const uint8_t>(tag_bytes.data(), tag_bytes.size()));
    if (!updated) { pctag::LogError("Failed to set tag"); return 1; }
    if (!pctag::WriteFileBytes(out, *updated, true, &err)) { pctag::LogError(err); return 1; }
    pctag::LogInfo("Tag inserted");
    return 0;
  }

  

  

  pctag::LogError("Unknown command");
  PrintHelp();
  return 1;
}
