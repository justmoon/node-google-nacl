{
  'variables': {
    'NACL_ROOT%': '/var/www/nacl_root/'
  },
  'targets': [
    {
      'target_name': 'sel_ldr_proxy',
      'sources': [
        'src/sel_ldr_proxy.cc',
        'src/nacl_launcher_wrapper.cc',
        'src/reverse_emulate.cc'
      ],
      'libraries': [
        '-L<(NACL_ROOT)/native_client/scons-out/opt-linux-x86-32/lib/',
        '-lnonnacl_util',
        '-lnrd_xfer',
        '-lnacl_base_shared',
        '-limc_shared',
        '-lenv_cleanser_shared',
        '-lreverse_service_shared',
        '-lsimple_service_shared',
        '-lthread_interface_shared',
        '-lplatform_shared',
        '-lgio',
        '-lnonnacl_srpc_shared'
      ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")',
        '<(NACL_ROOT)'
      ],
      
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            'NACL_LINUX=1'
          ]
        }]
      ],
    }
  ]
}