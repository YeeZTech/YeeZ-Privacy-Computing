#!/usr/bin/python3
import os;
import sys;
import json;
import argparse;
import pathlib;

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
sdk_dir = os.path.dirname(os.path.dirname(current_dir))
csv_dir = os.path.join(current_dir, 'csv')
mysql_dir = os.path.join(current_dir, 'mysql')

def generate_top_cmake(target_dir, name):
    fp = os.path.join(target_dir, "CMakeLists.txt");
    with open(fp, 'w') as cm:
        cm.write("project({})\n".format(name))
        cm.write("cmake_minimum_required(VERSION 3.9.3)\n")
        cm.write("add_definitions(-std=c++11)\n")
        cm.write("include_directories(${PROJECT_SOURCE_DIR})\n")
        cm.write("include_directories({})\n".format(sdk_dir))
        cm.write("include_directories({})\n".format(os.path.join(sdk_dir, "core/include")))
        cm.write("include_directories({})\n".format(os.path.join(sdk_dir, "vendor/fflib/include")))
        cm.write("link_directories({})\n".format(os.path.join(sdk_dir, "vendor/fflib/lib")))
        cm.write("set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/)\n")
        cm.write("set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/)\n")
        cm.write("set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output/)\n")
        cm.write("add_subdirectory(reader)\n")

def generate_reader_cmake(reader_dir, name, config):
    fp = os.path.join(reader_dir, "CMakeLists.txt")
    with open(fp, 'w') as cm:
        cm.write("add_library({}_csv_reader SHARED reader.cpp)\n".format(name))
        cm.write("target_link_libraries({}_csv_reader pthread ff_net)".format(name))

    pass


def generate_csv_reader_file(cpp, name):
    cpp.write("#include<toolkit/plugins/csv/csv_reader.h>\n")
    cpp.write("typedef ypc::plugins::typed_csv_reader<{}_item_t> {}_reader_t;\n".format(name, name));
    cpp.write("impl_csv_reader({}_reader_t)".format(name))

def generate_mysql_reader_file(cpp, name, config):
    cpp.write("#include<toolkit/plugins/mysql/mysql_reader.h>\n")
    cpp.write("struct _mysql_table_t{")
    cpp.write("constexpr static const char * table_name = \"{}\";".format(config["table_name"]))
    cpp.write("};\n")
    cpp.write("typedef ff::sql::table<ff::sql::mysql<ff::sql::cppconn>, _mysql_table_t");


    schema = config["schema"]
    for item in schema:
        cpp.write(", {}".format(item["name"]))
    cpp.write("> _table_t;")

    cpp.write("typedef ypc::plugins::typed_mysql_reader<{}_item_t, _table_t> {}_reader_t;\n".format(name, name));
    cpp.write("impl_mysql_reader({}_reader_t)".format(name))

def generate_reader_file(target_dir, name, config):
    reader_dir = os.path.join(target_dir, "reader");
    os.mkdir(reader_dir)

    generate_reader_cmake(reader_dir, name, config);
    reader_cpp = os.path.join(reader_dir, "reader.cpp");
    with open(reader_cpp, 'w') as cpp:
        cpp.write('#include "output/user_type.h"\n');
        if config["type"] == "csv":
            generate_csv_reader_file(cpp, name)
        elif config["type"] == "mysql":
            generate_mysql_reader_file(cpp, name, config)
        else:
            print("invalid type {}".format(config["type"]))
            exit()

    pass


def build_all(target_dir, config):
    build_dir = os.path.join(target_dir, "build")
    os.mkdir(build_dir)
    cmd = "cd {} && cmake ../ && make".format(build_dir)
    os.system(cmd)
    pass

def check_target_dir(target_dir):
    td = "";
    if os.path.isabs(target_dir) :
        td = target_dir
    else:
        td = os.path.join(os.getcwd(), target_dir)

    if not os.path.isdir(td):
        print("{} is not a directory".format(td))
        exit()

    if not os.path.exists(td):
        print("{} does not exist".format(td))
        exit()

    return td

def get_config(config_fp):
    if not os.path.exists(config_fp):
        print("Configuration file {} doesn't exist!".format(config_fp))
        exit()

    with open(config_fp, 'r') as cf:
        data = json.loads(cf.read())
        return data

def generate_user_data_type(target_dir, config, name):
    output_dir = os.path.join(target_dir, "output");
    os.mkdir(output_dir)

    udt = os.path.join(output_dir, "user_type.h");
    with open(udt, 'w') as udth:
        udth.write("#pragma once\n")
        udth.write("#include <ff/util/ntobject.h>\n")
        udth.write("#include <string>\n")

        if config["type"] == "mysql":
            udth.write("#include <ff/sql.h>\n")
        schema = config["schema"]
        ft = "typedef ff::util::ntobject<"
        for item in schema:
            if config["type"] == "csv":
                udth.write("define_nt({}, {});\n".format(item["name"], item["type"]))

            if config["type"] == "mysql":
                udth.write("define_column({}, {}, {}, \"{}\");\n".format(item["name"], item["col"], item["type"], item["colname"]))
            ft = ft+ item["name"] + ","
        ft = ft.strip(',')
        ft = ft + '> {}_item_t;\n'.format(name)
        udth.write(ft)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate plugins for support data sources. A part of Fidelius.')
    parser.add_argument('--name', help='plugin name');
    parser.add_argument('--target-dir', type=pathlib.Path, help='plugin output directory');
    parser.add_argument('--config', type=pathlib.Path, help='JSON configuration file');
    args = parser.parse_args()
    target_dir = check_target_dir(args.target_dir)
    target_dir = os.path.join(target_dir, args.name)
    if os.path.exists(target_dir):
        print("{} already exist!".format(target_dir))
        exit()
    os.mkdir(target_dir)

    conf = get_config(args.config)
    # TODO we should make args.name valid
    generate_user_data_type(target_dir, conf, args.name)

    generate_top_cmake(target_dir, args.name)
    generate_reader_file(target_dir, args.name, conf)
    # generate_parser_file(target_dir, args.name, conf)
    build_all(target_dir, conf)
