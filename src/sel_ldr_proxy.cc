// Modeled after: https://github.com/eseidel/native_client_patches/blob/master/tests/codelab/sel_ldr_launcher/

#define NACL_LINUX 1

#include <nan.h>
#include <vector>

#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/desc/nrd_all_modules.h"
#include "native_client/src/trusted/desc/nacl_desc_wrapper.h"
#include "native_client/src/trusted/nonnacl_util/sel_ldr_launcher.h"

#include "nacl_launcher_wrapper.h"

namespace {

using v8::Handle;
using v8::Object;
using v8::FunctionTemplate;
using nacl::DescWrapper;

void Initialize(Handle<Object> target) {
  NaClSrpcModuleInit();
  NaClNrdAllModulesInit();

  NaClLauncherWrapper::Init(target);
}


} // anonymous namespace

NODE_MODULE(sel_ldr_proxy, Initialize)
