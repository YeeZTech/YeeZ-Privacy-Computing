开发 Fidelius 应用
------------------------
### 安装 Fidelius SDK
开发应用需安装 Fidelius 开发环境，即 Fidelius SDK，编译安装请参考[文档](./Release_ZH.md)。

Fidelius 的默认安装目录为 `$HOME`，其目录结构如下：
```tree
.
├── include
├── lib
├── bin
```
- `include` 为 Fidelius 提供的头文件，`lib` 为 Fidelius 提供的动态、静态链接库。
- `bin` 为 Fidelius 提供的工具，如密码协议工具（yterminus）、数据加密工具（data_provider）、数据分析工具（fid_analyzer）等。


### 应用项目
一个典型的应用项目目录结构如下：
```tree
src
├── CMakeLists.txt
├── enclave
│   ├── CMakeLists.txt
│   ├── enclave.config.xml
│   ├── enclave_debug.lds
│   ├── enclave.lds
│   ├── enclave_private.pem
│   ├── enclave_parser.h
│   └── eparser.cpp
└── plugin
    ├── CMakeLists.txt
    ├── my_reader.cpp
    └── my_reader.h
```
目录 `enclave` 包含了数据分析的核心算法，目录 `plugin` 定义了输入数据的读取方式。

#### `enclave` 目录说明

- `enclave.config.xml` 为 enclave 的配置文件，定义了产品号、安全版本号，为运行在可信执行环境中的程序分配了栈空间大小、堆空间大小等。
- `enclave_debug.lds` 和 `enclave.lds` 保留原始内容不做修改。
- `enclave_private.pem` 为签名 enclave 的私钥。

##### `enclave_parser.h` 说明
在 `enclave_parser.h` 中，需定义一个数据分析的类，例如 `my_enclave_parser`，并且需要在该类中实现数据分析的函数 `inline stbox::bytes do_parse(const stbox::bytes &param)`，以下是一个简单的实现方式。

```c++
// enclave_parser.h
class my_enclave_parser {
public:
  my_enclave_parser(ypc::data_source<stbox::bytes> *source)
      : m_source(source){};
  inline stbox::bytes do_parse(const stbox::bytes &param) {
    return stbox::bytes("Hello!");
  }

protected:
  ypc::data_source<stbox::bytes> *m_source;
};
```
除此之外，Fidelius 还提供了 [高性能计算框架 HPDA](./pdf/hpda.pdf) 帮助开发者实现 `do_parse` 函数、完成数据分析算法的开发，HPDA 中包括计算引擎、输入/输出、算子等。

##### `eparser.cpp` 说明
一个典型的 `eparser.cpp` 内容如下：
```c++
// eparser.cpp
#include "enclave_parser.h"
#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/stdeth.h"

using Crypto = ypc::crypto::eth_sgx_crypto;
ypc::algo_wrapper<Crypto, ypc::sealed_data_stream, 
                  my_enclave_parser,
                  ypc::onchain_result<Crypto>> pw;

YPC_PARSER_IMPL(pw);
```

`eparser.cpp` 是 Fidelius 中算法模板 `ypc::algo_wrapper` 的实例化，算法模板的包括七个参数，可对参数进行组合以完成具体的需求场景（例如：[数据托管场景](./pdf/delegation.pdf)、[链下交付场景](./pdf/offchain.pdf) 等）。

算法模板的定义以及参数的说明如下：
```c++
template<typename Crypto, 
    typename DataSession, 
    typename ParserT, 
    typename Result, 
    typename ModelT = void, 
    template <typename> class DataAllowancePolicy = ignore_data_allowance, 
    template <typename> class ModelAllowancePolicy = ignore_model_allowance
> class algo_wrapper;
```

- `Crypto`：密码算法簇，目前支持以下两种加密算法：
    - `ypc::crypto::eth_sgx_crypto`：椭圆曲线为 Secp256k1，哈希函数为 sha3-256，对称加密算法为 Rijndael128GCM，兼容以太坊。
    - `ypc::crypto::gmssl_sgx_crypto`：椭圆曲线为 SM2，哈希函数为 SM3，对称加密算法为 SM4。
- `DataSession`：数据源方式，支持：
    - 无数据源（`noinput_data_stream`）
    - 单一未加密数据源（`raw_data_stream`）
    - 单一加密数据源（`sealed_data_stream`）
    - 多数据源（`multi_data_stream`）且其中每一个数据源都是加密数据。
- `ParserT`：表示自定义的算法类，由开发者自行开发。
- `Result`：表示结果的类型，支持
    - 本地结果（`local_result`）：结果不加密
    - 链上结果（`onchain_result`）：将结果发送到区块链中交付，要求数据大小能够上链
    - 链下结果交付（`offchain_result`）：结果不发送到区块链中
    - 结果加密转发（`forward_result`）：将结果转发给下一个计算任务使用
- `ModelT`: 表示模型参数的类型，是 `ff::util::ntobject<...>`，由开发者指定，默认为 `void`，即没有模型参数。
- `DataAllowancePolicy`: 表示数据源的许可验证策略：
    - 支持不检查数据源的许可（`ignore_data_allowance`）
    - 检查数据源的许可（`check_data_allowance`）
- `ModelAllowancePolicy`: 表示模型的许可验证策略：
    - 支持不检查模型的许可（`ignore_model_allowance`）
    - 检查模型参数的许可（`check_model_allowance`）

#### `plugin` 目录说明
数据分析中的输入数据是任意的，因此开发者提供开发算法的同时，需要提供读取数据的方式，通过实现以下接口定义读取数据的方式。
```c++
// 打开文件，创建文件指针
void *create_item_reader(const char *file_path, int len);

// 重置文件指针
int reset_for_read(void *handle);

// 从当前文件指针处获取一个 item，并存入 buf
int read_item_data(void *handle, char *buf, int *len);

// 关闭文件，重置文件指针
int close_item_reader(void *handle);

// 获取 item 个数
uint64_t get_item_number(void *handle);
  ```

### 参考文档
- [Fidelius 编程](./pdf/programming.pdf)
- [Fidelius 高性能计算框架（HPDA）](./pdf/hpda.pdf)
- [Fidelius 数据托管场景](./pdf/delegation.pdf)
- [Fidelius 链下交付场景](./pdf/offchain.pdf)
- [Fidelius 多数据融合场景](./pdf/multi_stream.pdf)
- [Fidelius 复杂任务场景](./pdf/task_graph.pdf)