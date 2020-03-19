#include <iostream>
#include <chrono>
#include <future>
#include <thread>
#include <sstream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "json.h"
#include "sha256.h"
#include "blockchain.h"
// local namespace globals
namespace {
	// mutex's condition variable and atomics for threads
	std::mutex hash_calculator_thread_mutex, is_ready_mutex, add_block_mutex, chain_mutex;
	std::condition_variable wait_point_hash_calculator;
	std::atomic<bool> running(false), skip(false);
	// queue for running hash calculations (to avoid launching way too many threads)
	std::queue<std::pair<BlockChainTypes::Block*, BlockChainTypes::flags>> block_queue;
	// copy functions to deep copy referenced variables
	BlockChainTypes::Block copy_block(BlockChainTypes::Block block) { return BlockChainTypes::Block { block.index, block.difficulty, block.hash, block.prev_hash, block.nonce, block.data, block.timestamp }; }
	std::pair<BlockChainTypes::Block, BlockChainTypes::flags> copy_block_queue_ele(std::pair<BlockChainTypes::Block, BlockChainTypes::flags> ele) { return std::make_pair(copy_block(ele.first), ele.second); }
};
// constructors
BlockChain::BlockChain() {
	// queue monitoring and job invoking thread
	std::thread(&BlockChain::monitor_and_run_queue, this).detach();
}
BlockChain::BlockChain(BlockChainTypes::Block& genesis, BlockChainTypes::flags mode) {
	std::thread(&BlockChain::monitor_and_run_queue, this).detach();
    if(mode == BlockChainTypes::_create_genesis_block) {
        // TODO: add genesis block creator
        difficulty = genesis.difficulty;
	    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	    genesis.timestamp = std::to_string(ms.count());
	    this->add_block(&genesis, BlockChainTypes::_add_block_genesis, BlockChainTypes::_empty_queue);
		this->print_block(this->chain.back(), BlockChainTypes::_should_not_print_to_cout);
    } else {
        this->add_block(&genesis, BlockChainTypes::_add_block_raw, BlockChainTypes::_empty_queue);
		chain_mutex.lock();
	    this->print_block(this->chain.back(), BlockChainTypes::_should_not_print_to_cout);
		chain_mutex.unlock();
    }
}
// print a given block as json string
std::string BlockChain::print_block(BlockChainTypes::Block& curr_block, BlockChainTypes::flags print_location) {
	nlohmann::json result;
	std::stringstream chain_as_string;
	result["index"] = curr_block.index;
	result["data"] = curr_block.data;
	result["difficulty"] = curr_block.difficulty;
	result["nonce"] = curr_block.nonce;
	result["timestamp"] = curr_block.timestamp;
	result["hash"] = curr_block.hash;
	result["prevhash"] = curr_block.prev_hash;
	chain_as_string << result.dump() << std::endl;
	if(print_location == BlockChainTypes::_should_print_to_cout) {
		std::cout << chain_as_string.str() << std::endl;
	}
	return chain_as_string.str();
}
// print entire chain as json string
std::string BlockChain::print_chain(BlockChainTypes::flags print_location) {
	std::stringstream chain_as_string;
	for(int i=0; i<this->chain.size(); i++) {
		nlohmann::json result;
		result["index"] = this->chain[i].index;
		result["data"] = this->chain[i].data;
		result["difficulty"] = this->chain[i].difficulty;
		result["nonce"] = this->chain[i].nonce;
		result["timestamp"] = this->chain[i].timestamp;
		result["hash"] = this->chain[i].hash;
		result["prevhash"] = this->chain[i].prev_hash;
		chain_as_string << result.dump() << std::endl;
	}
	if(print_location == BlockChainTypes::_should_print_to_cout) {
		std::cout << chain_as_string.str() << std::endl;
	}
	return chain_as_string.str();
}
// get last block as BlockChain::Block or std::string
template<typename T>
T BlockChain::get_last_block() {
	std::lock_guard<std::mutex> guard(chain_mutex);
	if(!this->chain.empty()) {
		return this->chain.back();
	}
	return "empty_chain";
}
template<>
std::string BlockChain::get_last_block<std::string>() {
	std::lock_guard<std::mutex> guard(chain_mutex);
	if(!this->chain.empty()) {
		return this->print_block(this->chain.back(), BlockChainTypes::_should_not_print_to_cout);
	}
	return "empty_chain";
}
// get block with given hash as BlockChain::Block or std::string
template<typename T>
T BlockChain::get_last_block(std::string hash) {
	std::lock_guard<std::mutex> guard(chain_mutex);
	if(!this->chain.empty()) {
		for(int i=chain.size() - 1; i >= 0; i--) {
			if(chain[i].hash == hash) {
				return chain[i];
			}
		}
	}
	return "empty_chain";
}
template<>
std::string BlockChain::get_last_block<std::string>(std::string hash) {
	std::lock_guard<std::mutex> guard(chain_mutex);
	if(!this->chain.empty()) {
		for(int i=chain.size() - 1; i >= 0; i--) {
			if(chain[i].hash == hash) {
				return this->print_block(chain[i], BlockChainTypes::_should_not_print_to_cout);
			}
		}
	}
	return "check_queue";
}
// starts_with and is_hash_valid to check if difficulty is met
bool BlockChain::starts_with(std::string main_str, std::string to_match) {
	if (main_str.find(to_match) == 0)
		return true;
	else
		return false;
}
bool BlockChain::is_hash_valid(std::string hash, int difficulty) {
	std::string prefix = "";
	while (difficulty--)
		prefix += "0";
	return this->starts_with(hash, prefix);
}
// calculate sha256 hash (double hash)
std::string BlockChain::calc_hash(std::string& hash_string) {
	SHA256 sha256;
	hash_string = sha256(hash_string);
	sha256.reset();
	return sha256(hash_string);
}
// utility for condition variable 
bool BlockChain::is_ready() {
	std::lock_guard<std::mutex> guard_hash_found(is_ready_mutex);
	return this->hash_found;
}
void BlockChain::set_found(bool val) {
	std::lock_guard<std::mutex> guard_hash_found(is_ready_mutex);
	this->hash_found = val;
}
// calculate hash's within nonce range until given difficulty is found
void BlockChain::hash_calculator_thread(BlockChainTypes::Block block_to_hash, std::pair<int, int> range) {
    while(!this->is_ready()) {
		int count = 0;
		for (int i = range.first; i <= range.second; i++, count++) {
			std::stringstream temp;
			temp << std::hex << i;
			block_to_hash.nonce = temp.str();
			std::string data_to_hash = std::to_string(block_to_hash.index) + block_to_hash.timestamp + block_to_hash.data + block_to_hash.prev_hash + block_to_hash.nonce;
			std::string hash = this->calc_hash(data_to_hash);
			if(!is_hash_valid(hash, this->difficulty)) {
				if(count == this->is_hash_complete_interval && this->is_ready())
					return;
				continue;
			} else {
				if(!is_ready()) {
					this->set_found(true);
					std::lock_guard<std::mutex> guard_calculated_hash(hash_calculator_thread_mutex);
					this->temp_hash = hash;
					this->temp_nonce = block_to_hash.nonce;
					wait_point_hash_calculator.notify_one();
					return;
				}
				return;
			}
		}
		range.first += this->thread_nonce_range;
		range.second += this->thread_nonce_range;
	}
}
bool BlockChain::is_block_valid(BlockChainTypes::Block new_block, BlockChainTypes::Block old_block) {
	if (old_block.index + 1 != new_block.index)
		return false;
	if (old_block.hash != new_block.prev_hash)
		return false;
	std::string data_to_hash = std::to_string(new_block.index) + new_block.timestamp + new_block.data + new_block.prev_hash + new_block.nonce;
	if (this->calc_hash(data_to_hash) != new_block.hash)
		return false;
	return true;
}
void BlockChain::add_block(BlockChainTypes::Block* curr_block, BlockChainTypes::flags mode, BlockChainTypes::flags location) {
	// if a hash is currently being calculated, add current job to queue
	if(!running.load() || skip.load()) {
		running = true;
		BlockChainTypes::Block new_block;
		if(mode == BlockChainTypes::_add_block_raw) {
			// TODO: verify the new block before adding
			this->chain.push_back(*curr_block);
		} else if(mode == BlockChainTypes::_add_block_regular) {
			chain_mutex.lock();
			this->difficulty = this->chain.back().difficulty;
			new_block.prev_hash = chain.back().hash;
			new_block.index = chain.back().index + 1;
			chain_mutex.unlock();
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			new_block.timestamp = std::to_string(ms.count());
			new_block.data = (*curr_block).data;
			new_block.difficulty = this->difficulty;
			std::vector<std::future<void>> hash_calculator_thread_pool;
			int conc = this->concurrency;
			if(difficulty < 3)
				conc = 1;
			for (int i = 0; i < conc; i++) {
				hash_calculator_thread_pool.emplace_back(std::async(&BlockChain::hash_calculator_thread, this, new_block, std::make_pair(i * this->thread_nonce_range, (i + 1) * this->thread_nonce_range)));
			}
			for (auto&i : hash_calculator_thread_pool) {
				i.get();
			}
			std::unique_lock<std::mutex> wait_hash_calculator_thread(add_block_mutex);
			wait_point_hash_calculator.wait(wait_hash_calculator_thread, std::bind(&BlockChain::is_ready, this));
			this->set_found(false);
			new_block.hash = this->temp_hash;
			new_block.nonce = this->temp_nonce;
			if (this->is_block_valid(new_block, chain.back())) {
				chain_mutex.lock();
				chain.push_back(new_block);
				chain_mutex.unlock();
				(*curr_block).hash = new_block.hash;
				(*curr_block).nonce = new_block.nonce;
				print_block(new_block, BlockChainTypes::_should_not_print_to_cout);
			}
		} else if(mode == BlockChainTypes::_add_block_genesis) {
			this->difficulty = (*curr_block).difficulty;
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			new_block.index = 1;
			new_block.timestamp = std::to_string(ms.count());
			new_block.data = (*curr_block).data;
			new_block.prev_hash = (*curr_block).prev_hash;
			new_block.difficulty = this->difficulty;
			std::vector<std::future<void>> hash_calculator_thread_pool;
			int conc = this->concurrency;
			if(difficulty < 3)
				conc = 1;
			for (int i = 0; i < conc; i++) {
				hash_calculator_thread_pool.emplace_back(std::async(&BlockChain::hash_calculator_thread, this, new_block, std::make_pair(i * this->thread_nonce_range, (i + 1) * this->thread_nonce_range)));
			}
			for (auto&i : hash_calculator_thread_pool) {
				i.get();
			}
			std::unique_lock<std::mutex> wait_hash_calculator_thread(add_block_mutex);
			wait_point_hash_calculator.wait(wait_hash_calculator_thread, std::bind(&BlockChain::is_ready, this));
			this->set_found(false);			
			new_block.hash = this->temp_hash;
			new_block.nonce = this->temp_nonce;
			chain_mutex.lock();
			chain.push_back(new_block);
			chain_mutex.unlock();
			(*curr_block).hash = new_block.hash;
			(*curr_block).nonce = new_block.nonce;
		}
		if(location == BlockChainTypes::_from_queue) {
			(*async_functions).notify_one();
		}
		running = false;
		skip = false;
	} else {
		// add job to queue
		chain_mutex.lock();
		block_queue.push(std::make_pair(curr_block, std::move(mode)));
		chain_mutex.unlock();
	}
}
void BlockChain::monitor_and_run_queue() {
	while(true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(this->monitor_queue_sleep_interval));
		// run job if hash isn't being calculated
		if(!block_queue.empty() && !running.load()) {
			skip = true;
			this->add_block(block_queue.front().first, block_queue.front().second, BlockChainTypes::_from_queue);
			block_queue.pop();
		}
	}
}
void BlockChain::set_condition_variable(std::condition_variable* var) {
	this->async_functions = var;
}