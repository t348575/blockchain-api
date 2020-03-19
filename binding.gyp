
{
    "targets": [{
        "target_name": "blockchainNativeApi",
        "cflags!": [ "-fno-exceptions", "-std=c++11", "-lpthread" ],
        "cflags_cc!": [ "-fno-exceptions", "-std=c++11", "-lpthread" ],
        "sources": [
            "cpp/blockchainNativeApi.cpp",
            "cpp/blockchainWrapper.cpp",
            "cpp/blockchain.cpp",
            "cpp/sha256.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
        ],
        'libraries': [],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_CPP_EXCEPTIONS' ]
    }]
}
