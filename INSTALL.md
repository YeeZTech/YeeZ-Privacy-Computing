English / [中文](doc/INSTALL_ZH.md)


## Install Fidelius
### Prerequisites
- Ensure that you have the following required operating systems:
  * Ubuntu 20.04 LTS Server 64bits

- Ensure that you have SGX enable both in BIOS and CPU, follow the [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) in the [SGX-hardware](https://github.com/ayeks/SGX-hardware) project.

- Download [libsgx_enclave_common.so](https://download.01.org/intel-sgx/sgx-linux/2.13/distro/ubuntu20.04-server/debian_pkgs/libs/libsgx-enclave-common/libsgx-enclave-common_2.13.103.1-xenial1_amd64.deb) and [libsgx_urts.so](https://download.01.org/intel-sgx/sgx-linux/2.13/distro/ubuntu20.04-server/debian_pkgs/libs/libsgx-urts/libsgx-urts_2.13.103.1-xenial1_amd64.deb), then install.
```
$ sudo dpkg -i libsgx-enclave-common_2.13.103.1-xenial1_amd64.deb libsgx-urts_2.13.103.1-xenial1_amd64.deb
```

- To install the additional required tools by apt install (protobuf, glog, boost and cppconn for example)
```
$ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
$ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
$ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
```

### Install YeeZ Privacy Computing
- Download public key of YeeZ repository:
```
$ wget -O yeez-pkey.pem https://repo.yeez.tech/public-key-yeez.txt
```

- Add the public key:
```
$ sudo apt-key add yeez-pkey.pem
```

- Add YeeZ repository to software source:
```
$ echo -e "deb https://repo.yeez.tech/ release main" | sudo tee -a /etc/apt/sources.list
```

- Update software source and install Fidelius:
```
$ sudo apt update
$ sudo apt install secp256k1 fflib ypc-common ypc-core ykeymgr stbox-common-u ydump yterminus fid-datahub fid-analyzer example-iris
```


## Run Fidelius
### Example (Iris data as data source, run K-Means algorithm)
- 1. Prepare sealed data for analyzing.
```
$ data_provider --data-url /path/to/ypc/bin/iris.data --plugin-path /usr/local/lib/libiris_reader.so --sealed-data-url iris.sealed --output iris.sealed.output --sealer-path /usr/local/lib/edatahub.signed.so
```
**NOTE**: `--data-url` should be the correct path to iris data.

- 2. Generate a Secp256k1 key pair for data provider, the key is generated in `$HOME/.yeez.key/`.
```
$ keymgr_tool --create
```

- 3. Sign the hash of iris data using the Secp256k1 private key.
```
$ keymgr_tool --sign $DATA_HASH --sign.hex --sign.private-key $SEALED_PRIVATE_KEY
```
**NOTE**: `$DATA_HASH` is the value of `data_id` in file `iris.sealed.output` generated in step 1, and $SEALED_PRIVATE_KEY is the value of `private_key` in Secp256k1 key file generated in step 2.

- 4. Generate a Secp256k1 key pair for data consumer, note that data consumer can generate such key pair without SGX dependency.
```
$ yterminus --gen-key  --no-password  --output iris.key.json
```

- 5. Fetch the enclave hash of K-Means algorithm.
```
$ ydump --enclave /usr/local/lib/iris_parser.signed.so --output info.json
```

- 6. Encrpty analysis parameters.
```
$ yterminus --dhash $DATA_HASH --tee-pubkey $PROVIDER_PUBLIC_KEY --use-param 123 --param-format text --use-enclave-hash $ENCLAVE_HASH --output iris_param.json --use-privatekey-file iris.key.json
```
**NOTE**: `$DATA_HASH` is the value of `data_id` in file `iris.sealed.output` generated in step 1, `$PROVIDER_PUBLIC_KEY` is the value of `public_key` in Secp256k1 key file in step 2, and `$ENCLAVE_HASH` is the value of `enclave-hash` in file `info.json` in step 5.

- 7. Run K-Means algorithm using iris data.
```
$ GLOG_logtostderr=1 fid_analyzer --sealed-data-url iris.sealed --sealer-path /usr/local/lib/edatahub.signed.so --parser-path /usr/local/lib/iris_parser.signed.so --keymgr /usr/local/lib/keymgr.signed.so --source-type json --param-path iris_param.json --result-path iris.result.encrypted --check-data-hash $DATA_HASH
```
**NOTE**: `$DATA_HASH` is the value of `data_id` in file `iris.sealed.output` generated in step 1.

- 8. Decrypt analysis result.
```
$ yterminus --decrypt-hex $ENCRYPTED_RESULT --use-privatekey-file iris.key.json --output result.output
```
**NOTE**: `$ENCRYPTED_RESULT` is the value of `encrypted-result` in file `iris.result.encrypted` generated in step 7.
