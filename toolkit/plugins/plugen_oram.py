#!/usr/bin/python3
import os
import sys
import json
import argparse
import pathlib

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
sdk_dir = os.path.dirname(os.path.dirname(current_dir))


# print(current_file)
# print(current_dir)
# print(sdk_dir)

def generate_reader_oram_h(target_dir, name):
  reader_h = os.path.join(target_dir, name + "_reader_oram.h")
  code = """
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void *create_item_reader(const char *file_path, int len);

int reset_for_read(void *handle);
int read_item_data(void *handle, char *buf, int *len);
int close_item_reader(void *handle);
uint64_t get_item_number(void *handle);

int get_item_index_field(void *handle, char *buf, int *len);

#ifdef __cplusplus
}
#endif
  """
  with open(reader_h, 'w') as file:
    file.write(code)

    
def generate_reader_oram_cpp(target_dir, name, index_name):
  reader_cpp = os.path.join(target_dir, name + "_reader_oram.cpp")
  code_1 = """
#include "person_reader_oram.h"
#include "ypc/common/limits.h"
#include "../common.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

void *create_item_reader(const char *file_path, int len) {

  file_t *f = new file_t();
  f->open_for_read(file_path);
  f->reset_read_item();
  return f;
}

int reset_for_read(void *handle) {
  if (!handle) {
    return -1;
  }
  file_t *f = (file_t *)handle;
  f->reset_read_item();
  return 0;
}

int read_item_data(void *handle, char *buf, int *len) {
  if (!handle) {
    return -1;
  }
  if (!buf) {
    return -1;
  }
  file_t *f = (file_t *)handle;

  ypc::memref r;
  bool t = f->next_item(r);
  if (t) {
    memcpy(buf, r.data(), r.size());
    *len = r.size();
    r.dealloc();
    return 0;
  } else {
    *len = 0;
    return -1;
  }

  return 0;
}

int close_item_reader(void *handle) {
  file_t *f = (file_t *)handle;
  f->close();
  return 0;
}

uint64_t get_item_number(void *handle) {
  file_t *f = (file_t *)handle;
  return f->item_number();
}

int get_item_index_field(void *handle, char *buf, int *len) {
  if (!handle) {
    return -1;
  }
  if (!buf) {
    return -1;
  }
  file_t *f = (file_t *)handle;

  ypc::memref r;
  bool t = f->next_item(r);
  if (t) {
    typedef typename ypc::cast_obj_to_package<row_t>::type pkg_t;
    auto item_pkg =
        ypc::make_package<pkg_t>::from_bytes(ypc::bytes(r.data(), r.size()));
  """

  code_2 = "    std::string index_field = item_pkg.get<{}>();".format(index_name)
  
  code_3 = """
    memcpy(buf, index_field.c_str(), index_field.size());
    *len = index_field.size();
    
    r.dealloc();
    return 0;
  } else {
    *len = 0;
    return -1;
  }

  return 0;
}
  """
  with open(reader_cpp, 'w') as file:
    file.write(code_1)
    file.write(code_2)
    file.write(code_3)

def generate_cmake(target_dir, name):
  cmake_file = os.path.join(target_dir, "CMakeLists.txt")
  with open(cmake_file, 'w') as file:
    file.write("add_library(person_reader_oram SHARED {}_reader_oram.cpp)\n".format(name))
    file.write("target_link_libraries({}_reader_oram core)".format(name))

def build_all():
  build_dir = os.path.join(sdk_dir, "build")
  # TODO:加上mod变量
  build_dir = os.path.join(build_dir, "debug")
  # os.mkdir(build_dir)
  # TODO:CMAKE指令修改
  cmd = "cd {} && cmake ../.. && make".format(build_dir)
  os.system(cmd)
  pass

def check_target_dir(target_dir):
  td = ""
  if os.path.isabs(target_dir) :
    td = target_dir
  else:
    td = os.path.join(sdk_dir, target_dir)

  if not os.path.isdir(td):
    print("{} is not a directory".format(td))
    exit()

  if not os.path.exists(td):
    print("{} does not exist".format(td))
    exit()

  return td


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Generate plugins for support data sources. A part of Fidelius.')
  parser.add_argument('--name', help='plugin name');
  parser.add_argument('--target-dir', type=pathlib.Path, help='plugin output directory');
  parser.add_argument('--index-name', type=pathlib.Path, help='the name of the index field');
  args = parser.parse_args()
  target_dir = check_target_dir(args.target_dir)
  target_dir = os.path.join(target_dir, "test_plugin")
  # target_dir = args.target_dir
  print(target_dir)
  if os.path.exists(target_dir):
    print("{} already exist!".format(target_dir))
    build_all()
    exit()
  os.mkdir(target_dir)
  generate_reader_oram_h(target_dir, args.name)
  generate_reader_oram_cpp(target_dir, args.name, args.index_name)
  generate_cmake(target_dir, args.name)
  build_all()






