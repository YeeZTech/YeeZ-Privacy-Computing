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

    crypto = "stdeth"
    data = ["person_list1", "person_list2"]
    parser = os.path.join(common.lib_dir, "person_first_match_multi.signed.so")
    plugin = os.path.join(
        common.lib_dir, "libperson_reader{}.so".format(common.debug_postfix()))

    input_param = ""
    use_js = True
    if use_js:
        input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"421003198707262936\\\"}]\""
    else:
        input_param = "421003198607270527"

    cj = multistream_job(crypto, name, data, parser, plugin, input_param, {
        'request-use-js': use_js,
    })

    cj.run()

    print("result is : ", cj.result)
