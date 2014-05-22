#ifndef NODE_GOOGLE_NACL_REVERSE_EMULATE_H
#define NODE_GOOGLE_NACL_REVERSE_EMULATE_H

#include <nan.h>
#include "native_client/src/trusted/reverse_service/reverse_service.h"

// Mock of ReverseInterface for use by nexes.
class ReverseEmulate : public nacl::ReverseInterface {
public:
  ReverseEmulate(NanCallback *ledger_entry_callback,
                 v8::Isolate *isolate,
                 v8::Persistent<v8::Context> context);
  virtual ~ReverseEmulate();
  
  // Startup handshake
  virtual void StartupInitializationComplete();
  
  // Name service use.
  virtual bool OpenManifestEntry(nacl::string url_key,
                                 struct NaClFileInfo* info);
  virtual bool CloseManifestEntry(int32_t desc);
  virtual void ReportCrash();

  // The low-order 8 bits of the |exit_status| should be reported to
  // any interested parties.
  virtual void ReportExitStatus(int exit_status);
  
  // Send a string as a PostMessage to the browser.
  virtual void DoPostMessage(nacl::string message);

  // Read ripple ledger.
  virtual bool ReadRippleLedger(nacl::string ledger_hash,
                                nacl::string* ledger_data);

  // Request ripple ledger entry.
  virtual void RippleLedgerEntry(nacl::string ledger_hash);

private:
  NaClMutex mu_;

  NanCallback *ledger_entry_callback_;
  v8::Isolate *isolate_;
  v8::Persistent<v8::Context> context_;
};

#endif