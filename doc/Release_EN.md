Compile and Install Fidelius
------------------------
### Dependency
- Make sure you have the following operating system:
  * Ubuntu 20.04 LTS Server 64bits

- Ensure that SGX is enabled in BIOS and CPU. Follow the [README](https://github.com/ayeks/SGX-hardware/blob/master/README.md) in [SGX-hardware](https://github.com/ayeks/SGX-hardware) or use the following method to confirm directly.
```
 $ git clone --depth 1 --branch ayeks-patch-1 https://github.com/ayeks/SGX-hardware.git
 $ cd SGX-hardware
 $ gcc test-sgx.c -o test-sgx
 $ ./test-sgx
```
If the following is included, then the hardware configuration for Intel SGX is compliant.
```
...
sgx available: 1
sgx launch control: 1

...
sgx 1 supported: 1
sgx 2 supported: 0
...
```
If the "sgx available" field is 0, it means that the CPU itself does not support Intel SGX; if the "sgx 1 supported" field is 0, it indicates that the BIOS does not support or has not enabled the Intel SGX feature. To be able to execute programs that use Intel SGX, you also need to correctly install the Intel SGX SDK in the next step.

- Compile and install the `Intel(R) SGX` driver, `SDK`, and `PSW` according to the instructions in the [README](https://github.com/intel/linux-sgx/blob/master/README.md) document of the linux-sgx project.
Note: It is not possible to install the `PSW` if the hardware environment does not support SGX.

- Install dependencies (protobuf，glog，boost，cppconn and etc):
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### Compile and Install Fidelius
```
$ git clone --depth 1 https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
$ cd YeeZ-Privacy-Computing && ./build.sh compile-project $COMPILE_MODE
```
**Note**: Set `$COMPILE_MODE` to "debug" for "sgx available" is 0, or set `$COMPILE_MODE` to "prerelease" for "sgx available" is 1.

Run Fidelius
------------------------
### Example (Use the Iris dataset as the data source to run the K-Means algorithm)
The K-Means clustering based on the Iris dataset is a very classic learning example in machine learning.
- Download example code:
```
$ git clone --depth 1 https://github.com/YeeZTech/YPC-algo-example.git
```

- Compile example code:
```
$ cd YPC-algo-example && mkdir -p build && cd build
$ cmake -DCMAKE_PREFIX_PATH=$YPC_INSTALL_DIR/lib/cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
$ make -j8
```
**Note**: `$YPC_INSTALL_DIR` is the installation path, which by default is `$HOME`. Set `$BUILD_TYPE` to "Debug" for debug mode, or set `$BUILD_TYPE` to "RelWithDebInfo" for prerelease/release mode.

- Run example code:
```
$ cd YPC-algo-example/integrate && python3 test_iris.py
```

- Getting the following results indicates that the sample code has run successfully:
```
$ result is :  ['0 -  (5.004082 ,3.416327 ,1.465306 ,0.244898) \n', '1 -  (5.883607 ,2.740984 ,4.388525 ,1.434426) \n', '2 -  (6.864864 ,3.067567 ,5.735135 ,2.059460) \n']
```

