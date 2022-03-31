#!/usr/bin/python3
import os
import sys
import subprocess
import json

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
test_dir = os.path.dirname(current_dir)
sdk_dir = os.path.dirname(test_dir).replace(" ", "\ ")

bin_dir = os.path.join(sdk_dir, "./bin")
lib_dir = os.path.join(sdk_dir, "./lib")
sealer_enclave = os.path.join(lib_dir, "edatahub.signed.so")
kmgr_enclave = os.path.join(lib_dir, "keymgr.signed.so")


def execute_cmd(cmd):
    print("execute_cmd: {}".format(cmd))
    cmd = 'node ' + cmd
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    p.wait()
    if p.returncode != 0:
        raise RuntimeError('Failed to execute cmd {}'.format(cmd))

    return p.stdout.read().decode('utf-8')


def fid_terminus(**kwargs):
    cmd = os.path.join(current_dir, "./js/simjs.js")
    for k, v in kwargs.items():
        cmd = cmd + " --{} {}".format(k, v)
    output = execute_cmd(cmd)
    return [cmd, output]
