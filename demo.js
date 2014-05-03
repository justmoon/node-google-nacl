var binding = require('bindings')('sel_ldr_proxy.node'),
    NaClLauncherWrapper = binding.NaClLauncherWrapper;


console.log(NaClLauncherWrapper);

var launcher = new NaClLauncherWrapper("srpc_test_nonbrowser.nexe");
console.log(launcher.start());
console.log(launcher.setupAppChannel());
console.log(launcher.getServices(NaClLauncherWrapper.CHANNEL_COMMAND));
console.log(launcher.getServices(NaClLauncherWrapper.CHANNEL_APP));
console.log(launcher.setupReverseService());


var launcher = new NaClLauncherWrapper("types_srpc_test.nexe");
console.log(launcher.start());
console.log(launcher.getServices(NaClLauncherWrapper.CHANNEL_COMMAND));
console.log(launcher.setupReverseService());
