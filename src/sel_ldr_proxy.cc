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


  std::vector<nacl::string> command_prefix;
  std::vector<nacl::string> sel_ldr_argv;
  std::vector<nacl::string> app_argv;

  nacl::SelLdrLauncherStandalone launcher;
  nacl::DescWrapperFactory factory;

  if (!launcher.StartViaCommandLine(command_prefix, sel_ldr_argv, app_argv)) {
    NaClLog(LOG_FATAL, "sel_ldr_proxy: Failed to launch sel_ldr\n");
  }

  NaClSrpcChannel command_channel;

  if (!launcher.SetupCommand(&command_channel)) {
    NaClLog(LOG_ERROR, "sel_ldr_proxy: set up command failed\n");
    return;
  }

  std::string app_name = "srpc_test_nonbrowser.nexe";

  DescWrapper *host_file = factory.OpenHostFile(app_name.c_str(), O_RDONLY, 0);
  if (NULL == host_file) {
    NaClLog(LOG_ERROR, "sel_ldr_proxy: could not open %s\n", app_name.c_str());
    return;
  }

  if (!launcher.LoadModule(&command_channel, host_file)) {
    NaClLog(LOG_ERROR, "sel_ldr_proxy: load module failed\n");
    return;
  }

  delete host_file;

  if (!launcher.StartModule(&command_channel)) {
    NaClLog(LOG_ERROR,
            "sel_ldr_proxy: start module failed\n");
    return;
  }

  NaClSrpcServicePrint(command_channel.client);

  NaClSrpcChannel channel;

  if (!launcher.SetupAppChannel(&channel)) {
    NaClLog(LOG_ERROR,
            "sel_ldr_proxy: set up app channel failed\n");
    return;
  }

  NaClSrpcServicePrint(channel.client);

  int ret;

  if (NACL_SRPC_RESULT_OK !=
      NaClSrpcInvokeBySignature(&channel, "int:i:i", 5, &ret)) {
    NaClLog(LOG_ERROR, "RPC call failed with status: %d\n", ret);
    return;
  }
  NaClLog(LOG_INFO, "RPC call succeeded with status: %d\n", ret);

  NaClLauncherWrapper::Init(target);

  // don't need to be read-only, only used by the JS shim
  //target->Set(NanSymbol("AF_UNIX"), Integer::New(AF_UNIX));
  //target->Set(NanSymbol("SOCK_DGRAM"), Integer::New(SOCK_DGRAM));

  //target->Set(NanSymbol("bind"),
  //            FunctionTemplate::New(Bind)->GetFunction());

  //target->Set(NanSymbol("send"),
  //            FunctionTemplate::New(Send)->GetFunction());

  //target->Set(NanSymbol("close"),
  //            FunctionTemplate::New(Close)->GetFunction());
}


} // anonymous namespace

NODE_MODULE(sel_ldr_proxy, Initialize)
