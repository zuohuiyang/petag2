#ifndef PETAG_API_H_
#define PETAG_API_H_

/*
 * PETAG DLL C Interface
 *
 * Overview:
 * - Exposes string-only metadata read/write APIs; metadata must be printable ASCII, length 1-255.
 * - Uses WINAPI (__stdcall) calling convention; export controlled by PETAG_API macro.
 * - Returns PetagStatus codes; see the enum description below.
 */

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
  PETAG_OK = 0,                    // success
  PETAG_E_INVALID_ARG = 100,       // invalid argument (null pointer, bad length, ASCII check failed, etc.)
  PETAG_E_BUFFER_TOO_SMALL = 110,  // output buffer too small on read; required length via outLen
  PETAG_E_IO = 200,                // file I/O failure
  PETAG_E_FORMAT = 201,            // file format error (e.g., not a PE)
  PETAG_E_CERT = 300,              // certificate/tag parse or write failure
  PETAG_E_NOT_FOUND = 404,         // tag not found
};

/*
 * Write a metadata tag to a signed PE without breaking signature validity.
 *
 * Params:
 * - filePath        input PE path (wide string)
 * - outputFilePath  output path (may equal input to overwrite)
 * - tagData         metadata string (printable ASCII, length 1-255; recommended: JSON)
 * - tagLen          metadata length in bytes
 * Returns: PetagStatus
 */
PETAG_API uint32_t WINAPI InsertPeTag(
    const wchar_t* filePath,
    const wchar_t* outputFilePath,
    const char* tagData,
    uint32_t tagLen);

/*
 * Read the metadata tag from a signed PE as a string.
 *
 * Params:
 * - filePath    input PE path (wide string)
 * - outTagData  output buffer (char*), allocated by caller
 * - outCapacity output buffer size in bytes
 * - outLen      actual length written; when buffer is insufficient, required length is returned
 * Returns: PetagStatus; on PETAG_E_BUFFER_TOO_SMALL caller should reallocate per outLen and retry
 */
PETAG_API uint32_t WINAPI ReadPeTag(
    const wchar_t* filePath,
    char* outTagData,
    uint32_t outCapacity,
    uint32_t* outLen);

#ifdef __cplusplus
}
#endif

#endif  // PETAG_API_H_

