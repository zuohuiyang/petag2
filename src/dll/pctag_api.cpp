#include "petag_api.h"

#include <vector>
#include <string>
#include <cstring>
#include <windows.h>

#include "base/io.h"
#include "chrome/updater/certificate_tag.h"
#include "codec/metadata_codec.h"

using updater::tagging::CreatePEBinary;
using updater::tagging::BinaryInterface;

extern "C" {

uint32_t WINAPI InsertPeTag(const wchar_t* filePath,
                            const wchar_t* outputFilePath,
                            const uint8_t* tagData,
                            uint32_t tagLen) {
  if (!filePath || !outputFilePath || !tagData || tagLen == 0) {
    return PETAG_E_INVALID_ARG;
  }

  std::string err;
  auto bytes = pctag::ReadFileBytes(filePath, &err);
  if (bytes.empty()) {
    return PETAG_E_IO;
  }
  if (!pctag::IsPEFile(bytes)) {
    return PETAG_E_FORMAT;
  }

  // Encode ASCII metadata into tag bytes inside DLL.
  std::string ascii(reinterpret_cast<const char*>(tagData), static_cast<size_t>(tagLen));
  auto encoded = pctag::EncodeMetadata(ascii, &err);
  if (encoded.empty()) {
    return PETAG_E_INVALID_ARG;
  }

  auto bin = CreatePEBinary(base::span<const uint8_t>(bytes.data(), bytes.size()));
  if (!bin) {
    bool fixed = false;
    // Attempt to normalize the WIN_CERTIFICATE header length field if mismatched.
    // Reuse CLI fallback by duplicating minimal logic here.
    if (bytes.size() >= sizeof(IMAGE_DOS_HEADER)) {
      auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
      if (dos->e_magic == 0x5A4D) {
        size_t pe_off = static_cast<size_t>(dos->e_lfanew);
        if (pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) <= bytes.size()) {
          size_t opt_off = pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
          if (opt_off + sizeof(IMAGE_OPTIONAL_HEADER32) <= bytes.size()) {
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
            }
            if (cert_va && cert_sz >= 8 && (size_t)cert_va + cert_sz <= bytes.size()) {
              uint32_t* len_ptr = reinterpret_cast<uint32_t*>(&bytes[cert_va]);
              if (*len_ptr != cert_sz) {
                *len_ptr = cert_sz;
                fixed = true;
              }
            }
          }
        }
      }
    }
    if (fixed) {
      bin = CreatePEBinary(base::span<const uint8_t>(bytes.data(), bytes.size()));
    }
    if (!bin) {
      return PETAG_E_CERT;
    }
  }

  auto updated = bin->SetTag(base::span<const uint8_t>(encoded.data(), encoded.size()));
  if (!updated) {
    return PETAG_E_CERT;
  }

  bool overwrite = wcscmp(filePath, outputFilePath) == 0 || pctag::FileExists(outputFilePath);
  if (!pctag::WriteFileBytes(outputFilePath, *updated, overwrite, &err)) {
    return PETAG_E_IO;
  }
  return PETAG_OK;
}

uint32_t WINAPI ReadPeTag(const wchar_t* filePath,
                          uint8_t* outTagData,
                          uint32_t outCapacity,
                          uint32_t* outLen) {
  if (!filePath || !outLen) {
    return PETAG_E_INVALID_ARG;
  }
  *outLen = 0;
  std::string err;
  auto bytes = pctag::ReadFileBytes(filePath, &err);
  if (bytes.empty()) {
    return PETAG_E_IO;
  }
  if (!pctag::IsPEFile(bytes)) {
    return PETAG_E_FORMAT;
  }
  auto bin = CreatePEBinary(base::span<const uint8_t>(bytes.data(), bytes.size()));
  if (!bin) {
    return PETAG_E_CERT;
  }
  auto t = bin->tag();
  if (!t || t->empty()) {
    return PETAG_E_NOT_FOUND;
  }
  // Decode tag bytes into ASCII metadata in DLL.
  std::string meta;
  if (!pctag::DecodeMetadata(*t, &meta, &err)) {
    return PETAG_E_CERT;
  }
  uint32_t need = static_cast<uint32_t>(meta.size());
  if (!outTagData || outCapacity < need) {
    *outLen = need;
    return PETAG_E_BUFFER_TOO_SMALL;
  }
  std::memcpy(outTagData, meta.data(), need);
  *outLen = need;
  return PETAG_OK;
}

}
