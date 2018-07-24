{
    'targets': [
        {
            'target_name': 'kwfilter',
            'include_dirs': [
                "<!(node -e \"require('nan')\")"
            ],
            'sources': [
                'src/KeywordFilter.cpp',
                'src/KeywordFilterCore.cpp'
            ],
            'defines': ['NDEBUG'],
            'conditions': [
                [ 'OS=="win"', {
		    'msbuild_toolset': 'v140_xp',
                    'defines': [
                        '_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS'
                    ]
                }],
                ['OS=="mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-O2', '-funroll-loops', '-stdlib=libc++ -std=c++11']
                    }
                }, {
                    'cflags': [ '-O2', '-funroll-loops','-std=c++0x', '-Wno-deprecated' ]
                }]
            ]
        }
    ]
}
