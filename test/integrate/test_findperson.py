from classic_job import classic_job
import os
import common
import sys


def gen_personlist(**kwargs):
    cmd = os.path.join(common.bin_dir, "./personlist_gen")
    output = common.execute_cmd(cmd)
    return [cmd, output]


if __name__ == "__main__":
    name = "personlist"
    gen_personlist()

    crypto = "stdeth"
    data = "person_list"
    parser = os.path.join(common.lib_dir, "person_first_match.signed.so")
    plugin = os.path.join(
        common.lib_dir, "libperson_reader{}.so".format(common.debug_postfix()))
    # input_param = "421003198607270527"
    input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"421003198607262936\\\"}]\""
    cj = classic_job(crypto, name, data, parser, plugin, input_param, {
        'request-use-js': True,
        'remove-files': True if len(sys.argv) < 2 else False,
    })
    cj.run()

    print("result is : ", cj.result)
