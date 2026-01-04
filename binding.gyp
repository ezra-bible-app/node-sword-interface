{
    "variables": {
      # 1. Detect if we are on iOS
      "is_ios%": "<!(python3 -c \"import os; sdk = os.environ.get('SDKROOT', ''); print(1 if 'iPhone' in sdk or os.environ.get('PLATFORM') == 'ios' else 0)\")",
      
      # 2. Get the deployment target (e.g., '15.0') from env, default to '13.0'
      "ios_ver%": "<!(python3 -c \"import os; print(os.environ.get('IPHONEOS_DEPLOYMENT_TARGET', '13.0'))\")",

      # 3. Detect the SDK to use (iphoneos or iphonesimulator)
      "ios_sdk": "<!(python3 -c \"import os; print('iphonesimulator' if 'iPhoneSimulator' in os.environ.get('SDKROOT', '') else 'iphoneos')\")"
    },
    "targets": [
    {
        'target_name': 'sword',
        'type': 'none',
        'conditions': [
            ["is_ios==1", {
                'actions': [
                    {
                          'action_name': 'build_sword',
                          'message': 'Building sword library for iOS...',
                          'inputs': [],
                          'outputs': ['sword_build/libsword.a'],
                          'action': ['./scripts/build_sword.sh', '--ios', '${PLATFORM_NAME}', '<(ios_ver)', '${ARCHS}'],
                    }
                ]
            }],
            [ "OS == 'mac'", {
                'actions': [
                    {
                        'action_name': 'build_sword',
                        'message': 'Building sword library for macOS ...',
                        'inputs': [],
                        'outputs': ['sword_build/libsword.a'],
                        'action': ['./scripts/build_sword.sh'],
                    }
                ]
            }],
            [ "OS == 'linux'", {
                'actions': [
                    {
                        'action_name': 'build_sword',
                        'message': 'Building sword library for Linux...',
                        'inputs': [],
                        'outputs': ['sword_build/libsword.a'],
                        'action': ['./scripts/build_sword.sh'],
                    }
                ]
            }],
            [ "OS == 'android'", {
                'actions': [
                    {
                        'action_name': 'build_sword',
                        'message': 'Building sword library for Android...',
                        'inputs': [],
                        'outputs': ['sword_build/libsword.a'],
                        'action': ['./scripts/build_sword.sh', '--android', '<(target_arch)'],
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
                        'action': ['node', '<(module_root_dir)/scripts/get_sword_build_win32.js'],
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
            "src/sword_backend/mutex.cpp",
            "src/sword_backend/file_system_helper.cpp",
            "src/sword_backend/module_helper.cpp",
            "src/sword_backend/dict_helper.cpp",
            "src/sword_backend/module_store.cpp",
            "src/sword_backend/string_helper.cpp",
            "src/sword_backend/strongs_entry.cpp",
            "src/sword_backend/repository_interface.cpp",
            "src/sword_backend/module_search.cpp",
            "src/sword_backend/module_installer.cpp",
            "src/sword_backend/sword_status_reporter.cpp",
            "src/sword_backend/text_processor.cpp",
            "src/sword_backend/unzip/unzip.c",
            "src/sword_backend/unzip/ioapi.c",
            "src/napi_module/install_module_worker.cpp",
            "src/napi_module/module_search_worker.cpp",
            "src/napi_module/napi_sword_helper.cpp",
            "src/napi_module/node_sword_interface.cpp",
            "src/napi_module/api_lock.cpp",
            "src/napi_module/binding.cpp"
        ],
        "conditions":[
            ["is_ios==1", {
                "type": "shared_library",
                'include_dirs': [
                    "<(module_root_dir)/src/sword_backend",
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<!@(./scripts/get_sword_include_path.sh)"
                ],
                "libraries": [
                    '<!@(./scripts/get_sword_library.sh "../sword_build/libsword.a")',
                    '-lbz2',
                    '-lz'
                ],
                "dependencies": [
                    'sword'
                ],
                "xcode_settings": {
                  "SDKROOT": "<(ios_sdk)",
                  "IPHONEOS_DEPLOYMENT_TARGET": "<(ios_ver)",
                  "ONLY_ACTIVE_ARCH": "YES",
                  "OTHER_LDFLAGS": [
                    "-dynamiclib",
                    "-install_name @rpath/node_sword_interface.framework/node_sword_interface",
                    "-undefined dynamic_lookup",
                    "-Wl,-bind_at_load",
                    "-all_load"
                  ],
                  "MACH_O_TYPE": "mh_dylib",
                  "PRODUCT_TYPE": "com.apple.product-type.framework"
                }
            }],
            ["OS=='linux'", {
                'include_dirs': [
                    "<(module_root_dir)/src/sword_backend",
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<!@(./scripts/get_sword_include_path.sh)"
                ],
                "libraries": [
                    '<!@(./scripts/get_sword_library.sh \"../sword_build/libsword.a\")',
                    '<!@(pkg-config --libs libcurl)'
                ],
                "dependencies": [
                    "<!(node -p \"require('node-addon-api').gyp\")",
                    'sword'
                ]
            }],
            ["OS=='android'", {
                'include_dirs': [
                    "<(module_root_dir)/src/sword_backend",
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<!@(./scripts/get_sword_include_path.sh)"
                ],
                "libraries": [
                    '<!@(./scripts/get_sword_library.sh \"../sword_build/libsword.a\")'
                ],
                "dependencies": [
                    "<!(node -p \"require('node-addon-api').gyp\")",
                    'sword'
                ],
                'ldflags': [
                    '-Wl,-z,max-page-size=16384'
                ]
            }],
            ["OS=='mac'", {
                'include_dirs': [
                    "<(module_root_dir)/src/sword_backend",
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<!@(./scripts/get_sword_include_path.sh)"
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
                    "<(module_root_dir)/src/sword_backend",
                    "<!@(node -p \"require('node-addon-api').include\")",
                    "<(module_root_dir)/build/sword-build-win32/include"
                ],
                "libraries": [
                    '-l<(module_root_dir)/build/sword-build-win32/lib/sword.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/libbz2.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/liblzma.lib',
                    '-l<(module_root_dir)/build/sword-build-win32/lib/libcurl_imp.lib',
                    '-lWldap32.lib',
                    '-lWs2_32.lib',
                    '-lcrypt32.lib',
                    '-lNormaliz.lib'
                ],
                "dependencies": [
                    'sword',
                    "<!(node -p \"require('node-addon-api').gyp\")"
                ],
                'defines': [ 'SWUSINGDLL' ]
            }]
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    },
    {
        "target_name": "node_sword_interface.framework",
        "type": "none",
        "dependencies": [ "node_sword_interface" ],
        "conditions": [
            ["is_ios==1", {
                "actions": [
                    {
                        "action_name": "create_framework",
                        "inputs": [],
                        "outputs": ["<(PRODUCT_DIR)/node_sword_interface.framework"],
                        "action": [
                            "sh",
                            "scripts/ios_framework_postbuild.sh",
                            "<(PRODUCT_DIR)",
                            "node_sword_interface"
                        ]
                    }
                ]
            }]
        ]
    },
    {
        "target_name": "cleanup",
        "type": "none",
        "dependencies": [ "node_sword_interface" ],
        "actions": [
            {
                "action_name": "clean_build_artifacts",
                "inputs": [],
                "outputs": ["<(PRODUCT_DIR)/cleanup.stamp"],
                "action": [ "sh", "scripts/cleanup_build_artifacts.sh", "<(PRODUCT_DIR)/cleanup.stamp" ]
            }
        ]
    }]
}
