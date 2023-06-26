#pragma once
#include "ypc/corecommon/utypes.h"// TODO:这里可能需要修改
#include "ypc/core/exceptions.h"
#include "ypc/core/memref.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <algorithm>



namespace ypc {
namespace oram {

template <uint64_t OramBlockNumLimit_t,
          uint32_t OramBlockSizeLimit_t, uint8_t OramBucketSize_t>
class oramblockfile {
public:
    const static uint64_t BlockNumLimit = OramBlockNumLimit_t;
    const static uint32_t DataSizeB = OramBlockSizeLimit_t;
    const static uint8_t BucketSizeZ = OramBucketSize_t;

    oramblockfile(const std::string &file_path): m_file(), m_file_path(file_path), m_header(), m_id_map() {
        open_for_write();
    }

    oramblockfile(const oramblockfile &) = delete;
    oramblockfile(oramblockfile &&) = delete;
    oramblockfile &operator=(const oramblockfile &) = delete;
    oramblockfile &operator=(oramblockfile &&) = delete;
    virtual ~oramblockfile() = default;

    
    
    void open_for_read() {
        if(m_file.is_open()) {
            throw std::runtime_error("already open");
        }

        m_file.open(m_file_path, std::ios::in | std::ios::binary);
        if (!m_file.is_open()) {
            throw file_open_failure(m_file_path, "oramblockfile::open_for_read");
        }

        read_header();
    }

    void open_for_write() {
        if (m_file.is_open()) {
            throw std::runtime_error("already open");
        }
        
        m_file.open(m_file_path, std::ios::in | std::ios::out | std::ios::binary);
        if (!m_file.is_open()) {
            throw file_open_failure(m_file_path, "oramblockfile::open_for_write");
        }
        read_header();
    }

    void reset() {
        m_file.clear();
        read_header();
    }

    void read_header() {
        m_file.seekg(0, m_file.beg);
        m_file.read((char *)&m_header, sizeof(header));
        // print_header();
    }

    void read_id_map() {
        m_file.seekg(m_header.id_map_filepos, m_file.beg);
        stbox::bytes id_map_str(m_header.position_map_filepos - m_header.id_map_filepos);
        m_file.read((char*)id_map_str.data(), id_map_str.size());

        id_map_t id_map_pkg = make_package<id_map_t>::from_bytes(id_map_str);
        auto id_map_array = id_map_pkg.get<id_map>();

        for(const auto& element : id_map_array) {
            m_id_map.insert({element.get<content_id>(), element.get<block_id>()});
        }

        // check out id_map
        // for(const auto& element : id_map_array) {
        //     uint64_t c_id = element.get<content_id>();
        //     uint32_t b_id = element.get<block_id>();
        //     // // std::cout << element.get<content_id>() << " " << element.get<block_id>() << std::endl;
        //     // std::cout << c_id << " " << b_id << " " << m_id_map[c_id] << std::endl;
        //     // assert(m_id_map[c_id] == b_id);
        // }

    }




    bool download_position_map(memref &posmap) {
        size_t len = m_header.oram_tree_filepos - m_header.position_map_filepos;
        // std::cout << "len = " << len << std::endl;
        if(posmap.data() == nullptr) {
            posmap.alloc(len);
        }
        if(posmap.size() < len) {
            posmap.dealloc();
            posmap.alloc(len);
        }

        m_file.seekg(m_header.position_map_filepos, m_file.beg);
        m_file.read((char *)posmap.data(), len);
        posmap.size() = len;
        return true;
    }

    uint32_t get_block_id(uint64_t c_id) {
        read_id_map();
        if(m_id_map.find(c_id) == m_id_map.end()) {
            throw std::runtime_error("not exist this content ID!");
        }
        return m_id_map.at(c_id);
    }

    const uint32_t& get_block_num() const{
        return m_header.block_num;
    }

    const uint32_t& get_bucket_num() const{
        return m_header.bucket_num_N;
    }

    const uint8_t& get_level_num() const{
        return m_header.level_num_L;
    }

    const uint32_t& get_bucket_str_size() const{
        return m_header.bucket_str_size;
    }

    const uint32_t& get_row_length() const{
        return m_header.row_length;
    }

    const uint32_t& get_batch_str_size() const{
        return m_header.batch_str_size;
    }

    void update_position_map(uint8_t * position_map, uint32_t len) {
        read_header();
        m_file.clear();

        stbox::bytes posmap_str(len);
        memcpy(posmap_str.data(), position_map, len);
        
        // std::cout << "----------oramblockfile update_position_map--------"  << std::endl;
        // std::cout << "posmap_str = " << posmap_str  << std::endl;
        // std::cout << "posmap_str.size() = " << posmap_str.size() << std::endl;



        // std::cout << "position_map_filepos = " << m_header.position_map_filepos << std::endl;
        m_file.seekp(m_header.position_map_filepos, m_file.beg);
        m_file.write((char *)(position_map), len);
    }


    bool download_path(uint32_t leaf, memref &en_path) {
        std::vector<uint32_t> offsets;
        leaf_to_offsets(leaf, offsets);
        // assert(offsets.size() == m_header.level_num_L + 1);
       
        std::vector<stbox::bytes> en_path_array;
        for(const uint32_t& offset : offsets) {
            stbox::bytes en_bucket_str(m_header.bucket_str_size);
            m_file.seekg(m_header.oram_tree_filepos + offset * m_header.bucket_str_size, m_file.beg);
            // std::streampos readPos = m_file.tellg();
            m_file.read((char *)en_bucket_str.data(), m_header.bucket_str_size);

            // bucket反序列化
            // bucket_pkg_t bucket_pkgE = make_package<bucket_pkg_t>::from_stbox::bytes(en_bucket_str);
            // auto block_array = bucket_pkgE.get<bucket>();

            // for(block_t e_block : block_array) {
            //     // std::cout << e_block.get<block_id>() << " " <<  e_block.get<leaf_label>() << " " << e_block.get<encrypted_batch>().size() << std::endl;
            // }

            en_path_array.push_back(en_bucket_str);
        }

        // 序列化
        path_pkg_t path_pkg;
        path_pkg.set<path>(en_path_array);
        stbox::bytes en_path_str = make_bytes<stbox::bytes>::for_package(path_pkg);
        

        size_t len = en_path_str.size();
        // std::cout << "len = " << len << std::endl;
        if(en_path.data() == nullptr) {
            en_path.alloc(len);
        }
        if(en_path.size() < len) {
            en_path.dealloc();
            en_path.alloc(len);
        }

        memcpy(en_path.data(), en_path_str.data(), len);
        en_path.size() = len;


        return true;
    }

    void upload_path(uint32_t leaf, uint8_t * encrpypted_path, uint32_t len) {
        read_header();
        m_file.clear();

        stbox::bytes encrpypted_path_str(len);
        memcpy(encrpypted_path_str.data(), encrpypted_path, len);
        
        // std::cout << "----------oramblockfile update_position_map--------"  << std::endl;
        // std::cout << "encrpypted_path_str.size() = " << encrpypted_path_str.size() << std::endl;

        // 反序列化
        path_pkg_t path_pkg = make_package<path_pkg_t>::from_bytes(encrpypted_path_str);
        auto bucket_str_array = path_pkg.get<path>();

        std::vector<uint32_t> offsets;
        leaf_to_offsets(leaf, offsets);
        // assert(offsets.size() == bucket_str_array.size());
        // std::cout << "oram_tree_filepos = " << m_header.oram_tree_filepos << std::endl;
        for(uint8_t i = 0; i < offsets.size(); ++i) {
            m_file.seekp(m_header.oram_tree_filepos + offsets[i] * m_header.bucket_str_size, m_file.beg);
            // std::streampos readPos = m_file.tellg();
            m_file.write((char *)bucket_str_array[i].data(), m_header.bucket_str_size);
        }
    }

    bool download_stash(memref &st) {
        if(!m_header.is_stash_valid) {
            return true;
        }

        m_file.seekg(0, m_file.end);
        std::streampos fileSize = m_file.tellg();
        size_t len = fileSize - m_header.stash_filepos;
        // std::cout << "len = " << len << std::endl;
        if(st.data() == nullptr) {
            st.alloc(len);
        }
        if(st.size() < len) {
            st.dealloc();
            st.alloc(len);
        }

        m_file.seekg(m_header.stash_filepos, m_file.beg);
        m_file.read((char *)st.data(), len);
        st.size() = len;
        return true;
    }

    void update_stash(uint8_t * stash, uint32_t len) {
        read_header();
        m_file.clear();

        if(!m_header.is_stash_valid) {
            m_header.is_stash_valid = true;
            m_file.seekp(0, m_file.beg);
            m_file.write((char *)&m_header, sizeof(m_header));
        }

        stbox::bytes stash_str(len);
        memcpy(stash_str.data(), stash, len);
        
        // std::cout << "----------oramblockfile update_position_map--------"  << std::endl;
        // std::cout << "posmap_str = " << posmap_str  << std::endl;
        // std::cout << "posmap_str.size() = " << posmap_str.size() << std::endl;

        // std::cout << "position_map_filepos = " << m_header.position_map_filepos << std::endl;
        m_file.seekp(m_header.stash_filepos, m_file.beg);
        // // 清空后续内容
        // m_file.write("", 0);
        // m_file.seekg(0, m_file.end);
        // std::streampos fileSize = m_file.tellg();
        // size_t tlen = fileSize - m_header.stash_filepos;
        // if(tlen != 0) {
        //     throw std::runtime_error("清空后续内容失败!");
        // }
        // m_file.seekp(m_header.stash_filepos, m_file.beg);
        m_file.write((char *)(stash), len);
        
    }

    void close() {
        m_file.close();
    }


private:

    void leaf_to_offsets(uint32_t leaf, std::vector<uint32_t> &offsets) {
        if(leaf == 0) {
            throw std::runtime_error("leaf label is invalid!");
        }

        // 将leaf转换成在Oram树中的索引
        uint32_t cur_node = (1 << m_header.level_num_L) - 1 + leaf - 1;

        bool flag = true;
        while(flag) {
            if(cur_node == 0)
                flag = false;
            
            // 这些块在server都是加密的
            offsets.push_back(cur_node);

            cur_node = (cur_node - 1) / 2;
        }
        std::reverse(offsets.begin(), offsets.end());
    }
    

    void print_header() {
        std::cout << "-------header params--------- " << std::endl;
        std::cout << "block_num = " << m_header.block_num << std::endl;
        std::cout << "bucket_num_N = " << m_header.bucket_num_N << std::endl;
        std::cout << "level_num_L = " << static_cast<int>(m_header.level_num_L) << std::endl;
        std::cout << "bucket_str_size = " << m_header.bucket_str_size << std::endl;
        std::cout << "row_length = " << m_header.row_length << std::endl;
        std::cout << "batch_str_size = " << m_header.batch_str_size << std::endl;
        std::cout << "-------header params--------- " << std::endl;
    }

    



    std::fstream m_file;
    std::string m_file_path;
    header m_header;
    // id_map_t m_id_map;
    // std::unordered_map<content_id, block_id> m_id_map;
    std::unordered_map<uint64_t, uint32_t> m_id_map;
};




}

}