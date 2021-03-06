#ifndef NACL_LAUNCHER_WRAPPER_H
#define NACL_LAUNCHER_WRAPPER_H

#include <nan.h>

#include "native_client/src/trusted/nonnacl_util/sel_ldr_launcher.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"

namespace nacl {
class ReverseService;
}  // namespace nacl

class NaClLauncherWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

  enum {
    CHANNEL_COMMAND,
    CHANNEL_APP,
    CHANNEL_REVERSE
  };
 private:
  explicit NaClLauncherWrapper();
  ~NaClLauncherWrapper();
  
  void LoadNexe(std::string nexe_name);

  static NAN_METHOD(New);
  static NAN_METHOD(Start);
  static NAN_METHOD(GetServices);
  static NAN_METHOD(SetupAppChannel);
  static NAN_METHOD(SetupReverseService);
  static NAN_METHOD(Invoke);
  static v8::Persistent<v8::Function> constructor;

  nacl::SelLdrLauncherStandalone launcher_;
  nacl::DescWrapperFactory factory_;

  NaClSrpcChannel command_channel_;
  NaClSrpcChannel app_channel_;
  nacl::ReverseService *reverse_service_;
};

#endif
