编译 Fidelius
------------------------
### 软件依赖
Fidelius Debug 版本不对硬件有特殊要求，但需要安装软件依赖。

- 确保拥有如下操作系统：
  * Ubuntu 18.04+ LTS Server 64bits

- 根据具体操作系统，安装 [SGX SDK](https://download.01.org/intel-sgx/latest/linux-latest/distro/)。

- 安装依赖工具（protobuf，glog，boost，cppconn等）：
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### 编译 Fidelius
```
$ git clone --depth 1 --branch v0.5.1 https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
$ cd YeeZ-Privacy-Computing && ./build.sh compile-project debug
```

运行 Fidelius
------------------------
### 示例（使用 Iris 数据集作为数据源，运行 K-Means 算法）
基于 Iris 数据集的 K-Means 聚类是机器学习中一个非常经典的学习示例。

- 运行示例代码：
```
$ cd YeeZ-Privacy-Computing/test/integrate
$ python3 test_iris.py
```

- 得到以下结果，表示示例代码运行成功：
```
$ result is :  ['0 -  (5.004082 ,3.416327 ,1.465306 ,0.244898) \n', '1 -  (5.883607 ,2.740984 ,4.388525 ,1.434426) \n', '2 -  (6.864864 ,3.067567 ,5.735135 ,2.059460) \n']
```
