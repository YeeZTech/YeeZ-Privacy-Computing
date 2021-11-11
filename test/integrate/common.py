#!/usr/bin/python3
import os
import sys
import subprocess

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
    p = subprocess.Popen(cmd, shell=True, stdout = subprocess.PIPE)
    return p.stdout.read().decode('utf-8')

def fid_keymgr_create(user_id):
    pass

def fid_keymgr_list():
    cmd = os.path.join(bin_dir, "./keymgr_tool")
    output = execute_cmd("{} --list".format(cmd))
    ls = output.split("\n")
    tkeyid = ''
    keys = {}

    for l in ls:
        l = l.strip()
        if l.startswith(">> key "):
            ks = l.split(":")
            tkeyid = ks[1].strip()
        if l.startswith("public key:"):
            ks = l.split(":")
            pkey = ks[1].strip()
            if pkey.startswith(tkeyid):
                keys[tkeyid] = pkey

    return keys

def fid_data_provider(**kwargs):
    cmd = os.path.join(bin_dir, "./data_provider")
    for k, v in kwargs.items():
        cmd = cmd + " --{} {}".format(k, v)
    output = execute_cmd(cmd);
    return [cmd, output]

def fid_yprepare(**kwargs):
    cmd = os.path.join(bin_dir, "./yprepare")
    for k, v in kwargs.items():
        cmd = cmd + " --{} {}".format(k, v)
    output = execute_cmd(cmd);
    return [cmd, output]

def fid_analyzer(**kwargs):
    cmd = os.path.join(bin_dir, "./fid_analyzer")
    for k, v in kwargs.items():
        cmd = "GLOG_logtostderr=1 " + cmd + " --{} {}".format(k, v)
    output = execute_cmd(cmd);
    return [cmd, output]


