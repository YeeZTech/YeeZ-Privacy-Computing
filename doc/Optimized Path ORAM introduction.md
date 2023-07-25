# Optimized Path ORAM 实现数据访问模式保护



## Optimized Path ORAM实现

- example/oram_personlist/plugin/person_reader_oram.cpp
  
  - 增加一个函数`int get_item_index_field(void *handle, char *buf, int *len)`，功能是读取原始未加密文件的每行数据中用来查询的索引字段。
- toolkit/datahub/oram_seal_file.cpp 
  - 功能：
    - 加密文件为自定义的格式ORAM Sealed File，该文件结构包含：
      - 文件头header
        - 包含ORAM树的重要参数：真实块数`block_num`、ORAM树高`level_num_L`、ORAM树节点(bucket)数`bucket_num_N`以及ORAM Sealed File中各数据结构的起始偏移量。
      - 索引表id map
        - 存储的是索引字段哈希和数据块号`block_id`之间的映射关系。
      - ORAM树
        - 是一个以数组形式存储的完全二叉树；
        - 其每个节点是一个bucket，一个bucket是由`Z`个数据块构成的（`Z`为常量）；每个数据块的结构包括：数据块号`block_id`、叶子节点序号`leaf label`、数据块中有效的行数`valid_item_num`以及加密的数据`encrypted_batch`；
        - 其中数据块分为真实数据块和虚数据块，在虚数据块中，设置数据块号`block_id`、叶子节点序号`leaf_label`和数据块中有效的行数`valid_item_num`均为0，加密的数据`encrypted_batch`是随机字符填充的内容；在真实数据块中，数据块号`block_id`和叶子节点序号`leaf_label`均为正整数，其最大值取决于真实块数`block_num`和ORAM树高`level_num_L`，加密的数据`encrypted_batch`存储的是真实的加密数据。
      - 地址映射表position map
        - 存储的是数据块号`block_id`和其映射的叶子节点序号`leaf label`之间的关系。
      - 暂存区Stash
        - 存储的是每次数据访问操作后，根据Path ORAM协议的写回策略而无法写回ORAM树中的真实数据块，被称为溢出的块。
  - 步骤：
    - 先循环读取行数据，以确定用来存储数据的batch数量(即真实块数`block_num`)和每个batch中包含的数据行数
    - 建立索引表id map
      - 循环读取每行数据的索引字段，将其哈希和其所在的数据块的`block_id`的映射关系写入id map
      - 索引字段对应的是查询阶段的查询参数
    - 建立文件头header
      - 根据真实块数`block_num`确定ORAM树高`level_num_L`、ORAM树节点(bucket)数`bucket_num_N`
    - 向ORAM Sealed File中写入文件头header、加密的索引表id map
    - 建立并写入加密的ORAM树
      - 初始化ORAM树时，为了减少访问操作产生的溢出块数量，应尽可能将真实数据块写在ORAM树的底部。因此按照顺序在树的顶部写入虚数据块，再将所有真实数据块依次写入ORAM树的底部。
      - 在写入真实数据块，应该将数据块号`block_id`和其对应的叶子节点序号`leaf label`的映射关系存储在地址映射表position map。
      - 由于使用数组形式存储ORAM树，需要保证所有的数据块是相同大小的。因此若最后一个batch包含的真实行数据数量小于一个batch最大可以容纳的行数，需要使用随机字符补足行数，用数据块结构中的`valid_item_num`记录有效的行数据行数。
    - 写入加密的地址映射表position map
    - 更新并写入文件头header中position map和stash的起始偏移量
- include/ypc/core_t/analyzer/oram_sealed_data_provider.h
  - 功能：实现path ORAM算法，获取到查询参数（索引字段）对应的数据块
  - 类`oram_sealed_data_provider`需要的初始化参数：
    - 数据提供方的加密数据文件哈希`data_hash`
    - 数据提供方枢私钥`private_key`
    - 数据提供方枢公钥`pkey`
    - 解密的参数`decrypted_param`
  - 成员函数`access()`
    - 功能：根据`decrypted_param`获取其所在的batch，实现了path ORAM算法获取目标块的过程。
    - 步骤：
      1. `download_oram_params()`函数发起一个OCALL`download_oram_params_OCALL`在`data_hash`对应的加密数据文件中，读取存储的ORAM树的相关参数信息；
      2. `get_block_id(block_id)`函数发起一个OCALL`get_block_id_OCALL`在`data_hash`对应的加密数据文件中，获取`decrypted_param`所在的数据块对应的数据块号`block_id`;
      3. `download_position_map()`函数发起一个OCALL`download_position_map_OCALL`在`data_hash`对应的加密数据文件中，读取加密的地址映射表position map，使用数据提供方枢私钥`private_key`来解密position map；
      4. 在地址映射表position map中获取`block_id`映射的叶节点`leaf`, 并使其映射一个新的随机的叶节点`new_leaf`；
      5. `download_path(leaf)`函数发起一个OCALL`download_path_OCALL`在`data_hash`对应的加密数据文件中，读取ORAM树根节点到leaf对应的叶节点的一条加密路径；
      6. `download_stash()`函数发起一个OCALL`download_stash_OCALL`在`data_hash`对应的加密数据文件中，读取上次访问数据操作后加密的溢出块数组，并使用数据提供方枢私钥`private_key`来解密出溢出块存储在暂存区`m_stash`;
      7. `decrypt_path()`函数使用数据提供方枢私钥`private_key`来解密路径，将解密的路径中的真实数据块添加到暂存区`m_stash`中；
      8. `access_in_stash(b_id, new_leaf)`函数在暂存区`m_stash`中查找数据块号`block_id`对应的数据块的数据记录在`m_items`中，`m_valid_item_num`记录目标数据块的有效行数；
      9. `rebuild_new_path(leaf)`函数根据`leaf`和暂存区`m_stash`中所有真实数据块映射的叶节点来重建一条写回ORAM树中的新路径。一个数据块在新路径中的位置是其映射的叶节点和leaf的最近公共祖先节点，如果最近公共祖先节点所在的bucket已经放满了Z个真实数据块，则沿着最近公共祖先节点到根节点的路径寻找空块，如果直到根节点也没找空块，则该数据块就是一个溢出块，它存放在暂存区`m_stash`中；
      10. `encrypt_path()`函数使用数据提供方枢公钥`pkey`来加密新路径；
      11. `update_position_map()`函数先使用数据提供方枢公钥`pkey`来加密更新后的地址映射表position map，然后发起一个OCALL`update_position_map_OCALL`在`data_hash`对应的加密数据文件中，写入更新后的加密position map；
      12. `upload_path(leaf)`函数发起一个OCALL`upload_path_OCALL`在`data_hash`对应的加密数据文件中，将加密后的新路径写回ORAM树中；
      13. `update_stash()`函数将暂存区`m_stash`中溢出的块使用数据提供方枢公钥`pkey`加密后，再发起一个OCALL`download_oram_params_OCALL`在`data_hash`对应的加密数据文件中，写入加密的溢出块数组。
  - 成员函数`process()`
    - 功能：遍历目标块中的每行数据，将每行数据发送给数据使用方过滤，以找到查询参数所在的数据行。
- include/ypc/core_t/analyzer/interface/data_interface.h
  - 增加一个类模板对应数据源解析类型`oram_sealed_datasource_parser`
  - `uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len)`
    - 从`data_source_info`中获取加密的查询参数`param_data`，加密查询参数使用的数据使用方的枢公钥`param_data_pkey`
    - 用`param_data_pkey`请求对应的数据使用方枢私钥`param_private_key`，使用`param_private_key`来解密`param_data`
    - 使用 数据提供方的加密数据文件哈希`data_hash`、数据提供方枢私钥`private_key`、数据提供方枢公钥`pkey`和解密的参数`decrypted_param`去初始化类`oram_sealed_data_provider`
- toolkit/analyzer/parsers/oram_parser.h
  - 功能：定义了path ORAM算法中调用的所有OCALL函数的接口函数，这些OCALL接口函数其实是调用了`oramblockfile.h`中对加密文件ORAM sealed file中的读写方法。
  - 在`feed_datasource()`中调用`m_parser->init_data_source(data_info_bytes)`时传递的参数`data_info_bytes`需要包含数据使用方的加密请求参数。
- include/ypc/core/oramblockfile.h
  - 功能：提供了读写加密文件ORAM sealed file中各种数据结构的方法。
  - `bool get_block_id(bytes &item_index_field_hash, uint32_t *block_id)`：读取ORAM Sealed File中的索引表id map进入内存，获取索引字段哈希`item_index_field_hash`所在的数据块号`block_id`；
  - `bool download_position_map(memref &posmap)`：读取ORAM Sealed File中加密的地址映射表position map；
  - `bool update_position_map(uint8_t * position_map, uint32_t len)`：向ORAM Sealed File写入更新后的加密地址映射表position map；
  - `bool download_path(uint32_t leaf, memref &en_path)`：读取从ORAM树根节点到叶节点`leaf`的路径；
  - `bool upload_path(uint32_t leaf, uint8_t * encrpypted_path, uint32_t len)`：向ORAM Sealed File写入一条重建的从ORAM树根节点到叶节点`leaf`的新路径；
  - `bool download_stash(memref &st)`：读取ORAM Sealed File中加密的暂存区Stash（只存储溢出数据块）；
  - `bool update_stash(uint8_t * stash, uint32_t len)`：向ORAM Sealed File写入访问操作后的加密暂存区Stash（只写入溢出块）。

- include/ypc/core/oram_sealed_file.h

  - 是对include/ypc/core/oramblockfile.h类的一个封装。

    

## 测试

- `test/integrate/classic_job_oram.py`
  - 类`classic_job`增加了一个成员变量`con_read_num`来设置连续读的次数；
  - 类`classic_job`的方法`run(self)`：仅在第一次读的时候生成数据提供方枢钥、生成数据使用方枢钥和加密原始数据文件

- `test/integrate/test_findperson_oram.py`可以设置连续读的次数和随机的查询参数，初始化类`classic_job`，通过调用类`classic_job`的方法`run(self)`来测试连续查询的结果的正确性。



## 后续优化部分

- CMOV指令实现

  ```cpp
  std::vector<oram_ntt::block_t> m_stash;
  std::vector<uint32_t> m_position_map;
  stbox::bytes m_encrypted_path;
  std::vector<oram_ntt::bucket_pkg_t> m_decrypted_path;
  ```

  以上使用了ntobject的换成结构体，调用OCALL的时候换成ntobject再序列化写到enclave以外（有必要吗？）就可以编写汇编函数了

- 默克尔树验证完整性

  - `include/ypc/core_t/analyzer/interface/data_interface.h`中函数`check_actual_data_hash()`待实现

- 对每个查询参数字段都要建立索引

- 查询参数是唯一的，不唯一的时候如何做？

- 范围查询

  - 数据文件在加密前按序排列，建立索引表id map时，block id也是按序排列的
    - 例如查询参数为10~100， 其对应的行数据在的block对应的ID可能为１～５
    - 拿到对应的块组合就可以找到查询参数对应的行数据组合