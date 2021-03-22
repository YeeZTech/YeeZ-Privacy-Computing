const PrivateKeyProvider = require("truffle-privatekey-provider");

require('dotenv').config()

module.exports = {

  networks: {

    // Useful for deploying to a public network.
    // NB: It's important to wrap the provider as a function.
     ropsten: {
       //provider: new HDWalletProvider(process.env.MNENOMIC, "https://ropsten.infura.io/v3/" + process.env.INFURA_API_KEY),
       provider: new PrivateKeyProvider(process.env.ROPSTEN_PK, "https://ropsten.infura.io/v3/" + process.env.INFURA_API_KEY),
       //provider : createKeystoreProvider(process.env.ACCOUNT, process.env.DATA_DIR, "https://ropsten.infura.io/v3/" + process.env.INFURA_API_KEY),
       network_id: 3,       // Ropsten's id
       gas: 7000000,        // Ropsten has a lower block limit than mainnet
       //confirmations: 3,    // # of confs to wait between deployments. (default: 0)
       timeoutBlocks: 200,  // # of blocks before a deployment times out  (minimum/default: 50)
       skipDryRun: true,     // Skip dry run before migrations? (default: false for public nets )
      gasPrice: 50000000000
     },

  },

  plugins: [
        'truffle-plugin-verify'
  ],

  api_keys: {
      etherscan: process.env.ETHERSCAN_API_KEY
  },

  // Set default mocha options here, use special reporters etc.
  mocha: {
    // timeout: 100000
  },

  // Configure your compilers
  compilers: {
    solc: {
       version: "0.5.10",    // Fetch exact version from solc-bin (default: truffle's version)
      // docker: true,        // Use "0.5.1" you've installed locally with docker (default: false)
       settings: {          // See the solidity docs for advice about optimization and evmVersion
        optimizer: {
          enabled: true,
          runs: 200
        },
      //  evmVersion: "byzantium"
       }
    }
  }
}
