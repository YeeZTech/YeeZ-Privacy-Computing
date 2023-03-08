Compile Fidelius
------------------------
### Dependency
Fidelius Debug version does not have special hardware requirements, but requires installation of software dependencies.

- Make sure you have the following operating system:
  * Ubuntu 18.04+ LTS Server 64bits

- Install [SGX SDK](https://download.01.org/intel-sgx/latest/linux-latest/distro/) according to the specific operating system

- Install dependencies（protobuf，glog，boost，cppconn and etc）：
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### Compile Fidelius
```
$ git clone --depth 1 --branch v0.5.1 https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
$ cd YeeZ-Privacy-Computing && ./build.sh compile-project debug
```

Run Fidelius
------------------------
### Example（Use the Iris dataset as the data source to run the K-Means algorithm）
The K-Means clustering based on the Iris dataset is a very classic learning example in machine learning.

- Run example code：
```
$ cd YeeZ-Privacy-Computing/test/integrate
$ python3 test_iris.py
```

- Getting the following results indicates that the sample code has run successfully：
```
$ result is :  ['0 -  (5.004082 ,3.416327 ,1.465306 ,0.244898) \n', '1 -  (5.883607 ,2.740984 ,4.388525 ,1.434426) \n', '2 -  (6.864864 ,3.067567 ,5.735135 ,2.059460) \n']
```

