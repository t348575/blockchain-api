const blockchainAPI = new (require('./build/Release/blockchainNativeApi.node')).BlockChainWrapper();
blockchainAPI.genesis({ difficulty: 3, prev_hash: 'de04d58dc5ccc4b9671c3627fb8d626fe4a15810bc1fe3e724feea761965fb71', data: 'first block' }).then(result => {
    console.log(JSON.parse(result));
    runRest();
});
function runRest() {
    blockchainAPI.addBlock({ data: "second block" }).then(result => {
        console.log(JSON.parse(result));
    });
    blockchainAPI.addBlock({ data: "third block" }).then(result => {
        console.log(JSON.parse(result));
    });
    blockchainAPI.addBlock({ data: "fourth block" }).then(result => {
        console.log(JSON.parse(result));
    });
}
