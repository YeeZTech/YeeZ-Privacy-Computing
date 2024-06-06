import os
import common
import subprocess

# 检查目录是否存在
def check_directory(directory_path):
  if os.path.exists(directory_path):
    return True
  else:
    print(f"\"{directory_path}\" does not exist.")
    return False

# 进入目录
def enter_directory(directory_path):
  if check_directory(directory_path):
    os.chdir(directory_path)
    return True
  else:
    return False

def compile_in_subdir(build_dir):
  # build_dir
  makefile_path = os.path.join(build_dir, 'Makefile')
  if not os.path.isfile(makefile_path):
    print(f"Error: makefile not found in {build_dir}")
    return False

  # 执行编译命令
  try:
    result = subprocess.run(['make', '-j8'], cwd = build_dir, check = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    print(result.stdout.decode('utf-8'))
    return True
  except subprocess.CalledProcessError as e:
    print(f"Error: {e.stderr.decode('utf-8')}")
    return False

def recompile(mode):
  cmd = os.path.join(common.sdk_dir, "./build.sh compile-project {} -j8").format(mode)
  output = common.execute_cmd(cmd)
  return [cmd, output]

def run_test_findperson_oram():
  cmd = 'python3 {}/test/integrate/test_findperson_oram.py'.format(common.sdk_dir)
  output = common.execute_cmd(cmd)
  return [cmd, output]


def gen_person_reader_oram(file_path, index_name):
  content = '''#include "person_reader_oram.h"
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
    std::string index_field = item_pkg.get<'''

  content = content + index_name

  content = content + '''>();
    memcpy(buf, index_field.c_str(), index_field.size());
    *len = index_field.size();
    
    r.dealloc();
    return 0;
  } else {
    *len = 0;
    return -1;
  }

  return 0;
}'''

  with open(file_path, "w") as file:
    file.write(content)




if __name__ == "__main__":

  index_name = "ZJHM"
  mode = "debug" # or "prerelease" or "release"

  file_path = os.path.join(common.sdk_dir, "example/oram_personlist/plugin/person_reader_oram.cpp")

  gen_person_reader_oram(file_path, index_name)

  build_dir = os.path.join(common.sdk_dir, 'build', mode)

  # 必须进入项目顶层目录执行
  if(enter_directory(common.sdk_dir)):

    # 重新编译
    if compile_in_subdir(build_dir) == False:
      recompile(mode)

    # 执行转换enclave并测试转换后加密数据文件是否能被正常读写
    r = run_test_findperson_oram()
    print(r[1])




