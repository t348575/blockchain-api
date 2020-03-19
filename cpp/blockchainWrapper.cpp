#include <napi.h>
#include <iostream>
#include "blockchainWrapper.h"
#include "asyncFunctions.h"
Napi::FunctionReference BlockChainWrapper::constructor;
Napi::Object BlockChainWrapper::Init(Napi::Env env, Napi::Object exports) {
    // create wrapper JS class with two member functions: genesis and addBlock
  	Napi::HandleScope scope(env);
  	Napi::Function func = DefineClass(env, "BlockChainWrapper", {
		InstanceMethod("genesis", &BlockChainWrapper::genesis),
        InstanceMethod("addBlock", &BlockChainWrapper::add_block)
  	});
  	constructor = Napi::Persistent(func);
  	constructor.SuppressDestruct();
  	exports.Set("BlockChainWrapper", func);
  	return exports;
}    
BlockChainWrapper::BlockChainWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BlockChainWrapper>(info) {};
Napi::Value BlockChainWrapper::genesis(const Napi::CallbackInfo& info) {
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
    if(genesis_set) {
        deferred.Reject(Napi::String::New(deferred.Env(), "genesis_already_set"));
    } else {
        // get passed object paramaters and queue AsyncFunctions
        // TODO: check property existence
        Napi::Object input_obj = info[0].As<Napi::Object>();
        Napi::Number d = input_obj.Get("difficulty").As<Napi::Number>();
        Napi::String da = input_obj.Get("data").As<Napi::String>();
        Napi::String ph = input_obj.Get("prev_hash").As<Napi::String>();
        std::string prev_hash = ph, data = da;
        int difficulty = d;
        BlockChainTypes::Block working_block{ 0, difficulty, "", prev_hash, "", data, "" };
        AsyncFunctions * a = new AsyncFunctions(deferred, blockchain_obj, working_block, 0, data);
        a->Queue();
        genesis_set = true;
    }
    return deferred.Promise();
}
Napi::Value BlockChainWrapper::add_block(const Napi::CallbackInfo& info) {
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());
    if(genesis_set) {
        // get passed object paramaters and queue AsyncFunctions
        Napi::Object input_obj = info[0].As<Napi::Object>();
        Napi::String da = input_obj.Get("data").As<Napi::String>();
        std::string data = da;
        BlockChainTypes::Block working_block{ 0, 0, "", "", "", data, "" };
        AsyncFunctions * a = new AsyncFunctions(deferred, blockchain_obj, working_block, 1, data);
        a->Queue();
    } else {
        deferred.Reject(Napi::String::New(deferred.Env(), "genesis_not_set"));
    }
    return deferred.Promise();
}