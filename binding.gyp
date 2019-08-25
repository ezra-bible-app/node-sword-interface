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
                        'action': ['./build_sword.sh'],
                    }
                ]
            }],
            [ "OS == 'win'", {
                'actions': [
                    {
                        'action_name': 'get_sword_win32',
                        'message': 'Cloning Win32 sword library from GitHub ...',
                        'inputs': [],
                        'outputs': ['sword-win32'],
                        'action': ['call <(module_root_dir)\get_sword_win32.bat'],
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
            "file_system_helper.cpp",
            "sword_facade.cpp",
            "node_sword_interface.cpp",
            "binding.cpp"
        ],
        "conditions":[
            ["OS=='linux'", {
                'include_dirs': [
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "sword/include"
                ],
                "libraries": [
                    '<(module_root_dir)/sword_build/libsword.a',
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
                    "sword/include"
                ],
                "libraries": [
                    '<(module_root_dir)/sword_build/libsword.a',
                    '<!@(pkg-config --libs libcurl)'
                ],
                "dependencies": [
                    "<!(node -p \"require('node-addon-api').gyp\")",
                    'sword'
                ]
            }],
            ["OS=='win'", {
                'include_dirs': [
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<(module_root_dir)/build/sword-win32/include"
                ],
                "libraries": [
                    '-l<(module_root_dir)/build/sword-win32/lib/libsword.lib',
                    '-l<(module_root_dir)/build/sword-win32/lib/icuuc.lib',
                    '-l<(module_root_dir)/build/sword-win32/lib/icuin.lib',
                    '-l<(module_root_dir)/build/sword-win32/lib/icudt.lib',
                    '-l<(module_root_dir)/build/sword-win32/lib/libbz2.lib',
                    '-l<(module_root_dir)/build/sword-win32/lib/liblzma.lib',
                    '-l<(module_root_dir)/build/sword-win32/lib/libcurl_imp.lib',
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
