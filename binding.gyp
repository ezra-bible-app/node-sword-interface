{
    "targets": [{
        "target_name": "ezra_sword_interface",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions -std=c++11 -pthread" ],
        "sources": [
            "ezra_sword_interface.cpp",
            "js_ezra_sword_interface.cpp",
            "binding.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "/usr/include/sword"
        ],
        'libraries': [ 
            '-lsword',
            '/usr/lib/x86_64-linux-gnu/libcurl.so.4'
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }]
}
