const blockchainAPI = new testAddon.BlockChainWrapper();
sha2.genesis({ difficulty: 3, prev_hash: 'de04d58dc5ccc4b9671c3627fb8d626fe4a15810bc1fe3e724feea761965fb71', data: 'first block' }).then(result => {
    console.log(JSON.parse(result));
    runRest();
});
function runRest() {
    sha2.addBlock({ data: "second block" }).then(result => {
        console.log(JSON.parse(result));
    });
    sha2.addBlock({ data: "third block" }).then(result => {
        console.log(JSON.parse(result));
    });
    sha2.addBlock({ data: "fourth block" }).then(result => {
        console.log(JSON.parse(result));
    });
}
