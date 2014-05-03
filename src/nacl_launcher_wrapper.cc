#include <node.h>
#include "nacl_launcher_wrapper.h"
#include "reverse_emulate.h"

#include "native_client/src/public/secure_service.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/desc/nrd_all_modules.h"
#include "native_client/src/trusted/desc/nacl_desc_wrapper.h"

using namespace v8;

Persistent<Function> NaClLauncherWrapper::constructor;

NaClLauncherWrapper::NaClLauncherWrapper() : launcher_() {
}

NaClLauncherWrapper::~NaClLauncherWrapper() {
}

void NaClLauncherWrapper::LoadNexe(std::string nexe_name) {
  std::vector<nacl::string> command_prefix;
  std::vector<nacl::string> sel_ldr_argv;
  std::vector<nacl::string> app_argv;

  if (!launcher_.StartViaCommandLine(command_prefix, sel_ldr_argv, app_argv)) {
    NaClLog(LOG_FATAL, "sel_ldr_proxy: Failed to launch sel_ldr\n");
  }

  if (!launcher_.SetupCommand(&command_channel_)) {
    NaClLog(LOG_ERROR, "sel_ldr_proxy: set up command failed\n");
    return;
  }
  
  nacl::DescWrapper *host_file = factory_.OpenHostFile(nexe_name.c_str(), O_RDONLY, 0);
  if (NULL == host_file) {
    NaClLog(LOG_ERROR, "sel_ldr_proxy: could not open %s\n", nexe_name.c_str());
    return;
  }

  if (!launcher_.LoadModule(&command_channel_, host_file)) {
    NaClLog(LOG_ERROR, "sel_ldr_proxy: load module failed\n");
    return;
  }
  
  delete host_file;
}

void NaClLauncherWrapper::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("NaClLauncherWrapper"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("start"),
      FunctionTemplate::New(Start)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("getServices"),
      FunctionTemplate::New(GetServices)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("setupReverseService"),
      FunctionTemplate::New(SetupReverseService)->GetFunction());
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("NaClLauncherWrapper"), constructor);
}

NAN_METHOD(NaClLauncherWrapper::New) {
  NanScope();

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new NaClLauncherWrapper(...)`
    if (args[0]->IsUndefined()) {
      return NanThrowError("Missing parameter: nexe");
    }
    String::Utf8Value nexeName(args[0]->ToString());
    NaClLauncherWrapper* obj = new NaClLauncherWrapper();
    obj->LoadNexe(*nexeName);
    obj->Wrap(args.This());
    return args.This();
  } else {
    // Invoked as plain function `NaClLauncherWrapper(...)`, turn into construct call.
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    NanReturnValue(constructor->NewInstance(argc, argv));
  }
}

NAN_METHOD(NaClLauncherWrapper::Start) {
  NanScope();

  NaClLauncherWrapper* obj = ObjectWrap::Unwrap<NaClLauncherWrapper>(args.This());

  if (!obj->launcher_.StartModule(&obj->command_channel_)) {
    return NanThrowError("NaClLauncherWrapper: launcher#StartModule failed");
  }

  NanReturnValue(Boolean::New(true));
}

NAN_METHOD(NaClLauncherWrapper::GetServices) {
  NanScope();

  NaClLauncherWrapper* obj = ObjectWrap::Unwrap<NaClLauncherWrapper>(args.This());

  uint32_t method_count = NaClSrpcServiceMethodCount(obj->command_channel_.client);
  Handle<Array> srpc_methods = Array::New(method_count);

  // Loop over SRPC methods
  for (uint32_t i = 0; i < method_count; ++i) {
    int retval;
    const char* method_name;
    const char* input_types;
    const char* output_types;

    retval = NaClSrpcServiceMethodNameAndTypes(obj->command_channel_.client,
                                               i,
                                               &method_name,
                                               &input_types,
                                               &output_types);
    if (!retval) {
      return NanThrowError("NaClLauncherWrapper#getServices: launcher#StartModule failed");
    }
    
    srpc_methods->Set(i, String::New(method_name));
  }

  NanReturnValue(srpc_methods);
}

NAN_METHOD(NaClLauncherWrapper::SetupReverseService) {
  NanScope();

  NaClLauncherWrapper* obj = ObjectWrap::Unwrap<NaClLauncherWrapper>(args.This());

  NaClDesc* h;
  NaClSrpcResultCodes rpc_result =
      NaClSrpcInvokeBySignature(&obj->command_channel_,
                                NACL_SECURE_SERVICE_REVERSE_SETUP,
                                &h);
  
  if (NACL_SRPC_RESULT_OK != rpc_result) {
    return NanThrowError("NaClLauncherWrapper#setupReverseService: SRPC call to reverse_setup failed");
  }
  
  // Make a nacl::DescWrapper* from the NaClDesc*
  nacl::scoped_ptr<nacl::DescWrapper> conn_cap(obj->launcher_.WrapCleanup(h));
  if (conn_cap == NULL) {
    return NanThrowError("NaClLauncherWrapper#setupReverseService: reverse desc wrap failed");
  }
  
  // The implementation of the ReverseInterface is our emulator class.
  nacl::scoped_ptr<ReverseEmulate> reverse_interface(new ReverseEmulate());
  
  // Create an instance of ReverseService, which connects to the socket
  // address and exports the services from our emulator.
  obj->reverse_service_ = new nacl::ReverseService(conn_cap.get(),
                                                   reverse_interface->Ref());

  reverse_interface.release();

  NanReturnValue(Boolean::New(true));
}