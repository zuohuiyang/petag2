#include "petag_api.h"

#include <windows.h>
#include <cstring>
#include <string>
#include <vector>

#include "base/io.h"
#include "chrome/updater/certificate_tag.h"
#include "codec/metadata_codec.h"

extern "C" {

uint32_t WINAPI InsertPeTag(const wchar_t* filePath,
                            const wchar_t* outputFilePath,
                            const char* tagData,
                            uint32_t tagLen) {
  if (!filePath || !outputFilePath || !tagData || tagLen == 0) {
    return PETAG_E_INVALID_ARG;
  }

  std::string err;
  std::vector<uint8_t> bytes = pctag::ReadFileBytes(filePath, &err);
  if (bytes.empty()) {
    return PETAG_E_IO;
  }
  if (!pctag::IsPEFile(bytes)) {
    return PETAG_E_FORMAT;
  }

  std::string ascii(tagData, tagData + static_cast<size_t>(tagLen));
  std::vector<uint8_t> encoded = pctag::EncodeMetadata(ascii, &err);
  if (encoded.empty()) {
    return PETAG_E_INVALID_ARG;
  }

  std::unique_ptr<updater::tagging::BinaryInterface> bin =
      updater::tagging::CreatePEBinary(
          base::span<const uint8_t>(bytes.data(), bytes.size()));
  if (!bin) {
    bool fixed = false;
    static constexpr uint16_t kDosMagic = 0x5A4D;
    static constexpr uint32_t kCertDirIndex = 4;
    // Attempt to normalize the WIN_CERTIFICATE header length field if mismatched.
    // Reuse CLI fallback by duplicating minimal logic here.
    if (bytes.size() >= sizeof(IMAGE_DOS_HEADER)) {
      const IMAGE_DOS_HEADER* dos =
          reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
      if (dos->e_magic == kDosMagic) {
        size_t pe_off = static_cast<size_t>(dos->e_lfanew);
        if (pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) <= bytes.size()) {
          size_t opt_off = pe_off + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
          if (opt_off + sizeof(IMAGE_OPTIONAL_HEADER32) <= bytes.size()) {
            uint16_t opt_magic = *reinterpret_cast<const uint16_t*>(bytes.data() + opt_off);
            uint32_t cert_va = 0, cert_sz = 0;
            if (opt_magic == 0x10b) {
              const IMAGE_OPTIONAL_HEADER32* opt =
                  reinterpret_cast<const IMAGE_OPTIONAL_HEADER32*>(bytes.data() + opt_off);
              cert_va = opt->DataDirectory[kCertDirIndex].VirtualAddress;
              cert_sz = opt->DataDirectory[kCertDirIndex].Size;
            } else if (opt_magic == 0x20b) {
              const IMAGE_OPTIONAL_HEADER64* opt =
                  reinterpret_cast<const IMAGE_OPTIONAL_HEADER64*>(bytes.data() + opt_off);
              cert_va = opt->DataDirectory[kCertDirIndex].VirtualAddress;
              cert_sz = opt->DataDirectory[kCertDirIndex].Size;
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
      bin = updater::tagging::CreatePEBinary(
          base::span<const uint8_t>(bytes.data(), bytes.size()));
    }
    if (!bin) {
      return PETAG_E_CERT;
    }
  }

  std::optional<std::vector<uint8_t>> updated =
      bin->SetTag(base::span<const uint8_t>(encoded.data(), encoded.size()));
  if (!updated) {
    return PETAG_E_CERT;
  }

  bool overwrite = wcscmp(filePath, outputFilePath) == 0 ||
                   pctag::FileExists(outputFilePath);
  if (!pctag::WriteFileBytes(outputFilePath, *updated, overwrite, &err)) {
    return PETAG_E_IO;
  }
  return PETAG_OK;
}

uint32_t WINAPI ReadPeTag(const wchar_t* filePath,
                          char* outTagData,
                          uint32_t outCapacity,
                          uint32_t* outLen) {
  if (!filePath || !outLen) {
    return PETAG_E_INVALID_ARG;
  }
  *outLen = 0;
  std::string err;
  std::vector<uint8_t> bytes = pctag::ReadFileBytes(filePath, &err);
  if (bytes.empty()) {
    return PETAG_E_IO;
  }
  if (!pctag::IsPEFile(bytes)) {
    return PETAG_E_FORMAT;
  }
  std::unique_ptr<updater::tagging::BinaryInterface> bin =
      updater::tagging::CreatePEBinary(
          base::span<const uint8_t>(bytes.data(), bytes.size()));
  if (!bin) {
    return PETAG_E_CERT;
  }
  std::optional<std::vector<uint8_t>> t = bin->tag();
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
