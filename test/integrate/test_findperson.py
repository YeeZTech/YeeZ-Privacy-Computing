from classic_job import classic_job
import os
import common


def gen_personlist(**kwargs):
    cmd = os.path.join(common.bin_dir, "./personlist_gen")
    output = common.execute_cmd(cmd)
    return [cmd, output]


if __name__ == "__main__":
    name = "findperson"
    gen_personlist()

    data = "person_list"
    parser = os.path.join(common.lib_dir, "person_first_match.signed.so")
    plugin = os.path.join(common.lib_dir, "libperson_reader.so")
    # input_param = "421003198607270527"
    input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"421003198607270233\\\"}]\""
    cj = classic_job(name, data, parser, plugin, input_param)
    cj.run()

    print("result is : ", cj.result)
