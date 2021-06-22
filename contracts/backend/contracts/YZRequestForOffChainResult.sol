pragma solidity >=0.4.21 <0.6.0;
import "./CertInterface.sol";
import "./SafeMath.sol";
import "./ECDSA.sol";
import "./YZDataInterface.sol";
import "./ProgramProxyInterface.sol";



contract YZDataRequestForOffChainResult {
  using SafeMath for uint256;
  using ECDSA for bytes32;

  enum RequestStatus{init, upload_url, request_key, settled, revoked}

    struct YZRequest{
      address payable from;
      bytes pkey4v;
      bytes secret;
      bytes input;
      bytes forward_sig;
      bytes32 program_hash;
      bytes32 result_hash;
      uint token_amount;
      uint gas_price;
      uint block_number;
      RequestStatus status;
      bool exists;
    }
    mapping (bytes32 => YZRequest) request_infos;

    YZDataInterface data;
    CertInterface public cert_proxy;
    uint256 public total_deposit;

    constructor(address _data) public{
      data = YZDataInterface(_data);
      cert_proxy = CertInterface(data.get_cert_proxy());
      total_deposit = 0;
    }

    event RequestData(bytes32 request_hash, bytes secret, bytes input, bytes forward_sig, bytes32 program_hash, uint gas_price);
    function request_data(bytes memory secret,
                          bytes memory input,
                          bytes memory forward_sig,
                          bytes32 program_hash, uint gas_price) public payable returns(bytes32 request_hash){

      //!Ignore the check for now
      bytes memory pkey = cert_proxy.pkeyFromAddr(msg.sender);
      require(pkey.length != 0, "not a registered user");
      require(data.is_program_hash_available(program_hash), "invalid program");
      (,,,,,bool is_offchain,)= ProgramProxyInterface(data.program_proxy()).get_program_info(program_hash);
      require(is_offchain, "can only use program with offchain result");

      request_hash = keccak256(abi.encode(msg.sender, pkey, secret, input, forward_sig, program_hash, gas_price));
      require(request_infos[request_hash].exists == false, "already exist");
      require(msg.value >= data.price() + data.program_price(program_hash), "not enough budget");

      request_infos[request_hash].from = msg.sender;
      request_infos[request_hash].pkey4v = pkey;
      request_infos[request_hash].secret = secret;
      request_infos[request_hash].input = input;
      request_infos[request_hash].forward_sig = forward_sig;
      request_infos[request_hash].program_hash = program_hash;
      request_infos[request_hash].token_amount = msg.value;
      request_infos[request_hash].gas_price = gas_price;
      request_infos[request_hash].block_number = block.number;
      request_infos[request_hash].status = RequestStatus.init;
      request_infos[request_hash].exists = true;

      emit RequestData(request_hash, secret, input, forward_sig, program_hash, gas_price);
      return request_hash;
    }

    event RefundRequest(bytes32 request_hash, uint256 old_amount, uint256 new_amount);
    function refund_request(bytes32 request_hash) public payable{
      require(request_infos[request_hash].exists, "request not exist");
      require(request_infos[request_hash].status != RequestStatus.settled, "already settled");
      require(request_infos[request_hash].from == msg.sender, "only request owner can refund");

      uint256 old = request_infos[request_hash].token_amount;
      request_infos[request_hash].token_amount = request_infos[request_hash].token_amount.safeAdd(msg.value);

      total_deposit += msg.value;
      emit RefundRequest(request_hash, old, request_infos[request_hash].token_amount);
    }

    event SubmitResultForOffChain(bytes32 request_hash, string result_url);
    function submit_result_offchain_url(bytes32 request_hash, string memory result_url) public returns(bool){
      require(request_infos[request_hash].exists, "request not exist");
      require(request_infos[request_hash].status == RequestStatus.init, "only for init status");
      request_infos[request_hash].status = RequestStatus.upload_url;
      emit SubmitResultForOffChain(request_hash, result_url);
      return true;
    }

    event RequestResultDecryptionKey(bytes32 request_hash, bytes32 result_hash);
    function request_result_decryption_key(bytes32 request_hash, bytes32 result_hash) public returns(bool){
      require(request_infos[request_hash].exists, "request not exist");
      require(request_infos[request_hash].status == RequestStatus.upload_url, "only for upload_url status");
      require(request_infos[request_hash].from == msg.sender, "only for request sender");
      request_infos[request_hash].status = RequestStatus.request_key;
      request_infos[request_hash].result_hash = result_hash;
      emit RequestResultDecryptionKey(request_hash, result_hash);

      return true;
    }


    event SubmitResultDecryptionKey(bytes32 request_hash, bytes key, uint cost_gas, bytes sig, uint256 cost_token, uint256 return_token);
    function submit_result_decryption_key(bytes32 request_hash, bytes memory key, uint cost_gas, bytes memory sig) public returns(bool){
      require(request_infos[request_hash].exists, "request not exist");
      require(request_infos[request_hash].status == RequestStatus.request_key, "only for request key");
      YZRequest storage r = request_infos[request_hash];

      (,,,,,,bytes32 enclave_hash)= ProgramProxyInterface(data.program_proxy()).get_program_info(r.program_hash);

      bytes32 vhash = keccak256(abi.encodePacked(key, r.result_hash, data.data_hash(), cost_gas, enclave_hash, r.input));
      bool v = verify_signature(vhash.toEthSignedMessageHash(), sig, r.pkey4v);
      require(v, "invalid decryption key");

      uint amount = cost_gas.safeMul(request_infos[request_hash].gas_price);
      amount = amount.safeAdd(data.price()).safeAdd(data.program_price(r.program_hash));

      require(amount <= r.token_amount, "insufficient fund");

      r.status= RequestStatus.settled;
      uint rest = r.token_amount.safeSub(amount);
      total_deposit = total_deposit.safeSub(amount);
      data.bank_account().transfer(amount);

      //TODO pay program author

      if(rest > 0){
        total_deposit = total_deposit.safeSub(rest);
        r.from.transfer(rest);
      }

      emit SubmitResultDecryptionKey(request_hash, key, cost_gas, sig, amount, rest);
      return true;
    }

    event RevokeRequest(bytes32 request_hash);
    function revoke_request(bytes32 request_hash) public{
      YZRequest storage r = request_infos[request_hash];
      require(msg.sender == r.from, "not owner of this request");
      require(request_infos[request_hash].status == RequestStatus.init, "only for init status");

      //require(block.number - r.block_number >= revoke_period, "not long enough for revoke");

      //TODO: charge fee for revoke
      r.status = RequestStatus.revoked;
      total_deposit = total_deposit.safeSub(r.token_amount);
      r.from.transfer(r.token_amount);
      emit RevokeRequest(request_hash);
    }

    function verify_signature(bytes32 hash, bytes memory sig, bytes memory pkey) private pure returns (bool){
      address expected = getAddressFromPublicKey(pkey);
      return hash.recover(sig) == expected;
    }

    function getAddressFromPublicKey(bytes memory _publicKey) private pure returns (address addr) {
      bytes32 hash = keccak256(_publicKey);
      assembly {
        mstore(0, hash)
        addr := mload(0)
      }
    }
}

contract YZDataRequestForOffChainFactory{
  event NewYZDataRequest(address addr);
  function createYZDataRequest(address data) public returns(address){
    YZDataRequestForOffChainResult r = new YZDataRequestForOffChainResult(data);
    emit NewYZDataRequest(address(r));
    return address(r);
  }
}
