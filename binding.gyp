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
		"conditions":[
			["OS=='linux'", {
			    'include_dirs': [
					"<!@(node -p \"require('node-addon-api').include\")",
					"/usr/include/sword"
				],
				"libraries": [
					'-lsword',
					'/usr/lib/x86_64-linux-gnu/libcurl.so.4'
				]
			}],
			["OS=='win'", {
			    'include_dirs': [
					"<!@(node -p \"require('node-addon-api').include\")",
					"F:\dev\sword-dep\sword\include"
				],
				"libraries": [
					'-lF:\dev\ezra-sword-interface-git-deps\libsword.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\icuuc.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\icuin.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\icudt.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\libbz2.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\zlib.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\liblzma.lib',
					'-lF:\dev\ezra-sword-interface-git-deps\libcurl.lib',
					'-lWldap32.lib',
					'-lWs2_32.lib',
					'-lcrypt32.lib',
					'-lNormaliz.lib'
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
