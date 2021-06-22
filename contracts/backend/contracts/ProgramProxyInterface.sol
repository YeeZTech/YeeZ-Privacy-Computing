pragma solidity >=0.4.21 <0.6.0;

contract ProgramProxyInterface{
    function get_program_info(bytes32 hash) public view returns(address author,
                                                                string memory program_name,
                                                                string memory program_desc,
                                                                string memory program_url,
                                                                uint256 price,
                                                                bool for_offchain,
                                                                bytes32 enclave_hash);
}
