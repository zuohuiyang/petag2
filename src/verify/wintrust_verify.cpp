#include "verify/wintrust_verify.h"
#include <windows.h>
#include <wintrust.h>
#include <softpub.h>

namespace pctag {

bool VerifyAuthenticode(const std::wstring& path, std::string* reason) {
  WINTRUST_FILE_INFO fileInfo{};
  fileInfo.cbStruct = sizeof(fileInfo);
  fileInfo.pcwszFilePath = path.c_str();

  GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
  WINTRUST_DATA data{};
  data.cbStruct = sizeof(data);
  data.dwUIChoice = WTD_UI_NONE;
  data.fdwRevocationChecks = WTD_REVOKE_NONE;
  data.dwUnionChoice = WTD_CHOICE_FILE;
  data.pFile = &fileInfo;
  data.dwStateAction = WTD_STATEACTION_VERIFY;
  data.dwProvFlags = WTD_REVOCATION_CHECK_NONE | WTD_CACHE_ONLY_URL_RETRIEVAL;

  LONG status = WinVerifyTrust(nullptr, &action, &data);
  data.dwStateAction = WTD_STATEACTION_CLOSE;
  WinVerifyTrust(nullptr, &action, &data);

  if (status == ERROR_SUCCESS) return true;
  if (reason) {
    switch (status) {
      case TRUST_E_NOSIGNATURE: *reason = "No signature present"; break;
      case TRUST_E_BAD_DIGEST: *reason = "Signature digest mismatch"; break;
      case CERT_E_UNTRUSTEDROOT: *reason = "Untrusted root certificate"; break;
      default: *reason = "Signature verification failed"; break;
    }
  }
  return false;
}

}

