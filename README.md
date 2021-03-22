Fidelius - YeeZ Privacy Computing
================================================
Introduction
------------
In order to empower data collaboration between enterprises and help enterprises use data to enhance their core competitiveness, we launched Fidelius, a privacy protection solution for data collaboration. Fidelius is based on the idea of "data is available but not visible", while effectively ensuring the consistency of the original data, the controllability of the calculation logic, the correctness and privacy of the calculation results.

The following figure describes the abstract flow of data collaboration based on Fidelius. Similar to the traditional data collaboration model, the participants include data providers and data consumers. Fidelius runs on the side of both data provider and data consumer, and the two parties interact with Fidelius to realize data collaboration operation. There is no direct data interaction between the data provider and the data consumer, and the raw/plaintext data will not leave the data provider, which fundamentally avoids the problem of private data leakage.

![](doc/Fidelius-Infr.png)

It is worth noting that, compared to the traditional data collaboration model, Fidelius has introduced blockchain. Because blockchain itself has the characteristics of a decentralized network, public and verifiable, Fidelius uses it as a trusted transmission channel and data calculation verification platform.

**NOTE**: this is a community version. Although it shares similar components with our enterprise version, it has different features. Please contract `contact@yeez.tech` for more details about the enterprise version.

Documentation
-------------
- [Fidelius: YeeZ Privacy Protection for Data Collaboration - A Blockchain based Solution](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)


Build Fidelius
-------------------------------------------------------
We provide two kinds of building modes of Fidelius, debug mode and release mode. Please follow the guidance (i.e., this document) for building with debug mode. Fidelius compiled under the release mode will not work until the developer completes the production licensing process. If you would like to deliver a production-quality Fidelius, please contact the `sgx_program@intel.com` for more information about a production license. Or you can contact `contact@yeez.tech` for convenience without applying for production license by yourself.

### Prerequisites
- Ensure that you have the following required operating systems:
  * Ubuntu\* 18.04 LTS Server 64bits

- Ensure that you have SGX enable both in BIOS and CPU, follow the [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) in the [SGX-hardware](https://github.com/ayeks/SGX-hardware) project.

- Follow the [README.md](https://github.com/intel/linux-sgx/blob/master/README.md) in the [linux-sgx](https://github.com/intel/linux-sgx) project to build and install the Intel(R) SGX driver, SDK and PSW.

- To install the additional required tools by apt install (protobuf, glog, boost, cppconn and etc.):
    ```
    $ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
    $ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
    $ sudo apt insatll mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
    ```

- To install the additional required tools from source (cryptopp):
    ```
    $ git clone https://github.com/weidai11/cryptopp
    $ cd cryptopp && make -j8 && sudo make install
    ```

- To install Ethereum smart contract environment dependencies: [Solidity](https://docs.soliditylang.org/en/v0.8.2/installing-solidity.html) and [Truffle](https://www.trufflesuite.com/).
    ```
    $ npm install -g solc
    $ npm install -g truffle
    ```


### Build YeeZ Privacy Computing (From Source Code)
- Download YeeZ Privacy Computing source code:
    ```
    $ git clone https://gitlab.com/Yeez/yeez-privacy-computing.git
    ```

- Build and install submodule dependencies:
    ```
    $ git submodule update --init
    $ cd yeez-privacy-computing/vendor/fflib && mkdir build && cd build && cmake .. && make
    ```

- To build YeeZ Privacy Computing with debug mode, enter the following command:
    ```
    $ cmake -DSGX_MODE=Debug -DSGX_HW=OFF .. && make -j8
    ```
    You can find libraries and binaries in directory `yeez-privacy-computing/lib` and `yeez-privacy-computing/bin` respectively.

- To building and deploy Ethereum smart contract, please enter the command:
    ```
    $ cd yeez-privacy-computing/contracts/backend && truffle migrate --network ropsten
    ```
    We use Etherum as underlying blockchain system, we deploy related smart contract to Ethereum testnet, i.e., [Ropsten](https://ropsten.etherscan.io/). The details of smart contract is located in the directory `yeez-privacy-computing/contracts`.


Run Fidelius
------------------------
According to the building mode of Fidelius, Fidelius can run under debug mode and release mode. Please follow the guidance (i.e., this document) for running with debug mode; and contact `sgx_program@intel.com` for applying for production license to run with release mode, or contact `contact@yeez.tech` for convenience without applying for production license by yourself.

As mentioned in Fidelius introduction, there are two roles in Fidelius system, i.e., data provider and data consumer, so to run Fidelius, we specify on which side the command executes.

### Initialization
The steps of initializing Fidelius is as follows:
1) (Data Provider) Create database.
    ```
    mysql> create database ypcd;
    ```

2) (Data Provider) Create configuration file with name `ypcd.conf` at directory `yeez-privacy-computing/bin`, the content of the file is as follows, and replace `$USER_NAME` and `$USER_PASSWD` with your own name and password respectively.
    ```
    [mysql]
    url = tcp://127.0.0.1:3306
    usr-name = $USER_NAME
    usr-password = $USER_PASSWD
    database = ypcd

    [net]
    control-port = 7068
    ```

3) (Data Provider) Initialize `ypcd` database.
    ```
    $ cd yeez-privacy-computing/bin && ./ypcd --init
    ```

4) (Data Provider) Start Yeez Privacy Computing Daemon (ypcd).
    ```
    $ ./ypcd --start
    ```

5) (Data Provider and Data Consumer) Create two pairs of public/private keys for data proviser and data consumer respectively, key pairs are created in directory `$HOME/.yeez.key/`.
    ```
    $ ./keymgr_tool --create  # for data provider
    $ ./keymgr_tool --create  # for data consumer
    ```

6) (Contract Deployer) Register pair `$PROVIDER_ADDR`,`$PROVIDER_PKEY` and `$CONSUMER_ADDR`,`$COMSUMER_PKEY` to smart contract by calling contract function `register(address,bytes)`.
    ```
    contract address: 0x45464EbC79186AA313A5c01D7E4447422ba36c97
    contract function: register(address addr, bytes pkey)
    ```
    **NOTE**: This address is our deployed contract on Ropsten, and only authorized owner can call this function. You may want to deploy your own contracts.


### Example (Iris data as data source, run K-Means algorithm)
We take K-Means algorithm running Iris data as an example of data collaboration.

7) (Data Provider) Seal data source (raw/plaintext data), the output file `sealed.output` contains the data hash (`data_id`) of raw/plaintext data.
    ```
    $ ./data_provider --data-url ./iris.data --sealed-data-url ./iris.data.sealed --sealer-path ../lib/edatahub.signed.so --plugin-path ../lib/libiris_reader.so --output sealed.output
    ```

8) (Data Provider) Register iris data to `ypcd`.
    ```
    $ ./ypcshuttle --onchain --input=iris_data.ini
    ```
    **NOTE**: The content of file `iris_data.ini` is as follows, please replace `$PATH_TO_YEEZ_PRIVACY_COMPUTING` with your own `yeez-privacy-computing` directory, and `$data_id` with `data_id` which outputs in last step (file `sealed.output`).
    ```
    [general]
    mode = file

    [data]
    sealed_data_url = $PATH_TO_YEEZ_PRIVACY_COMPUTING/bin/iris.data.sealed

    [blockchain]
    data_id = $data_id
    data_desc = Iris Data

    [exec]
    parser_path = $PATH_TO_YEEZ_PRIVACY_COMPUTING/lib/iris_parser.signed.so
    params = params here
    ```

9) (Data Provider) Register iris data to smart contract (create a new contract).
    ```
    contract address: 0x4be440d8af3c2AbD7b6aE1eF6176b8C516f360B7
    contract function: createYZData(bytes32 hash, string name, string desc, bytes sample,
                                    string env_info, uint price, address cert_addr,
                                    address program_proxy)
    ```
    Where function parameter `hash` is iris data hash `$data_id` generated in step 7, parameter `name`/`desc`/`sample`/`env_info` describe the meta information of iris data, parameter `price` is the cost using this dataset for analysis, `cert_addr` and `program_proxy` is the contract address in building step.

    **NOTE**: Transaction sender should be `$PROVIDER_ADDR`.

10) (Data Provider) Get data contract and data request contract by checking the event logs of `createYZData` transaction. For example, a successful [create data transaction](https://ropsten.etherscan.io/tx/0x414ce1c2e1d89befa6e06cdb66389fedbdd3c8993d1545e070d15b6955a5eea3) has three [event logs](https://ropsten.etherscan.io/tx/0x414ce1c2e1d89befa6e06cdb66389fedbdd3c8993d1545e070d15b6955a5eea3#eventlog), which contain a [NewYZData contract](https://ropsten.etherscan.io/address/0xf9c4ce0eab21e5842fd8fb4a7501b95b85507a4a) and a [NewYZDataRequest contract](https://ropsten.etherscan.io/address/0xe47667668fdc98eff6a690547a6399d875407b7d). After that, copy these two address to `$contract_YZData` and `$contract_YZDataRequest` in file `yeez-privacy-computing/toolkit/blockchain/ethereum/common/const.py`.

11) (Data Provider) Start request data listener daemon.
    ```
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/ && pip3 install -r requirements.txt
    $ export MYSQL_URL=$MYSQL_URL MYSQL_USERNAME=$MYSQL_USERNAME MYSQL_PASSWORD=$MYSQL_PASSWORD YPCD_DB=$YPCD_DB
    $ export YPC_HOME=$PATH_TO_YEEZ_PRIVACY_COMPUTING SUBMITTER_PASSWD=$YPC_HOME
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/common && python3 db_tables.py
    $ python3 daemon.py --host $HOST --project_id $PROJECT_ID
    ```
    Where `$PATH_TO_YEEZ_PRIVACY_COMPUTING` is the path to `yeez-privacy-computing`.

    Replace `$MYSQL_URL`/`$MYSQL_USERNAME`/`$MYSQL_PASSWORD`/`$YPCD_DB` with your own values, usually `$MYSQL_URL` is set to `127.0.0.1:3306`, and `YPCD_DB` is set to `ypcd`.

    `$SUBMITTER_PASSWD` is used to decrypt a raw/plaintext private key using Ethereum keystore to send transaction of analysis result.

    `$HOST` in last command line indicates Ethereum network name, so it is set to `ropsten`, while `$PROJECT_ID` is given by [Infura](https://infura.io/) if you create a project on it.

12) (Data Consumer) First of all, data consumer can explore the data set information via blockchain. Say a data consumer learned the IRIS data, and want to use it for a k-means analysis, he/she needs a parser, named `iris_parser.signed.so` here. He/she also needs to upload it to a server so that anyone can access it via a downloadable URL, and register the information of `iris_parser.signed.so` to smart contract by calling contract function `upload_program(string,string,string,uint256,bytes32)`.
    ```
    contract address: 0xBC8214F07a3253091aE8fb0A01079ea7B4955768
    contract function: upload_program(string name, string desc, string url, uint256 price, bytes32 enclave_hash)
    ```
    Where function parameter `url` is given after uploading program to server, parameter `price` is the cost using this program, parameter `enclave_hash` is given in file `dump.out` by entering the following command:
    ```
    $ sgx_sign dump -enclave iris_parser.signed.so --dumpfile dump.out
    ```

    **NOTE**: Transaction sender should be `$CONSUMER_ADDR`.

13) (Data Consumer) Prepare request information such as `encrypted-input`, `program-enclave-hash` and etc. for data analysis, please replace `$data_id` and `$CONSUMER_PKEY` generated in step 7 and step 5.
    ```
    $ ./yprepare --dhash=$data_id --use-pubkey=$CONSUMER_PKEY --use-param xxx --param-format text --use-enclave ../lib/iris_parser.signed.so --output params.json
    ```

14) (Data Consumer) Request data for analysis.
    ```
    contract address: contract_YZDataRequest
    contract function: request_data(bytes secret，bytes input, bytes forward_sig,
                                    bytes32 program_hash, uint gas_price)
    ```
    Where `contract_YZDataRequest` is generated in step 10. Parameters `secret`/`input`/`forward_sig` is specified with `$encrypted-skey`/`encrypted-input`/`forward-sig` respectively in file `params.json` at step 13. Parameter `program_hash` is generated in transaction event logs at step 12.

    On the data provider side, request data listener daemon in step 11 will monitor the transaction, download `iris_parser.signed.so` and run data analysis program automatically. After finishing analyzing, analysis result will be automatically sent to blockchain smart contract.

    **NOTE**: Transaction sender should be `$CONSUMER_ADDR`. Configuration file `$PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/transaction/config/YZDataRequest.json` should be revised, including the items of `$keystore_path`, `$sender` and `$contract_address`, where `$keystore_path` is Ethereum keystore that can be decrypted by `$SUBMITTER_PASSWD` at step 11. `$sender` is set to `$CONSUMER_ADDR`, and `$contract_address` is set to `$contract_YZDataRequest`.

15) (Data Consumer) Decrypt result. Encrypted analysis results are automatically sent to blockchain smart contract, data consumer can decrypt the result by entering the following command:
    ```
    $ ./keymgr_tool --decrypt $encrypted-result --decrypt.private-key $CONSUMER_SEALED_KEY
    ```


License
-------
The yeez-privacy-computing library (i.e. all code outside of the `toolkit` directory) is licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0), also included in our repository in the `COPYING.APACHE` file.

The yeez-privacy-computing binaries (i.e. all code inside of the `toolkit` directory) is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html), also included in our repository in the `COPYING` file.
