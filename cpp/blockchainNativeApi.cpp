#include <napi.h>
#include "blockchainWrapper.h"
#include "blockchain.h"
// InitAll called when addon is included
// BlockChainWrapper::Init wraps the C++ class around the JS class

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return BlockChainWrapper::Init(env, exports);
}
NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll);