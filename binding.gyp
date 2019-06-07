{
    "targets": [
    {
        'target_name': 'sword',
        'type': 'shared_library',
		"conditions":[
			["OS=='mac'", {
                'actions': [
                    {
                        'action_name': 'build_sword',
                        'message': 'Building sword library...',
                        'inputs': [],
                        'outputs': ['sword_build/libsword.dylib'],
                        'action': ['eval', 'svn co http://crosswire.org/svn/sword/trunk sword && sed -i -e "s/1.8.900/1.8.0.0/g" sword/CMakeLists.txt && mkdir -p sword_build && cd sword_build && cmake ../sword && make sword'],
                    },
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
					"<!@(pkg-config --cflags-only-I sword | sed s/-I//g)"
				],
				"libraries": [
					'<!@(pkg-config --libs sword)',
					'<!@(pkg-config --libs libcurl)'
				]
			}],
			["OS=='mac'", {
			    'include_dirs': [
					"<!@(node -p \"require('node-addon-api').include\")",
					"sword/include"
				],
				"libraries": [
					'<(module_root_dir)/sword_build/libsword.dylib',
					'<!@(pkg-config --libs libcurl)'
				],
				"dependencies": [
					 "<!(node -p \"require('node-addon-api').gyp\")",
					 'sword'
				 ],
                "link_settings": {
                    "libraries": [
                        "-Wl,-rpath,@loader_path<(module_root_dir)/sword_build/"
                    ],
                },
			}],
			["OS=='win'", {
			    'include_dirs': [
					"<!@(node -p \"require('node-addon-api').include\")",
					"F:\dev\sword-dep\sword\include"
				],
				"libraries": [
                    '-lF:/dev/sword-libs/libsword.lib',
					'-lF:/dev/sword-libs/icuuc.lib',
					'-lF:/dev/sword-libs/icuin.lib',
					'-lF:/dev/sword-libs/icudt.lib',
					'-lF:/dev/sword-libs/libbz2.lib',
					'-lF:/dev/sword-libs/liblzma.lib',
					'-lF:/dev/sword-libs/libcurl_imp.lib',
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
