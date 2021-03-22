const YZDataRequestFactory = artifacts.require("YZDataRequestFactory");
const SafeMath = artifacts.require("SafeMath");
const ECDSA = artifacts.require("ECDSA");

async function performMigration(deployer) {
  await deployer.deploy(SafeMath);
  await deployer.deploy(ECDSA);
  await SafeMath.deployed();
  await deployer.link(SafeMath, YZDataRequestFactory);
  await deployer.link(ECDSA, YZDataRequestFactory);
  await deployer.deploy(YZDataRequestFactory);
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
