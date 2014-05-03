#ifndef NODE_GOOGLE_NACL_REVERSE_EMULATE_H
#define NODE_GOOGLE_NACL_REVERSE_EMULATE_H

#include "native_client/src/trusted/reverse_service/reverse_service.h"

// Mock of ReverseInterface for use by nexes.
class ReverseEmulate : public nacl::ReverseInterface {
public:
  ReverseEmulate();
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
  
private:
  NaClMutex mu_;
};

#endif