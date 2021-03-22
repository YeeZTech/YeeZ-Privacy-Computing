import web3
import json
import sys
import util


class contract:
    def __init__(self, w3, compiled_path):
        assert isinstance(w3, web3.main.Web3)
        assert isinstance(compiled_path, str)
        self.w3 = w3
        self.compiled_home = compiled_path

    def __build_transaction(self, args):
        assert isinstance(args.sender, str)
        assert isinstance(args.gas_limit, int)
        assert isinstance(args.tx_value, int)
        transaction = {
            'from': args.sender,
            'nonce': self.w3.eth.getTransactionCount(args.sender),
            'value': args.tx_value,
            'gas': args.gas_limit,
            'gasPrice': self.w3.toWei('10', 'gwei')
        }
        return transaction

    def __call(self, contract, transaction, func_name, args):
        assert isinstance(contract, web3.contract.Contract)
        assert isinstance(transaction, dict)
        assert isinstance(func_name, str)
        assert isinstance(args, list)
        print(args)

        call_tx = eval('contract.functions.%s(*args).buildTransaction(transaction)' % func_name)
        ret = eval('contract.functions.%s(*args).call(transaction)' % func_name)
        print(ret)
        return call_tx

    def call_contract(self, args):
        assert isinstance(args.contract_address, str)
        assert isinstance(args.contract_name, str)
        assert isinstance(args.func_name, str)
        assert isinstance(args.func_args, list)
        transaction = self.__build_transaction(args)
        abi_path = '%s/%s.abi.json' % (self.compiled_home, args.contract_name)
        contract = util.construct_from_abi_and_address(self.w3, abi_path, args.contract_address)
        call_tx = self.__call(contract, transaction, args.func_name, args.func_args)
        return json.dumps(call_tx)
