const YZDataRequestFactory = artifacts.require("YZDataRequestFactory");
const YZDataRequestForOffChainFactory = artifacts.require("YZDataRequestForOffChainFactory");
const SafeMath = artifacts.require("SafeMath");
const ECDSA = artifacts.require("ECDSA");

async function performMigration(deployer) {
  await deployer.deploy(SafeMath);
  await deployer.deploy(ECDSA);
  await SafeMath.deployed();
  await deployer.link(SafeMath, YZDataRequestFactory);
  await deployer.link(ECDSA, YZDataRequestFactory);
  await deployer.deploy(YZDataRequestFactory);

  await deployer.link(SafeMath, YZDataRequestForOffChainFactory);
  await deployer.link(ECDSA, YZDataRequestForOffChainFactory);
  await deployer.deploy(YZDataRequestForOffChainFactory);
}

module.exports = function(deployer){
deployer
    .then(function() {
      return performMigration(deployer)
    })
    .catch(error => {
      console.log(error)
      process.exit(1)
    })
};
