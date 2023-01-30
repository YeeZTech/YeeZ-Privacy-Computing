> 中文 / [English](../README.md)

# Fidelius - 熠智隐私计算中间件
## 特性
Fidelius 基于“数据可用不可见”思想，推出了面向数据合作的一站式隐私保护解决方案，有效保证了原始数据的一致性、计算逻辑的可控性、计算结果
的正确性及隐私性。

下图描述了基于 Fidelius 实现数据合作的抽象流程。图中参与方包括数据提供方和数据使用方。Fidelius 中间件分别运行在数据提供方和数据使用方中，双方通过与 Fidelius 交互实现数据合作。原始数据不会离开数据提供方的 Fidelius 中间件，这从根本上避免了隐私数据泄露的问题。

![](Fidelius-Infr.png)

相比传统的数据合作模式，Fidelius 可选择引入区块链网络。由于区块链本身具有去中心化网络、公开可验证等特性，Fidelius 将其作为可信的传输通道和数据计算验证平台。

## 文档
- [Fidelius: YeeZ Privacy Protection for Data Collaboration - A Blockchain based Solution](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)

## 快速开始
建议开发者登陆个人的 Github，把项目 fork 一个自己的版本，然后在上面进行修改。
### 环境依赖
- 确保拥有如下操作系统：
  * Ubuntu 20.04 LTS Server 64bits

- Fidelius 基于 Intel SGX 运行，若要使用完整功能，需确认硬件环境配备了支持的中央处理器（CPU），并对 BIOS 进行设置。即使不具备该硬件环境，仍能在安装了 Intel SGX SDK 之后运行 DEBUG 版本。
  * 检查 BIOS 和 CPU 是否启用 SGX，请遵循 [SGX-hardware](https://github.com/ayeks/SGX-hardware) 中的 [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) 。或直接使用如下方式进行确认：
```
 $ git clone https://github.com/ayeks/SGX-hardware.git
 $ cd SGX-hardware
 $ gcc test-sgx.c -o test-sgx
 $ ./test-sgx
```
若其中包括如下两行，则 Intel SGX 的硬件配置是符合的。
```
 ...
 sgx available: 1
 ...
 sgx 1 supported: 1
```
若 sgx available 字段为 0，则 CPU 本身不支持 Intel SGX；若 sgx 1 supported 字段为0，则说明 BIOS 不支持或未开启 Intel SGX 功能。为了能够执行 Intel SGX 的程序，还需要按照下一步正确安装 Intel SGX SDK。

- 按照 [linux-sgx](https://github.com/intel/linux-sgx) 项目中 [README.md](https://github.com/intel/linux-sgx/blob/master/README.md) 文档进行编译并安装`Intel(R) SGX`驱动`SDK`和`PSW`。注意：硬件环境不支持 sgx 的情况下无法安装`PSW`。 

- 安装依赖工具（protobuf，glog，boost，cppconn等）：
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```
- 安装 secp256k1 库
```
$ git clone https://github.com/bitcoin-core/secp256k1.git
$ cd secp256k1
$ ./autogen.sh
$ ./configure --enable-module-ecdh --enable-module-recovery
$ make
$ make check
$ sudo make install
```

### 源码编译 Fidelius - develop 分支
Fidelius 的 develop 分支的构建系统使用CMake（>=3.12)
```
$ git clone https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
$ cd YeeZ-Privacy-Computing 
$ git checkout develop
$ git submodule update --init
$ cd vendor/fflib && mkdir -p build && cd build && cmake -DRelease=1 .. && make -j8
$ cd YeeZ-Privacy-Computing 
$ cmake -DSGX_MODE=Debug -S . -B ./build_debug
$ cmake --build ./build_debug
```
> 💡 开发者可以根据需求修改编译选项，例如 Release 版本的编译选项应修改为`SGX_MODE=Release`与`SGX_HW=ON`。

## 运行一个 K-Means 示例
基于 Iris 数据集的 K-Means 聚类是机器学习中一个非常经典的学习示例，如下程序在 Fidelius 中实现了这个例子。
```
$ cd YeeZ-Privacy-Computing/test/integrate
$ python3 test_iris.py
```

## 进阶使用
- Fidelius 的 doxygen 文档：https://doc-fidelius.yeez.tech/index.html
- Fidelius 的 wiki 文档： https://doc-dianshu.yeez.tech/index.php/Fidelius
### 测试
如果开发者想在修改之后测试，可以使用如下命令生成一份测试报告 [CDash](https://my.cdash.org/index.php?project=Fidelius)
```
cd build_debug && ctest --dashboard Experimental
```

## 授权
`YeeZ-Privacy-Computing`库(即`toolkit`目录下的所有代码) 根据 [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) 获得许可，同时也包括我们存储库中的`COPYING.APACHE`文件。

`YeeZ-Privacy-Computing`二进制文件(即`toolkit`目录下的所有代码) 根据 [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) 获得授权，同时也包括我们存储库中的`COPYING`文件。

## 贡献
如果您想参与 Fidelius 项目，可以在 [Issue](https://github.com/YeeZTech/YeeZ-Privacy-Computing/issues) 页面随意开启一个新的话题，比如文档、创意、Bug等。
我们是一个开放共建的开源项目，欢迎参与到我们的项目中～

### 贡献者
<a href="https://github.com/YeeZTech/YeeZ-Privacy-Computing/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=YeeZTech/YeeZ-Privacy-Computing" />
</a>

## 社区
* 微信：

![wechat_helper](./wechat_image.JPG)

## Fidelius 企业版
这是一个社区版本。了解企业版更多信息，请联系`contact@yeez.tech`。
