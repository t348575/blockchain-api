let { BlockChainAPI } = require('./BlockChainAPI');
blockchainAPI = new BlockChainAPI({'dsa':'dsa'});
blockchainAPI.addBlock({ difficulty: 3, prev_hash: 'de04d58dc5ccc4b9671c3627fb8d626fe4a15810bc1fe3e724feea761965fb71', data: 'first block' }, new Date().getTime()).then(res => {
    console.log('Result: ', JSON.parse(res));
    next();
}).catch(err => {
    console.log('Error: ', err);
});
function next() {
    blockchainAPI.addBlock({ data: 'second block'}).then(res => {
        console.log('Result: ', JSON.parse(res));
    }).catch(err => {
        console.log('Error: ', err);
    });
    blockchainAPI.addBlock({ data: 'third block'}).then(res => {
        console.log('Result: ', JSON.parse(res));
    }).catch(err => {
        console.log('Error: ', err);
    });
    blockchainAPI.addBlock({ data: 'fourth block'}).then(res => {
        console.log('Result: ', JSON.parse(res));
    }).catch(err => {
        console.log('Error: ', err);
    });
}
