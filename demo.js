//var SegfaultHandler = require('segfault-handler');
//SegfaultHandler.registerHandler();

var binding = require('bindings')('sel_ldr_proxy.node'),
    NaClLauncherWrapper = binding.NaClLauncherWrapper;

/* Loading ripple-lib with Node.js */
var Remote = require('ripple-lib').Remote;

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

function ledger_entry_callback (err, result) {
  if (err) {
    console.log(err);
  } else {
    console.log(result);
    //Get ledger data then call back into the app channel.
  }
}

function ledgerListener (ledger_data) {
  console.log(ledger_data);
  
  var launcher = new NaClLauncherWrapper("read_ledger.nexe");
  launcher.setupReverseService(ledger_entry_callback);
  launcher.start();
  launcher.setupAppChannel();
  launcher.invoke(NaClLauncherWrapper.CHANNEL_APP, "new_ledger:s:", ledger_data.ledger_hash);
}

remote.connect(function() {
  /* remote connected */
  remote.on('ledger_closed', ledgerListener);
});