#ifndef AsyncFunctions_H
#define AsyncFunctions_H
#include <napi.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "promiseWorker.h"
#include "blockchain.h"
namespace {
    std::condition_variable async_functions;
    std::mutex async_functions_mutex;
};
class AsyncFunctions : public PromiseWorker {
    public:
        AsyncFunctions(Napi::Promise::Deferred const &d, std::shared_ptr<BlockChain> blockchain_obj, BlockChainTypes::Block& resource, int mode, std::string name) : PromiseWorker(d), resource{std::move(resource)}, mode(mode), blockchain_obj(blockchain_obj), name(name) {};
        void Execute() {
            switch(this->mode) {
                case 0:
                case 1:
                case 2: {
                    resource.hash = "check_queue";
                    blockchain_obj->set_condition_variable(&async_functions);
                    blockchain_obj->add_block(&this->resource, this->get_flag(), BlockChainTypes::_empty_queue);
                    std::string temp_result = blockchain_obj->get_last_block<std::string>(resource.hash);
                    if(temp_result == "check_queue") {
                        std::unique_lock<std::mutex> guard(async_functions_mutex);
                        async_functions.wait(guard, [this](){ return !(resource.hash == "check_queue"); });
                    }
                    this->result = blockchain_obj->get_last_block<std::string>(resource.hash);
                    break;
                }
                case 3: {
                    this->result = blockchain_obj->print_chain(BlockChainTypes::_should_not_print_to_cout);
                    break;
                }
            }
        };
        void Resolve(Napi::Promise::Deferred const &deferred) {
	        deferred.Resolve(Napi::String::New(deferred.Env(), this->result));
        };
        void Reject(Napi::Promise::Deferred const &deferred) {
            deferred.Reject(Napi::String::New(deferred.Env(), this->reject_reason));
        }
    private:
        BlockChainTypes::Block resource;
        std::string result, name, reject_reason;
        int mode;
        std::shared_ptr<BlockChain> blockchain_obj;
        BlockChainTypes::flags get_flag() {
        	if(this->mode == 0) return BlockChainTypes::_add_block_genesis;
	        if(this->mode == 1) return BlockChainTypes::_add_block_regular;
	        if(this->mode == 2) return BlockChainTypes::_add_block_raw;
	        return BlockChainTypes::_improper_flag;
        };
};
#endif
