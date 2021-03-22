const YZDataFactory = artifacts.require("YZDataFactory");
const CertifiedUsers = artifacts.require("CertifiedUsers");
const YZDataRequestFactory = artifacts.require("YZDataRequestFactory");

async function performMigration(deployer, network, accounts) {
  b = await CertifiedUsers.deployed();
  f = await YZDataRequestFactory.deployed();
  await deployer.deploy(YZDataFactory, b.address, f.address);
}

module.exports = function(deployer, network, accounts){
deployer
    .then(function() {
      return performMigration(deployer, network, accounts)
    })
    .catch(error => {
      console.log(error)
      process.exit(1)
    })
};
