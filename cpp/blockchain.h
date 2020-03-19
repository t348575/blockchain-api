#ifndef BlockChain_H
#define BlockChain_H
#include <string>
#include <vector>
namespace BlockChainTypes {
    // basic block structure
    struct Block {
    	int index, difficulty = 0;
    	std::string hash, prev_hash, nonce, data, timestamp;
    };
    // enum for flags for events
    enum flags {
        _create_genesis_block,
        _add_block_raw,
        _add_block_regular,
        _add_block_genesis,
        _should_not_print_to_cout,
        _should_print_to_cout,
        _improper_flag,
        _from_queue,
        _empty_queue
    };
}
class BlockChain {
    public:
        BlockChain();
        BlockChain(BlockChainTypes::Block &genesis, BlockChainTypes::flags mode);
        void add_block(BlockChainTypes::Block* curr_block, BlockChainTypes::flags mode, BlockChainTypes::flags location);
        std::string print_block(BlockChainTypes::Block& curr_block, BlockChainTypes::flags print_location);
        std::string print_chain(BlockChainTypes::flags print_location);
        template<typename T>
        T get_last_block();
        template<typename T>
        T get_last_block(std::string hash);
        bool is_block_valid(BlockChainTypes::Block new_block, BlockChainTypes::Block old_block);
        bool is_ready();
        void set_condition_variable(std::condition_variable* var);
    private:
        std::condition_variable * async_functions;
        std::string temp_hash, temp_nonce;
        bool hash_found = false;
        int difficulty, thread_nonce_range = 4000, is_hash_complete_interval = 256, monitor_queue_sleep_interval = 1000, concurrency = std::thread::hardware_concurrency();
        std::vector<BlockChainTypes::Block> chain;
        void hash_calculator_thread(BlockChainTypes::Block block_to_hash, std::pair<int, int> range);
        void set_found(bool val);
        std::string calc_hash(std::string& hash_string);
        bool is_hash_valid(std::string hash, int difficulty);
        bool starts_with(std::string main_str, std::string to_match);
        void monitor_and_run_queue();
};
#endif