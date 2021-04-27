from ethereum import utils
import subprocess
import shlex
import sys
import const


def run_cmd(cmd):
    assert isinstance(cmd, str)
    ret = subprocess.check_output(shlex.split(cmd))
    return ret.decode('utf-8')


def checksum_encode(addr):
    addr = addr.replace('0x', '')
    addr = addr.lower()
    o = ''
    v = utils.big_endian_to_int(utils.sha3(addr))
    for i, c in enumerate(addr):
        if c in '0123456789':
            o += c
        else:
            o += c.upper() if (v & (2**(255 - 4*i))) else c.lower()
    return '0x'+o


def split_by_32bytes(data):
    step = const.PARAMS_BYTES_IN_HEX_LENGTH
    return [data[i:i+step] for i in range(0, len(data), step)]


def safe_str_cast(num_str, base):
    try:
        return int(num_str, base)
    except Exception as err:
        logger.info('\n!!!!!!!!!! string to integer cast failed !!!!!!!!!!\nerror:%s\n' % err)
        log_handler.flush()
        print('\n!!!!!!!!!! string to integer cast failed !!!!!!!!!!\nerror:%s\n' % err)
    raise Exception('\n!!!!!!!!!! string to integer cast failed !!!!!!!!!!\n')


def main():
    ret = pkey_endian_change(sys.argv[1])
    print(ret)
    ret = pkey_endian_change(ret)
    print(ret)


if __name__ == '__main__':
    main()
