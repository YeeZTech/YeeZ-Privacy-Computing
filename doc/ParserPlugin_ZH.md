开发 Fidelius 解析插件
------------------------

`fid_analyzer` 本身不包含任何业务解析逻辑。它按 `module_id` 从解析器注册表中挑出一个**非可信解析模块**（一个 `.so`），通过稳定的 C ABI（`include/toolkit/analyzer/fid_parser_plugin.h`）与之通信，由该模块负责加载 enclave 并实现自己 EDL 契约里的 OCALL。

因此，业务方需要新增一种数据格式的解析能力时，**不需要修改 Fidelius，也不需要在 Fidelius 里新增目录**：在自己的项目里编译一个插件 DSO 即可。本文中的 `myorg` 代指你自己的项目命名空间。

### 为什么可能需要多个插件

enclave 的 OCALL 表在签名时就固定了。一旦你给 EDL 增删 OCALL，旧的、已经签名发布的 enclave 就不能再由新插件服务。这时的做法不是改旧 enclave，而是**为每一代契约保留一个插件**，各自用不同的 `module_id`：

```
myorg.eparser.v1   ocall_get_frame
myorg.eparser.v2   ocall_get_frame + ocall_get_page
myorg.eparser.v3   ocall_get_data
```

`module_id` 是注册表里 enclave 与插件之间的绑定键，**一旦发布就不能重命名**。

### 引入 CMake 支持

```cmake
find_package(YPC CONFIG REQUIRED)
```

`YPCConfig.cmake` 会自动 `include(FidParserPlugin)`，之后即可使用：

- `add_fid_parser_plugin()` —— 编译一个插件 DSO
- `fid_mark_parser_enclave()` —— 标记一个 enclave 属于哪个契约，`enclave_sign()` 随后会自动为它生成注册片段
- `fid_register_parser_enclave()` —— 手工为某个已签名文件生成注册片段

### 编译插件 DSO

```cmake
add_fid_parser_plugin(
  TARGET myorg_eparser_plugin
  MODULE_ID myorg.eparser.v1
  EDL ${CMAKE_CURRENT_SOURCE_DIR}/myorg_parser.edl
  EDL_SEARCH_PATHS
    ${YPC_INCLUDE_DIR}/
    ${YPC_INCLUDE_DIR}/ypc/edl/util/
    ${YPC_INCLUDE_DIR}/ypc/edl/
    ${YPC_INCLUDE_DIR}/ypc/stbox/
  SRCS myorg_parser_plugin.cpp handler/my_handler.cpp
  LINK_LIBRARIES YPC::stbox_common YPC::core glog)
```

该函数会：生成 Edger8r 的 untrusted 代码；计算 EDL 契约指纹；链接插件运行时 `YPC::fid_parser_plugin_runtime`；用版本脚本把导出符号收敛到 `fid_parser_plugin_query` 一个；把产物装到 `<prefix>/lib/fidelius/parser-modules/`；并在 `<prefix>/share/fidelius/parser-registry.d/` 写一份 `*.module.json`（含 DSO 的 SHA-256）。

插件源文件只需要提供一份运行时配置和一个入口，其余交给运行时：

```c++
#include <toolkit/analyzer/fid_parser_plugin_runtime.h>
#include "myorg_parser_u.h"     // Edger8r 生成

const fid_parser_plugin_runtime_config kConfig = {
    sizeof(fid_parser_plugin_runtime_config),
    FID_PARSER_MODULE_ID,                  // 由 add_fid_parser_plugin 定义
    FID_PARSER_EDL_CONTRACT_FINGERPRINT,   // 同上
    0,                                     // 行为标志位
    myorg_parser_begin_parse_data_item, /* ...其余 ECALL 桩... */ };

extern "C" FID_PARSER_PLUGIN_EXPORT int32_t fid_parser_plugin_query(
    uint32_t host_abi_major, uint32_t host_abi_minor,
    uint32_t host_api_struct_size, const fid_parser_plugin_api **api_out) {
  return fid_parser_plugin_runtime_query(&kConfig, host_abi_major,
                                         host_abi_minor, host_api_struct_size,
                                         api_out);
}
```

自己 EDL 新增的 OCALL 就在同一个文件里用 `extern "C"` 实现；运行时通过
`fid_parser_plugin_runtime_active_context()` 把当前实例的上下文交还给你。

### 注册 enclave

`fid_analyzer` 只运行注册表认可的已签名 enclave，绑定关系是 `.signed.so` 的精确 SHA-256 与 MRENCLAVE。有两条注册路径：

**一、本项目编译出来的 enclave —— 构建期自动注册。** 在 `enclave_sign()` 之前标记契约即可：

```cmake
fid_mark_parser_enclave(my_evaluate_parser
  MODULE_ID myorg.eparser.v1
  EDL ${CMAKE_CURRENT_SOURCE_DIR}/myorg_parser.edl
  EDL_SEARCH_PATHS ${YPC_INCLUDE_DIR}/:${YPC_INCLUDE_DIR}/ypc/edl/)
enclave_sign(my_evaluate_parser KEY ... CONFIG ...)
```

用 `add_ypc_applet()` 构建的 applet 使用标准 `eparser.edl` 契约，会被自动标记为 `fidelius.eparser.v1`，无需手工调用。

每次重新构建 enclave 都会改变它的 SHA-256，构建期注册会同步更新片段，因此这条路径不需要额外维护。

**二、早已签名发布、不再重新编译的二进制 —— 离线注册。** 用框架提供的
`<prefix>/share/fidelius/registry-tools/RegisterPublishedParserEnclaves.cmake`，
配一份属于你自己项目的清单：

```sh
cmake -DFID_ENCLAVE_DIR=<存放 *.signed.so 的目录> \
      -DFID_REGISTRY_OUTPUT_DIR=<prefix>/share/fidelius/parser-registry.d \
      -DFID_ENCLAVE_MANIFEST=<你的项目>/published_enclaves.cmake \
      -P <prefix>/share/fidelius/registry-tools/RegisterPublishedParserEnclaves.cmake
```

清单里可以调用 `fid_parser_contract()` / `fid_parser_standard_contract()` 求契约指纹，再用
`fid_register_published_enclave(<名字> <module_id> <指纹>)` 逐个登记。该脚本只读取、哈希、
`sgx_sign dump` 这些文件，不会签名也不会改写 enclave。

一份清单大致长这样：

```cmake
fid_parser_contract(v1_fingerprint EDL "${CMAKE_CURRENT_LIST_DIR}/myorg_parser.edl")
fid_parser_standard_contract(standard_fingerprint)

fid_register_published_enclave(my_evaluate_parser myorg.eparser.v1 "${v1_fingerprint}")
fid_register_published_enclave(txt_evaluate_parser fidelius.eparser.v1 "${standard_fingerprint}")
```

> 两条路径写的是同一个目录下的同名片段。请让它们指向**互不重叠**的 enclave 集合：
> 构建期路径负责本项目编译出来的，离线路径负责外部发布的。否则后跑的一方会覆盖前一方，
> 把同一个二进制标成不同的契约。

### 注册表是可信输入

`fid_analyzer` 会拒绝对 group/other 可写的注册表目录或片段。`GenerateFidParserRegistry.cmake`
写入时会设好 `0755`/`0644`；如果手工调整过目录，记得 `chmod go-w`。

运行时可以用 `--parser-registry-dir` 追加额外的片段目录，便于在不安装的情况下用构建树里的
`<build>/fidelius/parser-registry.d` 做测试。

### 参考

- 插件 C ABI：`include/toolkit/analyzer/fid_parser_plugin.h`
- 插件运行时：`include/toolkit/analyzer/fid_parser_plugin_runtime.h`
- 框架自带的标准插件：`toolkit/analyzer/plugins/eparser_plugin.cpp`
