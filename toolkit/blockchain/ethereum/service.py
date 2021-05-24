from threading import Thread
import os
import sys

sys.path.append('./common')
import common
from db_tables import db
from db_tables import RequestData
from eth_api import eth_api
from log import logger

sys.path.append('./transaction')
import util
from transaction import transaction


ypc_home = os.environ['YPC_HOME']
ypc_bin = '%s/bin' % ypc_home
ypc_lib = '%s/lib' % ypc_home

tx_home = '{}/toolkit/blockchain/ethereum/transaction'.format(ypc_home)
submitter_passwd = os.environ['SUBMITTER_PASSWD']


def __download_program(program_url):
    filename = os.path.basename(program_url)
    save_path = '%s/%s' % (ypc_bin, filename)
    logger.info('Downloading analysis program to path: %s' % save_path)
    ret = common.run_cmd('''wget -O %s %s''' % (save_path, program_url))
    logger.info(ret)
    return save_path


def __update_func_args(request_hash, args):
    query = db.session.query(RequestData).filter(RequestData.request_hash==request_hash).all()
    if not query:
        return False, args
    assert len(query) is 1
    record = query[0]
    args = args._replace(keystore_path='{}/{}'.format(tx_home, args.keystore_path))
    args = args._replace(func_args=[request_hash,record.data_hash,0,record.encrypted_result,record.result_signature])
    print(args)
    return True, args


def __submit_result(ea, request_hash):
    args = util.read_conf('{}/config/YZDataRequest.json'.format(tx_home), '{}/compiled'.format(tx_home))
    succ, args = __update_func_args(request_hash, args)
    if not succ:
        return
    tx = transaction(ea.host, ea.project_id)
    tx.call_and_send_transaction(args.keystore_path, submitter_passwd, args)


def __settle_request(request_hash):
    query = db.session.query(RequestData).filter(RequestData.request_hash==request_hash).all()
    if not query:
        return
    assert len(query) is 1
    record = query[0]
    rd = RequestData(request_hash=request_hash, status=2)
    db.session.merge(rd)
    db.session.commit()


def __start_data_analysis_service(ea, request_hash, program_url):
    program_path = __download_program(program_url)
    logger.info('Start to run analysis program...')
    cmd = '''{0}/fid_analyzer --sealed-data-url {0}/iris.data.sealed --sealer-path {1}/edatahub.signed.so --keymgr-path {1}/keymgr.signed.so --parser-path {2} --source-type db --db-conf ypcd.conf --request-hash {3}'''.format(ypc_bin, ypc_lib, program_path, request_hash)
    ret = common.run_cmd(cmd)
    logger.info(ret)
    logger.info('Run analysis program done')
    __submit_result(ea, request_hash)
    logger.info('Submit analysis result/signature to blockchain (smart contract)')
    __settle_request(request_hash)


def start_data_analysis_service(ea, request_hash, program_url):
    t = Thread(target=__start_data_analysis_service, args=[ea, request_hash, program_url])
    t.start()
