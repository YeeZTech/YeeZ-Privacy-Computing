//
// Created by gaowh on 9/13/23.
//

#include "lib.hpp"

int ydump_cmdline(int argc, char *argv[])
{
    boost::program_options::variables_map vm;
    try
    {
        vm = parse_command_line(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "invalid cmd line parameters!" << std::endl;
        return -1;
    }

    std::string etype = "parser";
    if (vm.count("enclave-type") != 0u)
    {
        etype = vm["enclave-type"].as<std::string>();
    }

    std::string enclave_path;
    if (vm.count("enclave") != 0u)
    {
        enclave_path = vm["enclave"].as<std::string>();
    }
    else
    {
        std::cerr << "missing --enclave";
        exit(-1);
    }

    std::unordered_map<std::string, std::string> result;
#ifdef DEBUG
    LOG(INFO) << "this is for debug";
#endif

    ypc::bytes enclave_hash;
    if (etype == "parser")
    {
        ypc::parser_sgx_module mod(enclave_path.c_str());
        mod.get_enclave_hash(enclave_hash);

        uint32_t parser_type = mod.get_parser_type();
        get_enclave_type(parser_type, result);
        // enclave_hash = ypc::bytes(_enclave_hash.data(), _enclave_hash.size());
    }
    // TODO we should support other types, and more info here, like version,
    // signer

    std::stringstream ss;
    ss << enclave_hash.as<ypc::hex_bytes>();
    result["enclave-hash"] = ss.str();
    result["version"] = ypc::get_ypc_version();

    if (vm.count("output") != 0u)
    {
        std::string output_path =
            ypc::complete_path(vm["output"].as<std::string>());

        boost::property_tree::ptree pt;
        for (auto &it : result)
        {
            pt.put(it.first, it.second);
        }
        boost::property_tree::json_parser::write_json(output_path, pt);
    }
    else
    {
        for (auto &it : result)
        {
            std::cout << it.first << ": " << it.second << std::endl;
        }
    }

    return 0;
}

int ydump(
    const std::string &etype,
    const std::string &enclave_path,
    const std::string &output_path)
{
    if (enclave_path == "")
    {
        std::cerr << "missing enclave_path";
        exit(-1); 
    }
    
    std::unordered_map<std::string, std::string> result;
#ifdef DEBUG
    LOG(INFO) << "this is for debug";
#endif

    ypc::bytes enclave_hash;
    if (etype == "parser")
    {
        ypc::parser_sgx_module mod(enclave_path.c_str());
        mod.get_enclave_hash(enclave_hash);

        uint32_t parser_type = mod.get_parser_type();
        get_enclave_type(parser_type, result);
        // enclave_hash = ypc::bytes(_enclave_hash.data(), _enclave_hash.size());
    }
    // TODO we should support other types, and more info here, like version,
    // signer

    std::stringstream ss;
    ss << enclave_hash.as<ypc::hex_bytes>();
    result["enclave-hash"] = ss.str();
    result["version"] = ypc::get_ypc_version();

    if (output_path != "")
    {
        boost::property_tree::ptree pt;
        for (auto &it : result)
        {
            pt.put(it.first, it.second);
        }
        boost::property_tree::json_parser::write_json(output_path, pt);
    }
    else
    {
        for (auto &it : result)
        {
            std::cout << it.first << ": " << it.second << std::endl;
        }
    }

    return 0; 
}