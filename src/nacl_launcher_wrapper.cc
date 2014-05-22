#include <node.h>
#include "nacl_launcher_wrapper.h"
#include "reverse_emulate.h"

#include "native_client/src/public/secure_service.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/desc/nrd_all_modules.h"
#include "native_client/src/trusted/desc/nacl_desc_wrapper.h"

#include "native_client/src/trusted/sel_universal/parsing.h"

#define THROW_ERROR_PRINTF(message_tpl, ...) {\
    char message[999];\
    snprintf(message,\
             sizeof(message),\
             message_tpl,\
             __VA_ARGS__);\
    return NanThrowError(message);\
  }


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
    NaClLog(LOG_FATAL, "NaClLauncherWrapper::LoadNexe: Failed to launch sel_ldr\n");
  }

  if (!launcher_.SetupCommand(&command_channel_)) {
    NaClLog(LOG_ERROR, "NaClLauncherWrapper::LoadNexe: set up command failed\n");
    return;
  }
  
  nacl::DescWrapper *host_file = factory_.OpenHostFile(nexe_name.c_str(), O_RDONLY, 0);
  if (NULL == host_file) {
    NaClLog(LOG_ERROR, "NaClLauncherWrapper::LoadNexe: could not open %s\n", nexe_name.c_str());
    return;
  }

  if (!launcher_.LoadModule(&command_channel_, host_file)) {
    NaClLog(LOG_ERROR, "NaClLauncherWrapper::LoadNexe: load module failed\n");
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
  tpl->PrototypeTemplate()->Set(String::NewSymbol("setupAppChannel"),
      FunctionTemplate::New(SetupAppChannel)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("setupReverseService"),
      FunctionTemplate::New(SetupReverseService)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("invoke"),
      FunctionTemplate::New(Invoke)->GetFunction());
  constructor = Persistent<Function>::New(tpl->GetFunction());

  constructor->Set(NanSymbol("CHANNEL_COMMAND"), Integer::New(CHANNEL_COMMAND));
  constructor->Set(NanSymbol("CHANNEL_APP"), Integer::New(CHANNEL_APP));
  constructor->Set(NanSymbol("CHANNEL_REVERSE"), Integer::New(CHANNEL_REVERSE));
  exports->Set(String::NewSymbol("NaClLauncherWrapper"), constructor);
}

NAN_METHOD(NaClLauncherWrapper::New) {
  NanScope();

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new NaClLauncherWrapper(...)`
    if (args[0]->IsUndefined()) {
      return NanThrowError("NaClLauncherWrapper::New: missing parameter: nexe");
    }
    String::Utf8Value nexe_name(args[0]->ToString());
    NaClLauncherWrapper* obj = new NaClLauncherWrapper();
    obj->LoadNexe(*nexe_name);
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
    return NanThrowError("NaClLauncherWrapper::Start: launcher#StartModule failed");
  }

  NanReturnValue(Boolean::New(true));
}

NAN_METHOD(NaClLauncherWrapper::GetServices) {
  NanScope();

  int channel = (args[0]->IsUndefined()) ? (int) CHANNEL_COMMAND : args[0]->IntegerValue();

  NaClLauncherWrapper* obj = ObjectWrap::Unwrap<NaClLauncherWrapper>(args.This());

  NaClSrpcChannel *selected_channel;

  switch (channel) {
    case CHANNEL_COMMAND:
      selected_channel = &obj->command_channel_;
      break;
    case CHANNEL_APP:
      selected_channel = &obj->app_channel_;
      break;
    default:
      return NanThrowError("NaClLauncherWrapper::GetServices: Invalid channel");
  }

  uint32_t method_count = NaClSrpcServiceMethodCount(selected_channel->client);
  Handle<Array> srpc_methods = Array::New(method_count);

  // Loop over SRPC methods
  for (uint32_t i = 0; i < method_count; ++i) {
    int retval;
    const char* method_name;
    const char* input_types;
    const char* output_types;

    retval = NaClSrpcServiceMethodNameAndTypes(selected_channel->client,
                                               i,
                                               &method_name,
                                               &input_types,
                                               &output_types);
    if (!retval) {
      return NanThrowError("NaClLauncherWrapper::GetServices: NaClSrpcServiceMethodNameAndTypes failed");
    }
    
    Handle<Array> rpc_desc = Array::New(3);
    rpc_desc->Set(0, String::New(method_name));
    rpc_desc->Set(1, String::New(input_types));
    rpc_desc->Set(2, String::New(output_types));

    srpc_methods->Set(i, rpc_desc);
  }

  NanReturnValue(srpc_methods);
}

NAN_METHOD(NaClLauncherWrapper::SetupAppChannel) {
  NanScope();

  NaClLauncherWrapper* obj = ObjectWrap::Unwrap<NaClLauncherWrapper>(args.This());

  if (!obj->launcher_.SetupAppChannel(&obj->app_channel_)) {
    return NanThrowError("NaClLauncherWrapper::SetupAppChannel: set up app channel failed");
  }

  NanReturnValue(Boolean::New(true));
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
    return NanThrowError("NaClLauncherWrapper::SetupReverseService: SRPC call to reverse_setup failed");
  }
  
  // Make a nacl::DescWrapper* from the NaClDesc*
  nacl::scoped_ptr<nacl::DescWrapper> conn_cap(obj->launcher_.WrapCleanup(h));
  if (conn_cap == NULL) {
    return NanThrowError("NaClLauncherWrapper::SetupReverseService: reverse desc wrap failed");
  }
  
  // The implementation of the ReverseInterface is our emulator class.
  if (args[0]->IsUndefined()) {
    return NanThrowError("NaClLauncherWrapper::SetupReverseService: missing parameter: ledger_entry_callback");
  }
  
  //Persistent<Function> callbackHandle = Persistent<Function>::New(Handle<Function>::Cast(args[0]));
  Local<Function> callbackHandle = args[0].As<Function>();
  NanCallback *ledger_entry_callback = new NanCallback(callbackHandle);
  nacl::scoped_ptr<ReverseEmulate> reverse_interface(new ReverseEmulate(ledger_entry_callback,
                                                                        Isolate::GetCurrent(),
                                                                        Persistent<Context>::New(Context::GetCurrent())));

  // Create an instance of ReverseService, which connects to the socket
  // address and exports the services from our emulator.
  obj->reverse_service_ = new nacl::ReverseService(conn_cap.get(),
                                                   reverse_interface->Ref());

  reverse_interface.release();
  conn_cap.release();

  // Starts the RPC handler for the reverse interface.
  if (!obj->reverse_service_->Start()) {
    return NanThrowError("NaClLauncherWrapper::SetupReverseService: reverse service start failed");
  }

  NanReturnValue(Boolean::New(true));
}

void BuildArgVec(NaClSrpcArg* argv[], NaClSrpcArg arg[], size_t count) {
  for (size_t i = 0; i < count; ++i) {
    NaClSrpcArgCtor(&arg[i]);
    argv[i] = &arg[i];
  }
  argv[count] = NULL;
}

NAN_METHOD(NaClLauncherWrapper::Invoke) {
  NanScope();

  int channel = args[0]->IntegerValue();
  String::Utf8Value rpc_signature(args[1]->ToString());

  NaClLauncherWrapper* obj = ObjectWrap::Unwrap<NaClLauncherWrapper>(args.This());

  NaClSrpcChannel *selected_channel;

  switch (channel) {
    case CHANNEL_COMMAND:
      selected_channel = &obj->command_channel_;
      break;
    case CHANNEL_APP:
      selected_channel = &obj->app_channel_;
      break;
    default:
      return NanThrowError("NaClLauncherWrapper::Invoke: Invalid channel");
  }

  uint32_t rpc_num = NaClSrpcServiceMethodIndex(selected_channel->client,
                                                *rpc_signature);
  if (rpc_num == kNaClSrpcInvalidMethodIndex) {
    THROW_ERROR_PRINTF("NaClLauncherWrapper::Invoke: SRPC method '%s' not found", *rpc_signature);
  }

  const char* rpc_name;
  const char* arg_types;
  const char* ret_types;

  if (!NaClSrpcServiceMethodNameAndTypes(selected_channel->client,
                                         rpc_num,
                                         &rpc_name,
                                         &arg_types,
                                         &ret_types)) {
    THROW_ERROR_PRINTF("NaClLauncherWrapper::Invoke: Unable to determine name and types for method %s (%d)", *rpc_signature, rpc_num);
  }

  NaClSrpcArg  in[NACL_SRPC_MAX_ARGS];
  NaClSrpcArg* inv[NACL_SRPC_MAX_ARGS + 1];

  BuildArgVec(inv, in, strlen(arg_types));

  printf ("%s\n", *rpc_signature);
  printf ("arg_types: %d\n", strlen(arg_types));
  printf ("ret_types: %d\n", strlen(ret_types));

  int i = 0;
  while (arg_types[i]) {
    char type = arg_types[i];
    NaClSrpcArg* arg = inv[i];
    switch (type) {
      case NACL_SRPC_ARG_TYPE_BOOL:
        arg->tag = NACL_SRPC_ARG_TYPE_BOOL;
        arg->u.bval = args[i+2]->BooleanValue();
        break;
      case NACL_SRPC_ARG_TYPE_INT:
        arg->tag = NACL_SRPC_ARG_TYPE_INT;
        arg->u.ival = args[i+2]->Int32Value();
        break;
      case NACL_SRPC_ARG_TYPE_STRING:
        arg->tag = NACL_SRPC_ARG_TYPE_STRING;
        size_t count;
        arg->arrays.str =
          strdup(NanCString(args[i+2], &count));
        if (NULL == arg->arrays.str) {
          NanThrowError("NaClLauncherWrapper:Invoke: string alloc problem");
        }
        printf ("rpc string arg: %s\n", arg->arrays.str);
        break;
      default:
        THROW_ERROR_PRINTF("NaClLauncherWrapper:Invoke: SRPC method requires unsupported argument type %c", type);
    }
    i++;
  }

  NaClSrpcArg  out[NACL_SRPC_MAX_ARGS];
  NaClSrpcArg* outv[NACL_SRPC_MAX_ARGS + 1];

  BuildArgVec(outv, out, strlen(ret_types));

  i = 0;
  while (ret_types[i]) {
    char type = ret_types[i];
    NaClSrpcArg* arg = outv[i];
    switch (type) {
      case NACL_SRPC_ARG_TYPE_INT:
        arg->tag = NACL_SRPC_ARG_TYPE_INT;
        arg->u.ival = 0;
        break;
      case NACL_SRPC_ARG_TYPE_STRING:
        arg->tag = NACL_SRPC_ARG_TYPE_STRING;
        arg->arrays.str = strdup("");
        break;
    }
    i++;
  }

  printf ("NaClSrpcInvokeV\n"); 
  fflush(stdout);

  //Isolate::GetCurrent()->Exit();
  NanUnlocker();
  const  NaClSrpcError result = NaClSrpcInvokeV(
    selected_channel, rpc_num, inv, outv);
  NanLocker();

  if (NACL_SRPC_RESULT_OK != result) {
    THROW_ERROR_PRINTF("NaClLauncherWrapper::Invoke: RPC call failed: %s", NaClSrpcErrorString(result));
  }

  i = 0;
  Handle<Array> return_values = Array::New(strlen(ret_types));
  while (ret_types[i]) {
    NaClSrpcArg* arg = outv[i];
    switch (arg->tag) {
      case NACL_SRPC_ARG_TYPE_INT:
        return_values->Set(i, Number::New(arg->u.ival));
        break;
      case NACL_SRPC_ARG_TYPE_STRING:
        arg->tag = NACL_SRPC_ARG_TYPE_STRING;
        return_values->Set(i, String::New(arg->arrays.str));
        break;
    }
    i++;
  }
  NanReturnValue(return_values);
}