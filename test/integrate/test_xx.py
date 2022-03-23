from classic_job import classic_job
import os
import common

if __name__ == "__main__":
    name = "xx"
    data = os.path.join(
        common.bin_dir, "../../../dataset/rawdata/各省市区保护野生植物.csv")
    parser = os.path.join(common.lib_dir, "simple_parser.signed.so")
    plugin = os.path.join(common.lib_dir, "libsimple_reader.so")
    input_param = "\"[{\\\"type\\\":\\\"string\\\",\\\"value\\\":\\\"西伯利亚冷杉\\\"}]\""
    cj = classic_job(name, data, parser, plugin, input_param)
    cj.run()

    print("result is : ", cj.result)
