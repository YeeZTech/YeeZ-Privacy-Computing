from sqlalchemy import func
from hexbytes import HexBytes
import math
import json
import time

import service
import sys
sys.path.append('./common')
import const
import common
from eth_api import eth_api
from eth_api import request_proxy
from eth_api import get_program_info
from eth_api import argument_encoding_str
from log import logger
sys.path.append('../model')
from db_tables import db
from db_tables import Contract
from db_tables import RequestData


def all_data_and_request_init(ea):
    query = db.session.query(Contract).all()
    [const.map_data_and_request.update({record.yz_data:record.yz_request}) for record in query]
    [const.map_request_and_data.update({record.yz_request:record.yz_data}) for record in query]


def __create_request_filter(ea, contract_request):
    return ea.create_filter('latest', 'latest', common.checksum_encode(contract_request), [[const.topic_RequestData]])


def __update_data_and_request(ea, contract_data):
    contract_request = request_proxy(ea, contract_data)
    info = Contract(yz_data=contract_data, yz_request=contract_request)
    query = db.session.query(Contract).filter(Contract.yz_data==contract_data).all()
    if not query:
        db.session.add(info)
        db.session.commit()
        const.map_data_and_request.update({contract_data:contract_request})
        const.map_request_and_data.update({contract_request:contract_data})
        const.request_filters.append(__create_request_filter(ea, contract_request))


def __handle_data(ea, change):
    assert 'data' in change
    contract_data = change['data'][-40:]
    __update_data_and_request(ea, contract_data)


def handle_data(ea, data_filter):
    changes = ea.get_filter_changes(data_filter.filter_id)
    # changes = [{'address': '0x65745e8C07d82DB6F3949f7aa9F91352550d6910', 'blockHash': HexBytes('0x463e947e9b80b22f7ef887fed3ba0fe3470eb4aa5ee439d4580d8a4c50569706'), 'blockNumber': 10117984, 'data': '0x000000000000000000000000b729c179b08c340683fe82be5a3c5c4ca7e374b3', 'logIndex': 5, 'removed': False, 'topics': [HexBytes('0x729f473103420c10ca9328745c7f6d9fca3abbe6dfbc6854a4e6c21455d46b4a')], 'transactionHash': HexBytes('0x914cad1a9f077f3df9f0f98c7c49dcaf1bd198b45189d5735cc9d0152e950d8e'), 'transactionIndex': 2}]
    if changes:
        print('\nGet data filter changes, size: %d, content: %s\n' % (len(changes), str(changes)))
        logger.info('Get data filter changes, size: %d, content: %s' % (len(changes), str(changes)))
        [__handle_data(ea, change) for change in changes]


def __parse_string_or_bytes_arguments_encoding(data, offset_in_bytes):
    p_hex_len = const.PARAMS_BYTES_IN_HEX_LENGTH
    ratio = int(p_hex_len / const.PARAMS_BYTES)
    offset_in_hex = common.safe_str_cast(offset_in_bytes, 16) * ratio
    arg_len = common.safe_str_cast(data[offset_in_hex:offset_in_hex+p_hex_len],16) * ratio
    return data[offset_in_hex+p_hex_len : offset_in_hex+p_hex_len+arg_len]


def __parse_data_request_event_logs(data):
    items = common.split_by_32bytes(data)
    assert len(items) > 7
    p_hex_len = const.PARAMS_BYTES_IN_HEX_LENGTH
    ratio = int(p_hex_len / const.PARAMS_BYTES)
    # parse param `request_hash`
    request_hash = items[0]
    # parse param `secret`
    secret_hex = __parse_string_or_bytes_arguments_encoding(data, items[1])
    # parse param `input`
    input_hex = __parse_string_or_bytes_arguments_encoding(data, items[2])
    # parse param `forward_sig`
    forward_sig_hex = __parse_string_or_bytes_arguments_encoding(data, items[3])
    # parse param `program_hash`
    program_hash = items[4]
    # parse param `pkey`
    pkey_hex = __parse_string_or_bytes_arguments_encoding(data, items[6])
    return request_hash, secret_hex, input_hex, forward_sig_hex, program_hash, pkey_hex


def __parse_program_info(info):
    items = common.split_by_32bytes(info)
    assert len(items) > 6
    p_hex_len = const.PARAMS_BYTES_IN_HEX_LENGTH
    ratio = int(p_hex_len / const.PARAMS_BYTES)
    # parse param `program_url`
    url_offset = common.safe_str_cast(items[3], 16) * ratio
    url_length = common.safe_str_cast(info[url_offset:url_offset+p_hex_len], 16) * ratio
    url_hex = info[url_offset+p_hex_len : url_offset+p_hex_len+url_length]
    # parse param `enclave_hash`
    enclave_hash = items[5]
    return bytes.fromhex(url_hex).decode('utf-8'), enclave_hash


def __get_program_info(ea, program_hash):
    info = get_program_info(ea, const.contract_ProgramStore[ea.host], program_hash)
    return __parse_program_info(info)


def __handle_request(ea, change):
    assert 'data' in change
    data = change['data'][2:]
    request_hash, encrypted_skey, encrypted_input, forward_sig, program_hash, pkey4v = __parse_data_request_event_logs(data)
    assert 'address' in change
    contract_request = change['address'][2:].lower()
    contract_data = const.map_request_and_data[contract_request]
    logger.info('Parse event logs\n\trequest_hash: %s\n\tencrypted_skey: %s\n\tencrypted_input: %s\n\tforward_sig: %s\n\tprogram_hash: %s\n' % (request_hash, encrypted_skey, encrypted_input, forward_sig, program_hash))
    pkey4e = argument_encoding_str(ea, contract_data, const.hash_pkey, 'bytes')
    program_url, program_enclave_hash = __get_program_info(ea, program_hash)
    logger.info('Request params\n\tanalyzer_pkey: %s\n\tprovider_pkey: %s\n\tprogram_url: %s\n\tprogram_enclave_hash: %s\n' % (pkey4v, pkey4e, program_url, program_enclave_hash))
    rd = RequestData(request_hash=request_hash,encrypted_skey=encrypted_skey, encrypted_input=encrypted_input, provider_pkey=pkey4e, analyzer_pkey=pkey4v, program_enclave_hash=program_enclave_hash, forward_sig=forward_sig, status=0)
    db.session.merge(rd)
    db.session.commit()
    logger.info('Start data analysis service...')
    service.start_data_analysis_service(ea, contract_request, request_hash, program_url)


def handle_request(ea, yzdata_req_filter):
    changes = ea.get_filter_changes(yzdata_req_filter.filter_id)
    # changes = [{'address': '0xd92fcA55A74Fc9D0EEBd5c74654b5C55d846c990', 'blockHash': HexBytes('0x257d15b41c0a4beaff8093689287ffdf825e573a27c3794d304e9a9a367677f0'), 'blockNumber': 10121230, 'data': '0x131a7940af1508cf2e60de1c2874fc3ba501b76f3893473ea75424c53b43502d00000000000000000000000000000000000000000000000000000000000000e000000000000000000000000000000000000000000000000000000000000001800000000000000000000000000000000000000000000000000000000000000200022de6ba3d2a07d4f42862b28dd97ccbdaeb46ceaedc13485821c3bd4c06664e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002800000000000000000000000000000000000000000000000000000000000000070caf015f5327548597b1570961834cbc2c877fabb807596e65ac1ecdcbd5927b9cdfb1811df22a956794e20505da4d26d57f8e3678bd8ab6880dbe0d6cda71790638af909738fd11be77a643dedf727c0fe57358097da5f6513494e32a035aec73f5bcaf3897b826d9edd64f17ec1978e000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000053f49701cdfb1811df22a956794e20505da4d26d57f8e3678bd8ab6880dbe0d6cda71790638af909738fd11be77a643dedf727c0fe57358097da5f6513494e32a035aec7b466c514215e4eda1812e9e158b3234d000000000000000000000000000000000000000000000000000000000000000000000000000000000000000041d5dbf423d45d5f74388178769ac31d2baa41233383e340c5dc4f0ce179e5985a57a9dac41c5a98b538a221c863be89d9ce4b400cd0a17c1bdc3a4170ec7b9d111b0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000409e333811412e09381b31f179bdb6bf34b9f2accc3dad85a290d7fc6c9780b31317ea23616d5e15a2872c07ce66a0c37d9ab9de0bab10cd563f824051e0abe8da', 'logIndex': 5, 'removed': False, 'topics': [HexBytes('0xa2eb61ce32ba0f2b61d32b2505b63c7d91843928753a586e5a3d0feaf08948ff')], 'transactionHash': HexBytes('0x11812607ab77272c03ac1c120a36df533c2cc65fc1d184feee9fe2611c3bb8bc'), 'transactionIndex': 3}]
    if changes:
        print('\nGet request filter changes, size: %d, content: %s\n' % (len(changes), str(changes)))
        logger.info('Get request filter changes, size: %d, content: %s' % (len(changes), str(changes)))
        [__handle_request(ea, change) for change in changes]
