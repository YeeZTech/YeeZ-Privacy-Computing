const CertifiedUsers = artifacts.require("CertifiedUsers");

module.exports = function(deployer) {
  deployer.deploy(CertifiedUsers);
};
