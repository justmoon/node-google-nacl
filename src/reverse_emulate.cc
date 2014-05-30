#include "reverse_emulate.h"

#include <cstring>

#include "native_client/src/trusted/validator/nacl_file_info.h"
#include "native_client/src/shared/platform/nacl_log.h"

using namespace v8;

ReverseEmulate::ReverseEmulate(NanCallback *request_account_txs,
                               NanCallback *submit_payment_tx,
                               Isolate *isolate,
                               v8::Persistent<v8::Context> context) : 
    request_account_txs_(request_account_txs),
    submit_payment_tx_(submit_payment_tx),
    isolate_(isolate),
    context_(context) {
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

bool ReverseEmulate::ReadRippleLedger(nacl::string ledger_hash,
                                      nacl::string* ledger_data) {
  NaClLog(1, "ReverseEmulate::ReadRippleLedger (ledger_hash=%s)\n", ledger_hash.c_str());
  if (ledger_hash.empty()) {
    NaClLog(LOG_ERROR,
            "ReverseEmulate::ReadRippleLedger:"
            " missing ledger_hash\n");
    return false;
  }

  Locker v8Locker(isolate_);
  Isolate::Scope isolateScope(isolate_);
  HandleScope handle_scope;
  Context::Scope context_scope(context_);

  if (request_account_txs_!=NULL) {
    Local<Value> argv[] = {
      Local<Value>::New(Null()),
      String::New(ledger_hash.c_str())
    };
    request_account_txs_->Call(2, argv); 
  }
  
  *ledger_data = "Hello Ripple Ledger!";
  return true;
}

void ReverseEmulate::GetRippleAccountTxs(nacl::string account,
                                         int          ledger_index_min,
                                         int          ledger_index_max,
                                         nacl::string callback) {
  NaClLog(1, "ReverseEmulate::GetRippleAccountTxs\n");
  if (account.empty() || callback.empty()) {
    NaClLog(LOG_ERROR,
            "ReverseEmulate::GetRippleAccountTxs:"
            " missing data\n");
    return;
  }

  Locker v8Locker(isolate_);
  Isolate::Scope isolateScope(isolate_);
  HandleScope handle_scope;
  Context::Scope context_scope(context_);

  Local<Object> result = Object::New();
  result->Set (String::New("account"), String::New(account.c_str()));
  result->Set (String::New("ledger_index_min"), Number::New(ledger_index_min));
  result->Set (String::New("ledger_index_max"), Number::New(ledger_index_max));
  result->Set (String::New("callback"), String::New(callback.c_str()));
  if (request_account_txs_!=NULL) {
    Local<Value> argv[] = {
      Local<Value>::New(Null()),
      result
    };
    request_account_txs_->Call(2, argv); 
  }
}

void ReverseEmulate::SubmitRipplePaymentTx(nacl::string account,
                                           nacl::string secret,
                                           nacl::string recipient,
                                           nacl::string amount,
                                           nacl::string currency,
                                           nacl::string issuer,
                                           nacl::string callback) {
  NaClLog(1, "ReverseEmulate::SubmitRipplePaymentTx\n");

  /* currency, issuer, and callback are optional parameters. */
  if (account.empty() || secret.empty() ||
      recipient.empty() || amount.empty()) {
    NaClLog(LOG_ERROR,
            "ReverseEmulate::SubmitRipplePaymentTx:"
            " missing data\n");
    return;
  }

  Locker v8Locker(isolate_);
  Isolate::Scope isolateScope(isolate_);
  HandleScope handle_scope;
  Context::Scope context_scope(context_);

  Local<Object> result = Object::New();
  result->Set (String::New("account"),   String::New(account.c_str()));
  result->Set (String::New("secret"),    String::New(secret.c_str()));
  result->Set (String::New("recipient"), String::New(recipient.c_str()));
  result->Set (String::New("amount"),    String::New(amount.c_str()));
  result->Set (String::New("currency"),  String::New(currency.c_str()));
  result->Set (String::New("issuer"),    String::New(issuer.c_str()));
  result->Set (String::New("callback"),  String::New(callback.c_str()));
  if (submit_payment_tx_!=NULL) {
    Local<Value> argv[] = {
      Local<Value>::New(Null()),
      result
    };
    submit_payment_tx_->Call(2, argv); 
  }
}