#ifndef PETAG_API_H_
#define PETAG_API_H_

#include <stdint.h>
#include <wchar.h>

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifdef PETAG_DLL_EXPORTS
#define PETAG_API __declspec(dllexport)
#else
#define PETAG_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum PetagStatus : uint32_t {
  PETAG_OK = 0,
  PETAG_E_INVALID_ARG = 100,
  PETAG_E_BUFFER_TOO_SMALL = 110,
  PETAG_E_IO = 200,
  PETAG_E_FORMAT = 201,
  PETAG_E_CERT = 300,
  PETAG_E_NOT_FOUND = 404,
};

PETAG_API uint32_t WINAPI InsertPeTag(
    const wchar_t* filePath,
    const wchar_t* outputFilePath,
    const char* tagData,
    uint32_t tagLen);

PETAG_API uint32_t WINAPI ReadPeTag(
    const wchar_t* filePath,
    char* outTagData,
    uint32_t outCapacity,
    uint32_t* outLen);

#ifdef __cplusplus
}
#endif

#endif  // PETAG_API_H_

