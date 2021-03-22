pragma solidity >=0.4.21 <0.6.0;

contract CertInterface{
  function addrFromPKey(bytes memory pkey) public view returns(address);

  function pkeyFromAddr(address addr) public view returns(bytes memory);

  function is_pkey_exist(bytes memory pkey) public view returns(bool);
}
