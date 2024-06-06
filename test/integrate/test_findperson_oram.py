from classic_job_oram import classic_job_oram
import os
import common
import sys
import random
import re

def gen_personlist(**kwargs):
    cmd = os.path.join(common.bin_dir, "./personlist_gen_oram")
    output = common.execute_cmd(cmd)
    return [cmd, output]


if __name__ == "__main__":
    name = "findperson_oram"
    gen_personlist()

    crypto = "stdeth"
    data = "person_list"
    parser = os.path.join(common.lib_dir, "person_first_match_oram.signed.so")
    plugin = os.path.join(
        common.lib_dir, "libperson_reader_oram{}.so".format(common.debug_postfix()))
    
    con_read_num = 5

    cj = classic_job_oram(crypto, name, data, parser, plugin, con_read_num, {
        'request-use-js': True,
        'remove-files': True if len(sys.argv) < 2 else False,
    })

    id = 421003198607262336
    result = []
    for i in range(con_read_num):
        ran_i = random.randint(0, 7970 - 1)
        id_str = str(id + ran_i)
        # id_str = str(id + i)
        input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"" + id_str + "\\\"}]\""
        cj.input = input_param
        cj.run()
        result.append(cj.result)
        # print("input_param is : ", input_param)
        # print("result is : ", cj.result)
        matches = re.findall(r"\d+", cj.result[0])
        print(str(id))
        if matches[0] != str(id_str):
            print("not find target row!")
            break
    print(result)