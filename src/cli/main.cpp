#include <string>
#include <vector>
#include <iostream>
#include <windows.h>

#include "base/io.h"
#include "base/logging.h"
#include "codec/metadata_codec.h"
#include "verify/wintrust_verify.h"
#include "chrome/updater/certificate_tag.h"

using updater::tagging::CreatePEBinary;
using updater::tagging::BinaryInterface;

static void PrintHelp() {
  pctag::LogInfo("Usage:");
  pctag::LogInfo("  pctool -i|--insert <input> <output> <metadata>");
  pctag::LogInfo("  pctool -e|--read <input>");
  pctag::LogInfo("  pctool --verify <input>");
  pctag::LogInfo("  pctool --diagnose-pe <input>");
  pctag::LogInfo("  pctool --dump-certdir <input>");
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

  if (cmd == "--verify") {
    if (argc != 3) { pctag::LogError("Invalid arguments for verify"); return 1; }
    std::wstring in = Utf8ToWide(argv[2]);
    std::string reason;
    if (pctag::VerifyAuthenticode(in, &reason)) { pctag::LogInfo("Signature valid"); return 0; }
    pctag::LogError("Signature invalid: " + reason);
    return 1;
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
    std::string reason;
    if (!pctag::VerifyAuthenticode(out, &reason)) { pctag::LogError("Signature check failed: " + reason); return 1; }
    pctag::LogInfo("Tag inserted and signature valid");
    return 0;
  }

  if (cmd == "--diagnose-pe") {
    if (argc != 3) { pctag::LogError("Invalid arguments for diagnose-pe"); return 1; }
    std::wstring in = Utf8ToWide(argv[2]);
    std::string err;
    auto bytes = pctag::ReadFileBytes(in, &err);
    if (bytes.empty()) { pctag::LogError(err); return 1; }
    if (!pctag::IsPEFile(bytes)) { pctag::LogError("Input is not a PE file"); return 1; }
    auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
    if (bytes.size() < sizeof(IMAGE_DOS_HEADER) || dos->e_magic != 0x5A4D) { pctag::LogError("Invalid DOS header"); return 1; }
    size_t pe_off = static_cast<size_t>(dos->e_lfanew);
    if (pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) > bytes.size()) { pctag::LogError("Invalid NT headers"); return 1; }
    auto nt_sig = *reinterpret_cast<const DWORD*>(bytes.data() + pe_off);
    if (nt_sig != 0x00004550) { pctag::LogError("Invalid NT signature"); return 1; }
    auto file_hdr = reinterpret_cast<const IMAGE_FILE_HEADER*>(bytes.data() + pe_off + sizeof(DWORD));
    bool is_exe = (file_hdr->Characteristics & 0x0002) != 0;
    bool is_dll = (file_hdr->Characteristics & 0x2000) != 0;
    pctag::LogInfo(std::string("ExecutableImage=") + (is_exe?"true":"false"));
    pctag::LogInfo(std::string("DLL=") + (is_dll?"true":"false"));
    size_t opt_off = pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
    if (opt_off + file_hdr->SizeOfOptionalHeader > bytes.size()) { pctag::LogError("Optional header truncated"); return 1; }
    uint16_t opt_magic = *reinterpret_cast<const uint16_t*>(bytes.data() + opt_off);
    uint32_t dir_count = 0, cert_va = 0, cert_sz = 0;
    if (opt_magic == 0x10b) {
      auto opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(bytes.data() + opt_off);
      dir_count = opt->NumberOfRvaAndSizes;
      cert_va = opt->DataDirectory[4].VirtualAddress;
      cert_sz = opt->DataDirectory[4].Size;
    } else if (opt_magic == 0x20b) {
      auto opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(bytes.data() + opt_off);
      dir_count = opt->NumberOfRvaAndSizes;
      cert_va = opt->DataDirectory[4].VirtualAddress;
      cert_sz = opt->DataDirectory[4].Size;
    } else {
      pctag::LogError("Unsupported optional header");
      return 1;
    }
    pctag::LogInfo(std::string("DataDirectoryCount=") + std::to_string(dir_count));
    pctag::LogInfo("CertDir RVA=" + std::to_string(cert_va) + ", Size=" + std::to_string(cert_sz));
    bool at_end = (size_t)cert_va + cert_sz == bytes.size();
    pctag::LogInfo(std::string("CertAtEnd=") + (at_end?"true":"false"));
    pctag::LogInfo("FileSize=" + std::to_string(bytes.size()));
    return at_end && is_exe && !is_dll ? 0 : 1;
  }

  if (cmd == "--dump-certdir") {
    if (argc != 3) { pctag::LogError("Invalid arguments for dump-certdir"); return 1; }
    std::wstring in = Utf8ToWide(argv[2]);
    std::string err;
    auto bytes = pctag::ReadFileBytes(in, &err);
    if (bytes.empty()) { pctag::LogError(err); return 1; }
    if (!pctag::IsPEFile(bytes)) { pctag::LogError("Input is not a PE file"); return 1; }
    auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
    size_t pe_off = static_cast<size_t>(dos->e_lfanew);
    auto file_hdr = reinterpret_cast<const IMAGE_FILE_HEADER*>(bytes.data() + pe_off + sizeof(DWORD));
    size_t opt_off = pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
    uint16_t opt_magic = *reinterpret_cast<const uint16_t*>(bytes.data() + opt_off);
    uint32_t cert_va = 0, cert_sz = 0;
    if (opt_magic == 0x10b) {
      auto opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(bytes.data() + opt_off);
      cert_va = opt->DataDirectory[4].VirtualAddress;
      cert_sz = opt->DataDirectory[4].Size;
    } else {
      auto opt = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(bytes.data() + opt_off);
      cert_va = opt->DataDirectory[4].VirtualAddress;
      cert_sz = opt->DataDirectory[4].Size;
    }
    if (cert_va == 0 || cert_sz < 8 || (size_t)cert_va + cert_sz > bytes.size()) { pctag::LogError("Invalid certificate directory"); return 1; }
    size_t pos = cert_va;
    int idx = 0;
    while (pos + 8 <= (size_t)cert_va + cert_sz) {
      uint32_t len = *reinterpret_cast<const uint32_t*>(&bytes[pos]);
      uint16_t rev = *reinterpret_cast<const uint16_t*>(&bytes[pos+4]);
      uint16_t type = *reinterpret_cast<const uint16_t*>(&bytes[pos+6]);
      pctag::LogInfo("Entry " + std::to_string(idx) + ": len=" + std::to_string(len) + ", rev=0x" + std::to_string(rev) + ", type=" + std::to_string(type));
      if (len < 8 || pos + len > (size_t)cert_va + cert_sz) break;
      pos += ((len + 7) & ~size_t(7));
      ++idx;
    }
    return 0;
  }

  pctag::LogError("Unknown command");
  PrintHelp();
  return 1;
}
