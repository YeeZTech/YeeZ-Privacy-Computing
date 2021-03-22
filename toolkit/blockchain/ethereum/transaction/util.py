import web3
import json
import os
from collections import namedtuple
from ethereum import utils
import threading


def read_file_content(file_path):
    assert isinstance(file_path, str)
    f = open(file_path, 'r')
    content = f.read()
    f.close()
    return content


def read_compiled_file(filename, compiled_type):
    assert isinstance(filename, str)
    assert isinstance(compiled_type, str)
    assert compiled_type is 'abi' or compiled_type is 'bytecode'
    content = read_file_content(filename)
    d = json.loads(content)
    if compiled_type is 'bytecode':
        d = d['object']
    return d


def __assign_if_exists(key, d, val_set):
    assert isinstance(key, str)
    assert isinstance(d, dict)
    assert isinstance(val_set, list)
    if key in d.keys():
        val_set.append(d[key])
    return val_set


def read_conf(filename, compiled_path):
    assert isinstance(filename, str)
    content = read_file_content(filename)
    d = dict({'compiled_path':'%s' % compiled_path})
    d.update(json.loads(content))

    Arguments = namedtuple('Arguments', d.keys())
    val_set = list()
    val_set = __assign_if_exists('compiled_path', d, val_set)
    val_set = __assign_if_exists('host', d, val_set)
    val_set = __assign_if_exists('project_id', d, val_set)
    val_set = __assign_if_exists('op_type', d, val_set)
    val_set = __assign_if_exists('keystore_path', d, val_set)
    val_set = __assign_if_exists('sender', d, val_set)
    val_set = __assign_if_exists('gas_limit', d, val_set)
    val_set = __assign_if_exists('tx_value', d, val_set)
    val_set = __assign_if_exists('contract_address', d, val_set)
    val_set = __assign_if_exists('contract_name', d, val_set)
    val_set = __assign_if_exists('func_name', d, val_set)

    if 'func_args' in d.keys():
        val_set.append(list(d['func_args'].values()))

    args = Arguments._make(val_set)
    # print(args.func_args)
    return args


def construct_from_abi_and_address(w3, abi_path, contract_address):
    assert isinstance(w3, web3.main.Web3)
    assert isinstance(abi_path, str)
    assert isinstance(contract_address, str)
    abi = read_compiled_file(abi_path, 'abi')
    contract = w3.eth.contract(address=contract_address, abi=abi)
    return contract
