pragma solidity >=0.4.21 <0.6.0;

contract YZDataInterface{
  uint256 public price;
  bytes32 public data_hash;
  address payable public bank_account;
  function is_program_hash_available(bytes32 program_hash) public view returns(bool) ;
  function program_price(bytes32 program_hash) public view returns(uint256);
  function get_cert_proxy() public view returns(address);
  function program_proxy() public view returns(address);
}
