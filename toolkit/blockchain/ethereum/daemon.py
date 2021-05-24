import threading
import time
import argparse
import random
import traceback

import handler
import sys
sys.path.append('./common')
import common
import const
from eth_api import eth_api
from log import logger
from log import log_handler


class repeat_timer(threading.Timer):
    def run(self):
        while True:
            self.function(*self.args, **self.kwargs)
            if not self.finished.wait(self.interval):
                continue


def run(ea, yzdata_req_filter):
    try:
        handler.handle_yzdata_request(ea, yzdata_req_filter)
    except Exception as err:
        tb = traceback.format_exc()
        logger.warning('\n********** get exception **********\nerror:%s\ntraceback:%s\n' % (err, tb))
        log_handler.flush()
        print('\n********** get exception **********\nerror:%s\ntraceback:%s\n' % (err, tb))
        sys.exit()


def timer(host, project_id):
    ea = eth_api(host, project_id)
    logger.info('To start daemon timer')
    yzdata_req_contract = common.checksum_encode(const.contract_YZDataRequest[host])
    yzdata_req_filter = ea.create_filter('latest', 'latest', yzdata_req_contract, [[const.topic_RequestData]])
    t = repeat_timer(const.DAEMON_TIMER_IN_SECONDS, run, [ea, yzdata_req_filter])
    t.start()
    logger.info('Daemon started! Listen to Ethereum contract events...')


def main():
    parser = argparse.ArgumentParser(description='YPC Data Request Daemon', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--host', type=str, default='ropsten', help='network host connection')
    parser.add_argument('--project_id', type=str, default='', help='infura project id')
    args = parser.parse_args()
    timer(args.host, args.project_id)


if __name__ == '__main__':
    main()
