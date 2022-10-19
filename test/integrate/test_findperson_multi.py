from multistream_job import multistream_job
import os
import common


def gen_personlist(**kwargs):
    cmd = os.path.join(common.bin_dir, "./personlist_gen_multi")
    output = common.execute_cmd(cmd)
    return [cmd, output]


if __name__ == "__main__":
    name = "findperson"
    gen_personlist()

    data = ["person_list1", "person_list2"]
    parser = os.path.join(common.lib_dir, "person_first_match_multi.signed.so")
    plugin = os.path.join(common.lib_dir, "libperson_reader.so")
    # input_param = "421003198607270527"
    input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"421003198707262936\\\"}]\""
    cj = multistream_job(name, data, parser, plugin, input_param)
    cj.run()

    print("result is : ", cj.result)
