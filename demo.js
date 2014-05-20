//var SegfaultHandler = require('segfault-handler');
//SegfaultHandler.registerHandler();

var binding = require('bindings')('sel_ldr_proxy.node'),
    NaClLauncherWrapper = binding.NaClLauncherWrapper;

function ledger_entry_callback (err, result) {
    console.log("In the callback!");
    console.log(result);
}

var launcher = new NaClLauncherWrapper("read_ledger.nexe");
console.log(launcher.setupReverseService(ledger_entry_callback));
console.log(launcher.start());
console.log(launcher.setupAppChannel());
console.log(launcher.getServices(NaClLauncherWrapper.CHANNEL_COMMAND));
console.log(launcher.getServices(NaClLauncherWrapper.CHANNEL_APP));
console.log(launcher.invoke(NaClLauncherWrapper.CHANNEL_APP, "new_ledger:s:", "ASDF"));
