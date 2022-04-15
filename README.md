English / [中文](doc/README_ZH.md)

# Fidelius - YeeZ Privacy Computing
## Introduction
In order to empower data collaboration between enterprises and help enterprises use data to enhance their core competitiveness, we launched Fidelius, a privacy protection solution for data collaboration. Fidelius is based on the idea of "data is available but not visible", while effectively ensuring the consistency of the original data, the controllability of the calculation logic, the correctness and privacy of the calculation results.

The following figure describes the abstract flow of data collaboration based on Fidelius. Similar to the traditional data collaboration model, the participants include data providers and data consumers. Fidelius runs on both sides of data provider and data consumer, and the two parties interact with Fidelius to realize data collaboration operation. There is no direct data interaction between the data provider and the data consumer, and the raw/plaintext data will not leave the data provider, which fundamentally avoids the problem of private data leakage.

![](doc/Fidelius-Infr.png)

It is worth noting that, compared to the traditional data collaboration model, Fidelius has introduced blockchain. Because blockchain itself has the characteristics of a decentralized network, public and verifiable, Fidelius uses it as a trusted transmission channel and data calculation verification platform.

**NOTE**: This is a community version. Although it shares similar components with our enterprise version, it has different features. Please contact `contact@yeez.tech` for more details about the enterprise version.

## Documentation
- [Fidelius: YeeZ Privacy Protection for Data Collaboration - A Blockchain based Solution](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)

## Compile Fidelius
### Prerequisites
- Ensure that you have the following required operating systems:
  * Ubuntu 20.04 LTS Server 64bits

- Ensure that you have SGX enable both in BIOS and CPU, follow the [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) in the [SGX-hardware](https://github.com/ayeks/SGX-hardware) project.

- Follow the [README.md](https://github.com/intel/linux-sgx/blob/master/README.md) in the [linux-sgx](https://github.com/intel/linux-sgx) project to build and install the Intel(R) SGX driver, SDK and PSW.

- To install the additional required tools by apt install (protobuf, glog, boost and cppconn for example)
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### Compile From Source
```
$ git clone https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
$ git submodule update --init
$ cd YeeZ-Privacy-Computing && mkdir build
$ cmake -DSGX_MODE=Debug -DSGX_HW=OFF ../
$ make -j8
```
**NOTE:** To compile release version, the developer should set `SGX_MODE=Release`and`SGX_HW=ON`.

## Install Fidelius
Please refer to [INSTALL.md](INSTALL.md).

## License
The `YeeZ-Privacy-Computing` library (i.e. all code outside of the `toolkit` directory) is licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0), also included in our repository in the `COPYING.APACHE` file.

The `YeeZ-Privacy-Computing` binaries (i.e. all code inside of the `toolkit` directory) is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html), also included in our repository in the `COPYING` file.
