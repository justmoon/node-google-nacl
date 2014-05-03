#include "reverse_emulate.h"

#include <cstring>

#include "native_client/src/trusted/validator/nacl_file_info.h"
#include "native_client/src/shared/platform/nacl_log.h"

ReverseEmulate::ReverseEmulate() {
  NaClLog(1, "ReverseEmulate::ReverseEmulate\n");
  NaClXMutexCtor(&mu_);
}

ReverseEmulate::~ReverseEmulate() {
  NaClLog(1, "ReverseEmulate::~ReverseEmulate\n");
  NaClMutexDtor(&mu_);
}

void ReverseEmulate::StartupInitializationComplete() {
  NaClLog(1, "ReverseEmulate::StartupInitializationComplete ()\n");
}

bool ReverseEmulate::OpenManifestEntry(nacl::string url_key,
                                       struct NaClFileInfo* info) {
  NaClLog(1, "ReverseEmulate::OpenManifestEntry (url_key=%s)\n",
          url_key.c_str());
  memset(info, 0, sizeof(*info));
  info->desc = -1;
  //info->desc = OPEN(pathname.c_str(), O_RDONLY);
  return info->desc >= 0;
}

bool ReverseEmulate::CloseManifestEntry(int32_t desc) {
  NaClLog(1, "ReverseEmulate::CloseManifestEntry (desc=%d)\n", desc);
  //CLOSE(desc);
  return true;
}

void ReverseEmulate::ReportCrash() {
  NaClLog(1, "ReverseEmulate::ReportCrash\n");
}

void ReverseEmulate::ReportExitStatus(int exit_status) {
  NaClLog(1, "ReverseEmulate::ReportExitStatus (exit_status=%d)\n",
          exit_status);
}

void ReverseEmulate::DoPostMessage(nacl::string message) {
  NaClLog(1, "ReverseEmulate::DoPostMessage (message=%s)\n", message.c_str());
}