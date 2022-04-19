from classic_job import classic_job
import os
import common

if __name__ == "__main__":
    name = "iris"
    data = os.path.join(common.bin_dir, "iris.data")
    parser = os.path.join(common.lib_dir, "iris_parser.signed.so")
    plugin = os.path.join(common.lib_dir, "libiris_reader.so")
    input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"123\\\"}]\""
    cj = classic_job(name, data, parser, plugin, input_param)
    cj.run()

    print("result is : ", cj.result)
