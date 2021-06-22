const ProgramStore = artifacts.require("ProgramStore");
const TestProgramStore = artifacts.require("TestProgramStore");

async function performMigration(deployer, network, accounts) {
  await deployer.deploy(ProgramStore);

  if(network.includes("development") ||
    network.includes("ganache")
    ){
    await deployer.deploy(ProgramStore);
  }
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
