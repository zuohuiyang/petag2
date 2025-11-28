#include <windows.h>
#include <string>

#include "base/logging.h"
#include "petag_api.h"

typedef uint32_t (WINAPI *InsertPeTagFn)(const wchar_t* filePath,
                                         const wchar_t* outputFilePath,
                                         const char* tagData,
                                         uint32_t tagLen);
typedef uint32_t (WINAPI *ReadPeTagFn)(const wchar_t* filePath,
                                       char* outTagData,
                                       uint32_t outCapacity,
                                       uint32_t* outLen);

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

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintHelp();
    return 1;
  }
  std::string cmd = argv[1];
  if (cmd == "--help" || cmd == "-h") { PrintHelp(); return 0; }

  HMODULE h = LoadLibraryW(L"petag.dll");
  if (!h) {
    pctag::LogError("Failed to load petag.dll");
    return 1;
  }
  InsertPeTagFn InsertPeTag =
      reinterpret_cast<InsertPeTagFn>(GetProcAddress(h, "InsertPeTag"));
  ReadPeTagFn ReadPeTag =
      reinterpret_cast<ReadPeTagFn>(GetProcAddress(h, "ReadPeTag"));
  if (!InsertPeTag || !ReadPeTag) {
    pctag::LogError("Missing DLL exports");
    FreeLibrary(h);
    return 1;
  }

  if (cmd == "-e" || cmd == "--read") {
    if (argc != 3) {
      pctag::LogError("Invalid arguments for read");
      FreeLibrary(h);
      return 1;
    }
    std::wstring in = Utf8ToWide(argv[2]);
    uint32_t need = 0;
    constexpr size_t kSmallBufferSize = 64;
    char small[kSmallBufferSize];
    uint32_t out_len = 0;
    uint32_t st = ReadPeTag(in.c_str(), small, (uint32_t)sizeof(small), &out_len);
    if (st == PETAG_E_NOT_FOUND) { pctag::LogInfo("No metadata tag found"); FreeLibrary(h); return 0; }
    if (st == PETAG_E_BUFFER_TOO_SMALL) {
      need = out_len;
      std::string buf; buf.resize(need);
      st = ReadPeTag(in.c_str(), &buf[0], (uint32_t)buf.size(), &out_len);
      if (st != PETAG_OK) {
        pctag::LogError("Failed to read tag, code: " + std::to_string(st));
        FreeLibrary(h);
        return 1;
      }
      pctag::LogInfo("Metadata: " + buf);
      FreeLibrary(h);
      return 0;
    }
    if (st != PETAG_OK) {
      pctag::LogError("Failed to read tag, code: " + std::to_string(st));
      FreeLibrary(h);
      return 1;
    }
    std::string meta(small, small + out_len);
    pctag::LogInfo("Metadata: " + meta);
    FreeLibrary(h);
    return 0;
  }
  if (cmd == "-i" || cmd == "--insert") {
    if (argc != 5) {
      pctag::LogError("Invalid arguments for insert");
      PrintHelp();
      FreeLibrary(h);
      return 1;
    }
    std::wstring in = Utf8ToWide(argv[2]);
    std::wstring out = Utf8ToWide(argv[3]);
    std::string meta_arg = argv[4];
    uint32_t st = InsertPeTag(in.c_str(), out.c_str(), meta_arg.c_str(), (uint32_t)meta_arg.size());
    if (st != PETAG_OK) {
      pctag::LogError("Failed to insert tag, code: " + std::to_string(st));
      FreeLibrary(h);
      return 1;
    }
    pctag::LogInfo("Tag inserted");
    FreeLibrary(h);
    return 0;
  }
  pctag::LogError("Unknown command");
  PrintHelp();
  FreeLibrary(h);
  return 1;
}
