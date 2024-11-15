#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

namespace datahub
{
    /**
     *  @brief 将文件信息插入到PropertyTree中的函数
     *  @param pt PropertyTree
     *  @param path 文件路径
     *  @param encryptedSize 文件加密后的大小
     *  @param offset 文件在加密后的偏移
     */
    void insertFileInfo(boost::property_tree::ptree &pt, const std::string &path, size_t encryptedSize, size_t offset)
    {
        // 如果 path 第一个字符是 /，则去掉
        std::string pre_path = path[0] == '/' ? path.substr(1) : path;
        std::replace(pre_path.begin(), pre_path.end(), '.', '*');
        std::replace(pre_path.begin(), pre_path.end(), '/', '.');

        pt.add_child(pre_path, boost::property_tree::ptree());
        auto current = &pt.get_child(pre_path);

        // 在叶节点上存储文件加密大小和偏移
        current->put("encryptedSize", encryptedSize);
        current->put("path", path);
        current->put("offset", offset);
    }

    /**
     * Helper struct to store file information
     */
    struct FileInfo
    {
        FileInfo(size_t encryptedSize, size_t offset, const std::string &path)
            : encryptedSize(encryptedSize), offset(offset), path(path) {}
        size_t encryptedSize;
        size_t offset;
        std::string path;
    };

    /**
     * Helper func to find file information
     */
    void getFileInfo(const boost::property_tree::ptree &pt, std::vector<FileInfo> &fileInfos)
    {
        // 读取叶节点上的文件信息
        boost::optional<size_t> encryptedSize = pt.get_optional<size_t>("encryptedSize");
        boost::optional<size_t> offset = pt.get_optional<size_t>("offset");
        boost::optional<std::string> path = pt.get_optional<std::string>("path");
        if (encryptedSize && offset && path)
        {
            fileInfos.emplace_back(*encryptedSize, *offset, *path);
        }
    }

    void findDirInfo(const boost::property_tree::ptree &pt, std::vector<FileInfo> &fileInfos)
    {
        for (const auto &entry : pt)
        {
            const std::string &nodeName = entry.first;
            const boost::property_tree::ptree &childNode = entry.second;

            if (nodeName.find("*") != std::string::npos)
            {
                getFileInfo(childNode, fileInfos);
                continue;
            }
            // 如果是目录节点，递归遍历子节点
            findDirInfo(childNode, fileInfos);
        }
    }

    
    /**
     * @brief Finds and retrieves file information from a PropertyTree.
     * 
     * This function searches for file information in a given PropertyTree based on 
     * a specified path. It returns a vector containing file information such as 
     * encrypted size, offset, and path. The path can either be a file or a directory. 
     * If the path is a file, it retrieves the information directly. If the path is 
     * a directory, it recursively searches through its children to gather file information.
     * 
     * @param pt The PropertyTree to search within.
     * @param path The file or directory path to search for.
     * @return A vector of FileInfo objects containing the file information.
     */
    std::vector<FileInfo> findFileInfo(const boost::property_tree::ptree &pt, const std::string &path)
    {
        std::vector<FileInfo> fileInfos;
        // 输入的路径是文件
        if (path.find(".") != std::string::npos)
        {
            std::string pre_path = path;
            std::replace(pre_path.begin(), pre_path.end(), '.', '*');
            std::replace(pre_path.begin(), pre_path.end(), '/', '.');
            auto child = pt.get_child(pre_path);
            getFileInfo(child, fileInfos);
            return fileInfos;
        }
        // 输入的路径是目录
        std::string dir = path;
        std::replace(dir.begin(), dir.end(), '/', '.');
        auto children = pt.get_child(dir);
        findDirInfo(children, fileInfos);
        return fileInfos;
    }

    /**
     *  @brief 序列化PropertyTree到二进制文件
     *  @param pt PropertyTree
     *  @param filename 文件路径
     */
    void serializeToBinaryFile(const boost::property_tree::ptree &pt, const std::string &filename)
    {
        // 将 PropertyTree 转换为 JSON 字符串
        std::ostringstream jsonStream;
        boost::property_tree::write_json(jsonStream, pt);
        std::string jsonString = jsonStream.str();

        // 获取 JSON 字符串的长度，并写入文件
        std::ofstream ofs(filename, std::ios::binary | std::ios::app); // 以二进制模式打开，并追加到文件末尾
        if (ofs)
        {
            size_t length = jsonString.size();
            ofs.write(jsonString.data(), length);                               // 写入 JSON 数据
            ofs.write(reinterpret_cast<const char *>(&length), sizeof(length)); // 写入长度信息
            ofs.close();
            std::cout << "Serialized to binary and appended to " << filename << std::endl;
            std::cout << "Serialized length: " << length << " bytes" << std::endl;
        }
        else
        {
            std::cerr << "Failed to open file for writing." << std::endl;
        }
    }

    /**
     *  @brief 通过文件路径反序列化 PropertyTree
     *  @param filename 文件路径
     *  @return PropertyTree
     *
     *  该函数通过文件读取二进制数据，并反序列化为 PropertyTree
     */
    boost::property_tree::ptree deserializeFromBinaryFile(const std::string &filename)
    {
        std::ifstream ifs(filename, std::ios::binary);
        boost::property_tree::ptree pt;

        if (ifs)
        {
            // 移动到文件末尾以读取序列化长度
            ifs.seekg(-static_cast<int>(sizeof(size_t)), std::ios::end);
            size_t length;
            ifs.read(reinterpret_cast<char *>(&length), sizeof(length));

            // 根据读取的长度，定位到序列化数据的起始位置
            ifs.seekg(-static_cast<int>(sizeof(size_t)) - static_cast<int>(length), std::ios::end);
            std::string jsonString(length, '\0');
            ifs.read(&jsonString[0], length);

            // 将 JSON 字符串解析为 PropertyTree
            std::istringstream jsonStream(jsonString);
            boost::property_tree::read_json(jsonStream, pt);

            std::cout << "Deserialized JSON from binary file." << std::endl;
        }
        else
        {
            std::cerr << "Failed to open file for reading." << std::endl;
        }

        return pt;
    }
}
