中文 / [English](../INSTALL.md)


安装 Fidelius
------------------------
### 环境依赖

- 确保拥有如下操作系统：

  * Ubuntu 20.04 LTS Server 64bits

- 确保 BIOS 和 CPU 启用 SGX，请遵循 [SGX-hardware](https://github.com/ayeks/SGX-hardware) 中的 [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) 。

- 下载 [libsgx_enclave_common.so](https://download.01.org/intel-sgx/sgx-linux/2.13/distro/ubuntu20.04-server/debian_pkgs/libs/libsgx-enclave-common/libsgx-enclave-common_2.13.103.1-xenial1_amd64.deb) 和 [libsgx_urts.so](https://download.01.org/intel-sgx/sgx-linux/2.13/distro/ubuntu20.04-server/debian_pkgs/libs/libsgx-urts/libsgx-urts_2.13.103.1-xenial1_amd64.deb)，并安装。
```
$ sudo dpkg -i libsgx-enclave-common_2.13.103.1-xenial1_amd64.deb libsgx-urts_2.13.103.1-xenial1_amd64.deb
```

- 安装依赖工具（protobuf，glog，boost，cppconn等）：
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### 安装
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
$ sudo apt install secp256k1 fflib ypc-common ypc-core ykeymgr stbox-common-u ydump yterminus fid-datahub fid-analyzer example-iris
```


运行 Fidelius
------------------------
### 示例（使用 iris 数据集作为数据源，运行 K-Means 算法）
- 1. 准备 iris 数据集的封存数据
```
$ data_provider --data-url /path/to/ypc/bin/iris.data --plugin-path /usr/local/lib/libiris_reader.so --sealed-data-url iris.sealed --output iris.sealed.output --sealer-path /usr/local/lib/edatahub.signed.so
```
**注意**:`--data-url`为 iris 数据集的文件路径。

- 2. 为数据提供方生成 Secp256k1 密钥对，密钥对生成在`$HOME/.yeez.key/`路径下
```
$ keymgr_tool --create
```

- 3. 数据提供方使用 Secp256k1 的私钥签名 iris 数据集的数据哈希
```
$ keymgr_tool --sign $DATA_HASH --sign.hex --sign.private-key $SEALED_PRIVATE_KEY
```
**注意**:`$DATA_HASH`为步骤1中输出文件（`iris.sealed.output`）中的`data_id`字段，$SEALED_PRIVATE_KEY 为步骤2中密钥对文件的`private_key`字段。

- 4. 为数据使用方生成 Secp256k1 密钥对，数据使用方可以不依赖于 SGX 环境生成密钥对
```
$ yterminus --gen-key  --no-password  --output iris.key.json
```

- 5. 获取 K-Means 算法的算法哈希
```
$ ydump --enclave /usr/local/lib/iris_parser.signed.so --output info.json
```

- 6. 加密 K-Means 算法的输入参数
```
$ yterminus --dhash $DATA_HASH --tee-pubkey $PROVIDER_PUBLIC_KEY --use-param 123 --param-format text --use-enclave-hash $ENCLAVE_HASH --output iris_param.json --use-privatekey-file iris.key.json
```
**注意**:`$DATA_HASH`为步骤1中输出文件（`iris.sealed.output`）中的`data_id`字段， `$PROVIDER_PUBLIC_KEY`为步骤2中密钥对文件的`public_key`字段，`$ENCLAVE_HASH`为步骤5中输出文件`info.json`的`enclave-hash`字段。

- 7. 在 iris 数据集上运行 K-Means 算法
```
$ GLOG_logtostderr=1 fid_analyzer --sealed-data-url iris.sealed --sealer-path /usr/local/lib/edatahub.signed.so --parser-path /usr/local/lib/iris_parser.signed.so --keymgr /usr/local/lib/keymgr.signed.so --source-type json --param-path iris_param.json --result-path iris.result.encrypted --check-data-hash $DATA_HASH
```
**注意**:`$DATA_HASH`为步骤1中输出文件（`iris.sealed.output`）中的`data_id`字段。

- 8.解密分析结果
```
$ yterminus --decrypt-hex $ENCRYPTED_RESULT --use-privatekey-file iris.key.json --output result.output
```
**注意**:`$ENCRYPTED_RESULT`为步骤7中输出文件`iris.result.encrypted`的`encrypted-result`字段。
