//var SegfaultHandler = require('segfault-handler');
//SegfaultHandler.registerHandler();

var binding = require('bindings')('sel_ldr_proxy.node'),
    NaClLauncherWrapper = binding.NaClLauncherWrapper;

/* Loading ripple-lib with Node.js */
var Remote = require('ripple-lib').Remote;
var Amount = require('ripple-lib').Amount;

var remote = new Remote({
  trusted:        true,
  local_signing:  true,
  local_fee:      true, 
  fee_cushion:     1.5,
  servers: [
    {
        host:    's1.ripple.com'
      , port:    443
      , secure:  true
    }
  ]
});


function request_account_txs (err, result) {
  if (err) {
    console.log(err);
  } else {

    /* Get account transactions from the specified account. */    
    remote.request_account_tx({
      account: result.account,
      ledger_index_min: result.ledger_index,
      ledger_index_max: result.ledger_index
    }, function(err, res) {
      if (err) {
        console.log(err);
      } else {        
        if (res.transactions) {
          for (var i=0; i<res.transactions.length; i++) {
            console.log(res.transactions[i].tx);
            if (res.transactions[i].tx) {
              launcher.invoke(NaClLauncherWrapper.CHANNEL_APP,
                              result.callback,
                              JSON.stringify(res.transactions[i].tx));
            }
          }
        } else {
          console.log('Missing transaction(s):');
          console.log(res);
        }
      }
    });
  }
};


function submit_payment_tx (err, result) {
  if (err) {
    console.log(err);
  } else {
    remote.set_secret(result.account, result.secret);
    var transaction = remote.transaction();
    var amount;
    if (result.currency) {
      amount = new Amount.from_json({
        'value': result.amount,
        'currency': result.currency,
        'issuer': result.issuer
      });

    } else {
      /* XRP payment */
      amount = result.amount;
    }

    transaction.payment({
      from: result.account,
      to: result.recipient,
      amount: amount
    });

    transaction.submit(function(err, result) {
      if (err) {
        console.log(err);
      } else {
        console.log(result);
      }
    });
  }
}


function ledgerListener (ledger_data) {
  console.log (ledger_data.ledger_index);

  /* Inform the nexe of a new closed ledger. */
  launcher.invoke(NaClLauncherWrapper.CHANNEL_APP, 'new_ledger:s:', JSON.stringify(ledger_data));
}


var launcher = new NaClLauncherWrapper('contract.nexe');
launcher.setupReverseService(request_account_txs, submit_payment_tx);
launcher.start();
launcher.setupAppChannel();

remote.connect(function() {
  console.log ('Connected')

  remote.on('ledger_closed', ledgerListener);
});