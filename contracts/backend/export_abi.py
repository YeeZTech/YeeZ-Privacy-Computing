import json
import os
import sys

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
build_dir = os.path.join(current_dir, "build/contracts/")

def gen_abi_file(fp, name):
    tf = json.load(open(fp))
    print('processing {}'.format(fp))
    # print tf
    abi = tf['abi']
    bytecode = tf['bytecode']
    nfp = os.path.join(build_dir, name + ".abi.json")
    open(nfp, 'w').write(json.dumps(abi))

def get_all_json_files_in_dir(abs_dir):
  for item in os.listdir(abs_dir):
    if item.endswith('.json') and not item.endswith(".abi.json"):
      # files.append(abs_dir+'/' + item)
      gen_abi_file(os.path.join(abs_dir, item), item[0:item.find(".json")])

get_all_json_files_in_dir(build_dir)

