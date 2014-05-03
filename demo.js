var binding = require('bindings')('sel_ldr_proxy.node');


console.log(binding.NaClLauncherWrapper);

var launcher = new binding.NaClLauncherWrapper("srpc_test_nonbrowser.nexe");
console.log(launcher.start());
console.log(launcher.getServices());
console.log(launcher.setupReverseService());


var launcher = new binding.NaClLauncherWrapper("types_srpc_test.nexe");
console.log(launcher.start());
console.log(launcher.getServices());
console.log(launcher.setupReverseService());