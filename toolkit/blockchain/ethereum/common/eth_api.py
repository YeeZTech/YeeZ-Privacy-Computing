from web3 import Web3
# only effective for rinkeby network
# from web3.middleware import geth_poa_middleware
from hexbytes import HexBytes
import argparse
import const
import common


class eth_api:

    def __init__(self, host, project_id):
        assert isinstance(host, str)
        assert isinstance(project_id, str)
        self.host = host
        self.project_id = project_id
        self.wss = self.__websocket_provider(host, project_id)
        self.w3 = self.wss
        # only effective for rinkeby network
        # self.w3.middleware_onion.inject(geth_poa_middleware, layer=0)


    def __websocket_provider(self, host, project_id):
        url = 'wss://%s.infura.io/ws/v3/%s' % (host, project_id)
        return Web3(Web3.WebsocketProvider(url))


    def get_block(self, symbol):
        return self.w3.eth.getBlock(symbol)


    def get_block_height(self):
        block = self.get_block('latest')
        assert 'number' in block
        return block['number']


    def get_transaction_receipt(self, tx_hash):
        return self.w3.eth.getTransactionReceipt(tx_hash)


    def readonly_contract(self, contract_addr, data):
        contract_addr = common.checksum_encode(contract_addr)
        return self.eth_call({'to':contract_addr,'data':data})


    def eth_call(self, transaction):
        return self.w3.eth.call(transaction)


    def __check_filter_params(self, key, val, d):
        if val:
            d.update({key:val})
        return d


    def create_filter(self, block_s, block_e, address, topics):
        d = self.__check_filter_params('fromBlock', block_s, dict())
        d = self.__check_filter_params('toBlock', block_e, d)
        d = self.__check_filter_params('address', address, d)
        d = self.__check_filter_params('topics', topics, d)
        return self.wss.eth.filter(d)


    def get_filter_changes(self, filter_id):
        return self.wss.eth.getFilterChanges(filter_id)


def owner(ea, contract_addr):
    o_byte = ea.readonly_contract(contract_addr, '0x%s' % const.hash_owner[:8])
    return o_byte.hex()[-40:]


def pkeyFromAddr(ea, contract_addr, address):
    method_id = '0x%s' % const.hash_pkeyFromAddr[:8]
    address = common.checksum_encode(address)
    data = address[2:].lower().zfill(const.PARAMS_BYTES_IN_HEX_LENGTH)
    data = method_id + data
    pkey_byte = ea.readonly_contract(contract_addr, data)
    return pkey_byte.hex()[-128:]


def get_program_info(ea, contract_addr, program_hash):
    method_id = '0x%s' % const.hash_get_program_info[:8]
    data = program_hash.zfill(const.PARAMS_BYTES_IN_HEX_LENGTH)
    data = method_id + data
    info_byte = ea.readonly_contract(contract_addr, data)
    return info_byte.hex()[2:]


def main():
    parser = argparse.ArgumentParser(description='ETH API', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--host', type=str, default='ropsten', help='network host connection')
    parser.add_argument('--project_id', type=str, default='', help='infura project id')
    args = parser.parse_args()
    ea = eth_api(args.host, args.project_id)
    ret = owner(ea, const.contract_YZData[ea.host])
    ret = pkeyFromAddr(ea, const.contract_CertifiedUsers[ea.host], '0x2D68C532dc01482acE9397F8b9280732D3361063')
    ret = get_program_info(ea, const.contract_ProgramStore[ea.host], '59ac73b6b16fb44dd29e5f550f72001b19f9f0680db4e9ba176796527c984f77')
    print(ret)


if __name__ == '__main__':
    main()
