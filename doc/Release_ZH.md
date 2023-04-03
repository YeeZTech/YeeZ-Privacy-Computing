安装 Fidelius
------------------------
### 环境依赖

- 确保拥有如下操作系统：

  * Ubuntu 20.04 LTS Server 64bits

- 确保 BIOS 和 CPU 启用 SGX，请遵循 [SGX-hardware](https://github.com/ayeks/SGX-hardware) 中的 [README](https://github.com/ayeks/SGX-hardware/blob/master/README.md) 。或直接使用如下方式进行确认：
```
 $ git clone https://github.com/ayeks/SGX-hardware.git
 $ cd SGX-hardware
 $ gcc test-sgx.c -o test-sgx
 $ ./test-sgx
```
若其中包括如下，则 Intel SGX 的硬件配置是符合的。
```
...
sgx available: 1
sgx launch control: 1

...
sgx 1 supported: 1
sgx 2 supported: 0
...
```
若 sgx available 字段为 0，则 CPU 本身不支持 Intel SGX；若 sgx 1 supported 字段为0，则说明 BIOS 不支持或未开启 Intel SGX 功能。为了能够执行 Intel SGX 的程序，还需要按照下一步正确安装 Intel SGX SDK。

- 按照 linux-sgx 项目中 [README](https://github.com/intel/linux-sgx/blob/master/README.md) 文档进行编译并安装`Intel(R) SGX`驱动`SDK`和`PSW`。注意：硬件环境不支持 SGX 的情况下无法安装`PSW`。


- 安装依赖工具（protobuf，glog，boost，cppconn等）：
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### 安装 Fidelius 组件
- 下载熠智科技 apt 仓库的公钥：
```
$ wget -O yeez-pkey.pem https://repo.yeez.tech/public-key-yeez.txt
```

- 将下载的仓库公钥添加进系统：
```
$ sudo apt-key add yeez-pkey.pem
```

- 将熠智科技 apt 仓库添加进系统软件源：
```
$ echo -e "deb https://repo.yeez.tech/ release main" | sudo tee -a /etc/apt/sources.list
```

- 更新系统软件源并安装 Fidelius 组件：
```
$ sudo apt update
$ sudo apt install ypc fflib
```


运行 Fidelius
------------------------
### 示例（使用 Iris 数据集作为数据源，运行 K-Means 算法）
基于 Iris 数据集的 K-Means 聚类是机器学习中一个非常经典的学习示例。
- 下载示例代码：
```
$ git clone --depth 1 --branch v0.5.1 https://github.com/YeeZTech/YPC-algo-example.git
```

- 编译示例代码：
```
$ cd YPC-algo-example && mkdir -p build && cd build
$ cmake -DCMAKE_PREFIX_PATH=$YPC_INSTALL_DIR/lib/cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
$ make -j8
```
**注意**:`$YPC_INSTALL_DIR`为 Fidelius 组件安装路径，默认情况下为系统路径`/opt/ypc`。

- 运行示例代码：
```
$ cd YPC-algo-example/integrate && python3 test_iris.py
```

- 得到以下结果，表示示例代码运行成功：
```
$ result is :  ['0 -  (5.004082 ,3.416327 ,1.465306 ,0.244898) \n', '1 -  (5.883607 ,2.740984 ,4.388525 ,1.434426) \n', '2 -  (6.864864 ,3.067567 ,5.735135 ,2.059460) \n']
```
