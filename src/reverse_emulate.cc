#include "reverse_emulate.h"

#include <cstring>

#include "native_client/src/trusted/validator/nacl_file_info.h"
#include "native_client/src/shared/platform/nacl_log.h"

using namespace v8;

ReverseEmulate::ReverseEmulate(NanCallback *ledger_entry_callback) : 
    ledger_entry_callback_(ledger_entry_callback) {
  NaClLog(1, "ReverseEmulate::ReverseEmulate\n");
  NaClXMutexCtor(&mu_);

  ledger_entry_callback_ = ledger_entry_callback;
  Local<Value> argv[] = {
    Local<Value>::New(Null()),
    String::New("Hi ReverseEmulate constructor!")
  };
  ledger_entry_callback_->Call(2, argv);
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

bool ReverseEmulate::ReadRippleLedger(nacl::string ledger_hash,
                                      nacl::string* ledger_data) {
  NaClLog(1, "ReverseEmulate::ReadRippleLedger (ledger_hash=%s)\n", ledger_hash.c_str());
  if (ledger_hash.empty() && 0) {
    NaClLog(LOG_ERROR,
            "ReverseEmulate::ReadRippleLedger:"
            " missing ledger_hash\n");
    return false;
  }

  if (ledger_entry_callback_!=NULL) {
    Local<Value> argv[] = {
      Local<Value>::New(Null()),
      //String::New(ledger_hash.c_str())
      String::New("Hi ReadRippleLedger!")
    };
    ledger_entry_callback_->Call(2, argv); 
  }
  
  *ledger_data = "Hello Ripple Ledger!";
  return true;
}

void ReverseEmulate::RippleLedgerEntry(nacl::string ledger_hash) {
  NaClLog(1, "ReverseEmulate::RippleLedgerEntry (ledger_hash=%s)\n", ledger_hash.c_str());
}