from data_host import classic_job
import os
import common

if __name__ == "__main__":
    # should also set template param (Crypto) of iris_parser
    # and update iris_parser.signed.so by compiling the source code
    crypto = "stdeth"

    name = "iris"
    data = os.path.join(common.bin_dir, "iris.data")
    parser = os.path.join(common.lib_dir, "iris_parser.signed.so")
    plugin = os.path.join(
        common.lib_dir, "libiris_reader{}.so".format(common.debug_postfix()))
    input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"12\\\"}]\""
    cj = classic_job(crypto, name, data, parser, plugin, input_param)
    cj.run()

    print("result is : ", cj.result)
