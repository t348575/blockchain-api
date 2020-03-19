#include <napi.h>
#include <string>
#include <vector>
#include <future>
#include <atomic>
#include "asyncFunctions.h"
namespace BlockChainWrapperTypes {
    enum flags {
        _genesis_ready
    };
};
class BlockChainWrapper : public Napi::ObjectWrap<BlockChainWrapper> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        BlockChainWrapper(const Napi::CallbackInfo& info);
    private:
        bool genesis_set = false;
        // shared_ptr to avoid object destruction
        std::shared_ptr<BlockChain> blockchain_obj = std::make_shared<BlockChain>();
        static Napi::FunctionReference constructor;
        Napi::Value genesis(const Napi::CallbackInfo& info);
        Napi::Value add_block(const Napi::CallbackInfo& info);
};