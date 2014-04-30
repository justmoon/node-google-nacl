{
  'variables': {
    'NACL_ROOT%': '../nacl_root/'
  },
  'targets': [
    {
      'target_name': 'sel_ldr_proxy',
      'sources': [ 'src/sel_ldr_proxy.cc' ],
      'libraries': [
        '-L../<(NACL_ROOT)/native_client/scons-out/opt-linux-x86-32/lib/',
        '-lnonnacl_util',
        '-lnrd_xfer',
        '-lnacl_base_shared',
        '-lplatform_shared',
        '-limc_shared',
        '-lenv_cleanser_shared',
        '-lgio',
        '-lnonnacl_srpc_shared'
      ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")',
        '<(NACL_ROOT)'
      ]
    }
  ]
}