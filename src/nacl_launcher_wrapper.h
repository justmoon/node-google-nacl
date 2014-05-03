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

 private:
  explicit NaClLauncherWrapper();
  ~NaClLauncherWrapper();
  
  void LoadNexe(std::string nexe_name);

  static NAN_METHOD(New);
  static NAN_METHOD(Start);
  static NAN_METHOD(GetServices);
  static NAN_METHOD(SetupReverseService);
  static v8::Persistent<v8::Function> constructor;

  nacl::SelLdrLauncherStandalone launcher_;
  nacl::DescWrapperFactory factory_;

  NaClSrpcChannel command_channel_;
  nacl::ReverseService *reverse_service_;
};

#endif