# blockchain-api
  Blockchain API based on [BasicBlockChain](https://github.com/t348575/BasicBlockChain) using node-addon-api to run multithreaded native code for faster hash computation and execution
  
  
  ## Installation
    git clone https://github.com/t348575/blockchain-api.git
    npm install
    npm run build
    
    
  ## Testing
  ```
  npm run test
  ```
  Possible output:
  ```
  Result:  {
    data: 'first block',
    difficulty: 3,
    hash: '000c04a65a1fdb46d0bb962157d14fb5b4d516badcff7b6582b618b6eb5752d4',
    index: 1,
    nonce: '2468',
    prevhash: 'de04d58dc5ccc4b9671c3627fb8d626fe4a15810bc1fe3e724feea761965fb71',
    timestamp: '1584626538606'
}
Result:  {
    data: 'second block',
    difficulty: 3,
    hash: '0004d9415232b7856ac247f08d4dd5fa5e5883244624bd9690729b096bb7cf2c',
    index: 2,
    nonce: '1f8f',
    prevhash: '000c04a65a1fdb46d0bb962157d14fb5b4d516badcff7b6582b618b6eb5752d4',
    timestamp: '1584626538651'
}
Result:  {
    data: 'third block',
    difficulty: 3,
    hash: '000ff67be8f758a80f5fa9d0350db3080664065279c1f6236b8466ce35f28457',
    index: 3,
    nonce: '1050',
    prevhash: '0004d9415232b7856ac247f08d4dd5fa5e5883244624bd9690729b096bb7cf2c',
    timestamp: '1584626539605'
}
Result:  {
    data: 'fourth block',
    difficulty: 3,
    hash: '0007a6c07ff688e2ccacfaff31c8509a3284bda48456860bf3db5c0f27da2a41',
    index: 4,
    nonce: '524f',
    prevhash: '000ff67be8f758a80f5fa9d0350db3080664065279c1f6236b8466ce35f28457',
    timestamp: '1584626540608'
}
  ```
  
  ## Todo:
 - [ ] Add functions for verification of chain and block
 - [ ] Use web worker threads were possible
 - [ ] Add support for other hash functions
