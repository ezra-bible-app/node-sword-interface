{
    "targets": [
    {
        'target_name': 'sword',
        'type': 'none',
        'conditions': [
            [ "OS != 'win'", {
                'actions': [
                    {
                        'action_name': 'build_sword',
                        'message': 'Building sword library...',
                        'inputs': [],
                        'outputs': ['sword_build/libsword.a'],
                        'action': ['./scripts/build_sword.sh'],
                    }
                ]
            }],
            [ "OS == 'win'", {
                'actions': [
                    {
                        'action_name': 'get_sword_win32',
                        'message': 'Downloading sword-build-win32 artifacts from GitHub ...',
                        'inputs': [],
                        'outputs': ['sword-build-win32'],
                        'action': ['call PowerShell.exe -ExecutionPolicy Bypass -File <(module_root_dir)\scripts\get_sword_build_win32.ps1'],
                    }
                ]
            }]
        ]
    },
    {
        "target_name": "node_sword_interface",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions -std=c++11 -pthread" ],
        "sources": [
            "src/file_system_helper.cpp",
            "src/string_helper.cpp",
            "src/strongs_entry.cpp",
            "src/sword_facade.cpp",
            "src/sword_status_reporter.cpp",
            "src/napi_sword_helper.cpp",
            "src/node_sword_interface.cpp",
            "src/binding.cpp"
        ],
        "conditions":[
            ["OS=='linux'", {
                'include_dirs': [
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<!@(./scripts/get_sword_include_path.sh)"
                ],
                "libraries": [
                    '<!@(./scripts/get_sword_library.sh \"../sword_build/libsword.a\")',
                    '<!@(pkg-config --libs libcurl)',
                    '<!@(pkg-config --libs icu-uc icu-io)'
                ],
                "dependencies": [
                    "<!(node -p \"require('node-addon-api').gyp\")",
                    'sword'
                ]
            }],
            ["OS=='mac'", {
                'include_dirs': [
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<!@(./scripts/get_sword_include_path.sh)"
                ],
                "libraries": [
                    '<(module_root_dir)/sword_build/libsword.a',
                    '<!@(pkg-config --libs libcurl)',
                    '<!@(PKG_CONFIG_PATH=/usr/local/opt/icu4c/lib/pkgconfig pkg-config --libs icu-uc icu-io)'
                ],
                "dependencies": [
                    "<!(node -p \"require('node-addon-api').gyp\")",
                    'sword'
                ]
            }],
            ["OS=='win'", {
                'include_dirs': [
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<(module_root_dir)/build/sword-build-win32/include"
                ],
                "libraries": [
                    '-l<(module_root_dir)/build/sword-build-win32/lib/sword.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/icuuc.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/icuin.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/icudt.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/libbz2.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/liblzma.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/libcurl_imp.lib',
                    '-lWldap32.lib',
                    '-lWs2_32.lib',
                    '-lcrypt32.lib',
                    '-lNormaliz.lib'
                ],
                "dependencies": [
                    'sword'
                ],
                'defines': [ '_HAS_EXCEPTIONS=1' ]
            }]
        ], 
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }]
}
