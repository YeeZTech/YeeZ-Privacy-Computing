pragma solidity >=0.4.21 <0.6.0;
import "./CertInterface.sol";
import "./SafeMath.sol";
import "./ECDSA.sol";

contract YZDataInterface{
  uint256 public price;
  address payable public bank_account;
  function is_program_hash_available(bytes32 program_hash) public view returns(bool) ;
  function program_price(bytes32 program_hash) public view returns(uint256);
  function get_cert_proxy() public view returns(address);
}


contract YZDataRequest {
  using SafeMath for uint256;
  using ECDSA for bytes32;

    struct YZRequest{
      address payable from;
      bytes pkey4v;
      bytes secret;
      bytes input;
      bytes forward_sig;
      bytes32 program_hash;
      uint token_amount;
      uint gas_price;
      uint block_number;
      bool settled;
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
      request_infos[request_hash].settled = false;
      request_infos[request_hash].exists = true;

      emit RequestData(request_hash, secret, input, forward_sig, program_hash, gas_price);
      return request_hash;
    }

    event RefundRequest(bytes32 request_hash, uint256 old_amount, uint256 new_amount);
    function refund_request(bytes32 request_hash) public payable{
      require(request_infos[request_hash].exists, "request not exist");
      require(request_infos[request_hash].settled, "already settled");
      require(request_infos[request_hash].from == msg.sender, "only request owner can refund");

      uint256 old = request_infos[request_hash].token_amount;
      request_infos[request_hash].token_amount = request_infos[request_hash].token_amount.safeAdd(msg.value);

      total_deposit += msg.value;
      emit RefundRequest(request_hash, old, request_infos[request_hash].token_amount);
    }

    event SubmitResult(bytes32 request_hash, bytes data, uint cost_gas, bytes result, bytes sig, uint256 cost_token, uint256 return_token);
    event SResultInsufficientFund(bytes32 request_hash, uint256 expect_fund, uint256 actual_fund);
    function submit_result(bytes32 request_hash, bytes memory data_hash, uint cost, bytes memory result, bytes memory sig) public returns(bool){
      require(request_infos[request_hash].exists, "request not exist");
      require(!request_infos[request_hash].settled, "already settled");

      YZRequest storage r = request_infos[request_hash];
      bytes32 vhash = keccak256(abi.encodePacked(r.input, data_hash, uint64(cost), result));
      bool v = verify_signature(vhash.toEthSignedMessageHash(), sig, r.pkey4v);
      require(v, "invalid data");

      uint amount = cost.safeMul(request_infos[request_hash].gas_price);
      amount = amount.safeAdd(data.price()).safeAdd(data.program_price(r.program_hash));

      //emit event instead of revert, so users can refund
      if(amount > request_infos[request_hash].token_amount){
        emit SResultInsufficientFund(request_hash, amount, request_infos[request_hash].token_amount);
        return false;
      }

      r.settled = true;
      uint rest = r.token_amount.safeSub(amount);
      total_deposit = total_deposit.safeSub(amount);
      data.bank_account().transfer(amount);

      //TODO pay program author

      if(rest > 0){
        total_deposit = total_deposit.safeSub(rest);
        r.from.transfer(rest);
      }

      emit SubmitResult(request_hash, data_hash, cost, result, sig, amount, rest);
      return true;
    }

    event RevokeRequest(bytes32 request_hash);
    function revoke_request(bytes32 request_hash) public{
      YZRequest storage r = request_infos[request_hash];
      require(msg.sender == r.from, "not owner of this request");
      require(r.settled == false, "alread settled");

      //require(block.number - r.block_number >= revoke_period, "not long enough for revoke");

      //TODO: charge fee for revoke
      r.settled = true;
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

contract YZDataRequestFactory{
  event NewYZDataRequest(address addr);
  function createYZDataRequest(address data) public returns(address){
    YZDataRequest r = new YZDataRequest(data);
    emit NewYZDataRequest(address(r));
    return address(r);
  }
}
