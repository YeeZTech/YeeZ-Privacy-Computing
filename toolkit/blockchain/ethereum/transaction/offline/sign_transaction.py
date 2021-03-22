from web3.auto import w3
from eth_account.messages import encode_defunct
import json
import sys
sys.path.append('../')
import util


def get_account(keystore_path, passwd):
    assert isinstance(keystore_path, str)
    assert isinstance(passwd, str)
    encrypted_key = util.read_file_content(keystore_path)
    private_key = w3.eth.account.decrypt(encrypted_key, passwd)
    account = w3.eth.account.privateKeyToAccount(private_key)
    return account


def sign_transaction(keystore_path, passwd, transaction):
    assert isinstance(keystore_path, str)
    assert isinstance(passwd, str)
    assert isinstance(transaction, dict)
    account = get_account(keystore_path, passwd)
    signed = account.signTransaction(transaction)
    return signed


def main():
    transaction_content = read_file_content(sys.argv[1])
    ret = sign_transaction(sys.argv[2], sys.argv[3], json.loads(transaction_content))
    d = {
        'raw_transaction': ret.rawTransaction.hex()
    }
    print(json.dumps(d))


if __name__ == '__main__':
    main()
