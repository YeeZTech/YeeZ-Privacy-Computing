from classic_job_oram import classic_job_oram
from classic_job_convert import classic_job_convert
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
    name = "convert_person"
    gen_personlist()

    crypto = "stdeth"
    # crypto = "gmssl"
    data = "person_list"
    convert_parser = os.path.join(common.lib_dir, "convert_sealed_file.signed.so")
    convert_parser2 = os.path.join(common.lib_dir, "convert_sealed_file2.signed.so")
    oram_parser = os.path.join(common.lib_dir, "person_first_match_oram.signed.so")
    plugin = os.path.join(
        common.lib_dir, "libperson_reader{}.so".format(common.debug_postfix()))
    
    # con_read_num必须>=3，前两次转换成ORAM密封格式
    con_read_num = 6

    cj = classic_job_convert(crypto, name, data, convert_parser, convert_parser2, oram_parser, plugin, con_read_num, {
        'request-use-js': True,
        'remove-files': True if len(sys.argv) < 2 else False,
    })

    id = 421003198607262336
    result = []
    for i in range(con_read_num):
        ran_i = random.randint(0, 7970 - 1)
        id_str = str(id + ran_i)
        input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"" + id_str + "\\\"}]\""

        cj.input = input_param
        cj.run()

        if(cj.read_count > 2):
            result.append(cj.result)

            matches = re.findall(r"\d+", cj.result[0])
            if matches[0] != str(id_str):
                print("not find target row!")
                break
    print(result)