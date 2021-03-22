from sqlalchemy import func
from hexbytes import HexBytes
import math
import json
import time
import service
import sys
sys.path.append('./common')
from db_tables import db
from db_tables import RequestData
import const
import common
from eth_api import eth_api
from eth_api import pkeyFromAddr
from eth_api import owner
from eth_api import get_program_info
from log import logger


def __parse_yzdata_request_event_logs(data):
    items = common.split_by_32bytes(data)
    assert len(items) > 6
    p_hex_len = const.PARAMS_BYTES_IN_HEX_LENGTH
    ratio = int(p_hex_len / const.PARAMS_BYTES)
    # parse param `request_hash`
    request_hash = items[0]
    # parse param `secret`
    secret_offset = int(items[1],16) * ratio
    secret_length = int(data[secret_offset:secret_offset+p_hex_len],16) * ratio
    secret_hex = data[secret_offset+p_hex_len : secret_offset+p_hex_len+secret_length]
    # parse param `input`
    input_offset = int(items[2],16) * ratio
    input_length = int(data[input_offset:input_offset+p_hex_len],16) * ratio
    input_hex = data[input_offset+p_hex_len : input_offset+p_hex_len+input_length]
    # parse param `forward_sig`
    forward_sig_offset = int(items[3],16) * ratio
    forward_sig_length = int(data[forward_sig_offset:forward_sig_offset+p_hex_len],16) * ratio
    forward_sig_hex = data[forward_sig_offset+p_hex_len : forward_sig_offset+p_hex_len+forward_sig_length]
    # parse param `program_hash`
    program_hash = items[4]
    return request_hash, secret_hex, input_hex, forward_sig_hex, program_hash


def __get_request_sender(ea, tx_hash):
    tx_info = ea.get_transaction_receipt(tx_hash)
    assert 'from' in tx_info
    return tx_info['from'][2:].lower()


def __get_analyzer_pkey(ea, change):
    assert 'transactionHash' in change
    tx_hash = change['transactionHash'].hex()[2:]
    analyzer_addr = __get_request_sender(ea, tx_hash)
    return pkeyFromAddr(ea, const.contract_CertifiedUsers[ea.host], analyzer_addr)


def __get_provider_pkey(ea):
    provider_addr = owner(ea, const.contract_YZData[ea.host])
    return pkeyFromAddr(ea, const.contract_CertifiedUsers[ea.host], provider_addr)


def __parse_program_info(info):
    items = common.split_by_32bytes(info)
    assert len(items) > 6
    p_hex_len = const.PARAMS_BYTES_IN_HEX_LENGTH
    ratio = int(p_hex_len / const.PARAMS_BYTES)
    # parse param `program_url`
    url_offset = int(items[3],16) * ratio
    url_length = int(info[url_offset:url_offset+p_hex_len],16) * ratio
    url_hex = info[url_offset+p_hex_len : url_offset+p_hex_len+url_length]
    # parse param `enclave_hash`
    enclave_hash = items[5]
    return bytes.fromhex(url_hex).decode('utf-8'), enclave_hash


def __get_program_info(ea, program_hash):
    info = get_program_info(ea, const.contract_ProgramStore[ea.host], program_hash)
    return __parse_program_info(info)


def __handle_yzdata_request(ea, change):
    assert 'data' in change
    data = change['data'][2:]
    request_hash, encrypted_skey, encrypted_input, forward_sig, program_hash = __parse_yzdata_request_event_logs(data)
    logger.info('Parse event logs\n\trequest_hash: %s\n\tencrypted_skey: %s\n\tencrypted_input: %s\n\tforward_sig: %s\n\tprogram_hash: %s\n' % (request_hash, encrypted_skey, encrypted_input, forward_sig, program_hash))
    analyzer_pkey = common.pkey_endian_change(__get_analyzer_pkey(ea, change))
    provider_pkey = common.pkey_endian_change(__get_provider_pkey(ea))
    program_url, program_enclave_hash = __get_program_info(ea, program_hash)
    logger.info('Request params\n\tanalyzer_pkey: %s\n\tprovider_pkey: %s\n\tprogram_url: %s\n\tprogram_enclave_hash: %s\n' % (analyzer_pkey, provider_pkey, program_url, program_enclave_hash))
    rd = RequestData(request_hash=request_hash,encrypted_skey=encrypted_skey, encrypted_input=encrypted_input, provider_pkey=provider_pkey, analyzer_pkey=analyzer_pkey, program_enclave_hash=program_enclave_hash, forward_sig=forward_sig, status=0)
    db.session.merge(rd)
    db.session.commit()
    logger.info('Start data analysis service...')
    service.start_data_analysis_service(ea, request_hash, program_url)


def handle_yzdata_request(ea, yzdata_req_filter):
    changes = ea.get_filter_changes(yzdata_req_filter.filter_id)
    if changes:
        logger.info('Get filter changes, size: %d, content: %s' % (len(changes), str(changes)))
        [__handle_yzdata_request(ea, change) for change in changes]
