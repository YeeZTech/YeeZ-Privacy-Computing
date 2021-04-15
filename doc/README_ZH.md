中文 / [English](../README.md)

Fidelius - 熠智隐私计算中间件
=====================================



## 引言

为了赋能企业间的数据合作，助力企业利用数据提升自身核心竞争力，熠智科技推出了面向数据合作的一站式隐私保护解决方案 Fidelius。Fidelius 基于“数据可用不可见”思想，同时有效保证了原始数据的一致性、计算逻辑的可控性、计算结果的正确性及隐私性。

下图描述了基于 Fidelius 实现数据合作的抽象流程。与传统的数据合作模式类似，参与方包括了数据提供方和数据使用方。Fidelius 件分别运行在数据提供方和数据使用方中，双方通过与 Fidelius 交互实现数据合作操作。数据提供方和数据使用方之间没有直接的数据交互，并且原始数据不会离开数据提供方的 Fidelius 中间件，这从根本上避免了隐私数据泄露的问题。

![](Fidelius-Infr.png)

值得注意的是，相比传统的数据合作模式，Fidelius 引入了区块链网络。由于区块链本身具有去中心化网络、公开可验证等特性，Fidelius 将其作为可信的传输通道和数据计算验证平台。

**注意：** 这是一个社区版本。尽管它与我们的企业版共享相似的组件，但它有不同的特性。了解企业版更多信息，请联系`contact@yeez.tech`。



## 文档

- [Fidelius: YeeZ Privacy Protection for Data Collaboration - A Blockchain based Solution](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)



## 编译 Fidelius

我们提供了两种 Fidelius 的编译模式，Debug 模式和 Release 模式。Debug 模式请按照本指导进行编译，Release 模式下编译 Fidelius 需要开发者事先完成产品授权，否则将无法正常工作。由于Fidelius 中使用到了英特尔公司的SGX，因此如果想使用 Release 版本的 Fidelius，请查看英特尔SGX[开发文档](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions/attestation-services.html)，或者也可以通过联系`contact@yeez.tech`取得帮助。



### 环境依赖

- 确保拥有如下操作系统：

  - Ubuntu\* 18.04 LTS Server 64bits

- 确保 BIOS 和 CPU 启用 SGX，请遵循  [SGX-hardware](https://github.com/ayeks/SGX-hardware) 中的 [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) 。

- 按照 [linux-sgx](https://github.com/intel/linux-sgx) 项目中 [README.md](https://github.com/intel/linux-sgx/blob/master/README.md) 文档进行编译并安装 Intel(R) SGX 驱动、SDK 和 PSW。

- 通过apt方式安装依赖工具(protobuf, glog, boost, cppconn等)：

  ```shell
  $ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
  $ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
  $ sudo apt insatll mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
  ```

- 通过源码安装 (cryptopp):

  ```shell
  $ git clone https://github.com/weidai11/cryptopp
  $ cd cryptopp && make -j8 && sudo make install
  ```

- 安装以太坊智能合约环境的依赖工具: [Solidity](https://docs.soliditylang.org/en/v0.8.2/installing-solidity.html) and [Truffle](https://www.trufflesuite.com/).

  ```
  $ npm install -g solc
  $ npm install -g truffle
  ```

- 确保拥有一个以太网测试网络 `Ropsten` 上的账户，并且该账户至少有 1 个 ETH。账户可以用 MateMask 钱包生成，ETH 可以访问[该网址](https://faucet.ropsten.be/)申请得到。

- 确保拥有一个 `INFURA_API_KEY`，可以通过 [Infura](https://infura.io/) 网站获取。

- 确保拥有一个 `ETHERSACAN_API_KEY`, 可以通过 [EtherScan](https://ropsten.etherscan.io/) 网站获取。

### 编译(源码编译)

- 下载 Fidelius 源码：

  ```shell
  $ git clone https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
  ```

- 编译并安装其依赖的子模块：

  ```shell
  $ git submodule update --init
  $ cd YeeZ-Privacy-Computing/vendor/fflib && mkdir build && cd build && cmake .. && make
  ```

- 以Debug 模式编译 Fidelius，执行命令：

  ```shell
  $ cd YeeZ-Privacy-Computing
  $ mkdir -p build & cd build
  $ cmake -DSGX_MODE=Debug -DSGX_HW=OFF .. && make -j8
  ```

  编译成功后可以在 `YeeZ-Privacy-Computing/lib` 和 `YeeZ-Privacy-Computing/bin` 目录下中找到库文件和二进制文件。

- 编写部署智能合约所需的配置文件。在 `YeeZ-Privacy-Computing/contracts/backend` 目录下创建一个 `.env` 文件，其内容如下：

  ```shell
  ROPSTEN_PK = $Private_key
  ETHERSCAN_API_KEY = $EtherScan_API_KEY
  INFURA_API_KEY = $Infura_API_KEY
  ```

  - `$Private_key` 是用户使用 `MateMask` 生成的钱包的私钥。
  - `$EtherScan_API_KEY` 是在  [EtherScan](https://ropsten.etherscan.io/) 网站获取的 API Key。
  - `$Infura_API_KEY` 是在  [Infura](https://infura.io/) 网站获取的 API Key 。

  **注意：** 上述参数用户不可直接使用，需要用户替换为自己的参数。

- 安装部署智能合约所需的依赖  ` truffle-privatekey-provider` 和 `dotenv` , 命令如下：

  ```shell
  cd YeeZ-Privacy-Computing/contracts/backend 
  npm install  truffle-privatekey-provider
  npm install dotenv
  ```

- 编译和部署以太坊智能合约，使用命令：

  ```shell
  $ cd YeeZ-Privacy-Computing/contracts/backend && truffle migrate --network ropsten
  ```

  **注意：** 上述命令执行完毕后，用户需要记录 4 个智能合约的地址以方便后续调用。

  **注意：** 上述命令执行完毕之后将部署 4 个智能合约，分别是 `CertifiedUsers` 、`ProgramStore`、`YZDataRequestFactory` 和 `YZDataFactory`。

  - `CertifiedUsers` 合约用于存储数据提供方和数据分析者的注册信息。

  - `ProgramStore` 合约用于存储数据分析者的分析程序的元信息(程序名称、描述信息、下载地址等)。

  - `YZDataRequestFactory` 合约用于存储数据提供方的数据元信息(数据哈希，数据名称、数据描述、数据样例、数据价格等)。

  - `YZDataFactory` 合约用于存储数据分析者对数据提供方数据的请求信息(程序哈希、程序输入参数等),并处理数据合作请求并完成最终结算。

  

  我们使用以太坊作为底层的区块链系统，并将相关智能合约部署到以太坊测试网络，即 [Ropsten](https://ropsten.etherscan.io/)。

  具体的智能合约存储在 `YeeZ-Privacy-Computing/contracts` 目录下。

  

## 运行 Fidelius

**根据编译模式，Fidelius 可以在Debug和Release模式下运行。Debug 模式请按照本指导进行编译，以 Release 模式运行，请查看英特尔SGX[开发文档](baidu.com)，或者也可以通过联系`contact@yeez.tech`取得帮助。**

Fidelius 介绍中提到该系统中有两个角色，即数据提供方和数据分析者。因此运行 Fidelius 时，我们会指定命令在哪一方执行。

### 初始化

Fidelius 初始化步骤如下：

1. (数据提供方) 创建数据库。

   ```sql
   mysql> create database ypcd;
   ```

2. (数据提供方) 在 `Yeez-Privacy-Computing/bin` 目录下创建配置文件 `ypcd.conf`, 文件内容如下所示。其中`$USER_NAME` 和 `$USER_PASSWD` 需要用户替换为自己的用户名和密码。

   ```shell
   [mysql]
       url = tcp://127.0.0.1:3306
       usr-name = $USER_NAME
       usr-password = $USER_PASSWD
       database = ypcd
   [net]
   control-port = 7068
   ```

3. (数据提供方) 初始化 `ypcd` 数据库。

   ```shell
   $ cd YeeZ-Privacy-Computing/bin && ./ypcd --init 
   ```

4. (数据提供方) 启动 Fidelius 守护进程 (ypcd)。

   ```shell
   ./ypcd --start
   ```

5. (数据提供方/数据分析者) 创建一对公私钥对，创建完成后存储在  `$HOME/.yeez.key/` 目录下, 公私钥以json文件格式进行存储。

   ```shell
    $ ./keymgr_tool --create  # for data provider
    $ ./keymgr_tool --create  # for data consumer
   ```

6. (数据提供方/数据分析者)注册公钥 。通过调用 `register(address, bytes)` 函数将公钥注册到智能合约。

   ```shell
   contract address: 0x45464EbC79186AA313A5c01D7E4447422ba36c97
   contract function: register(address addr, bytes pkey)
   ```

   - `addr` 是调用的合约地址。

   - `pkey` 是公钥,。

   **注意1：** 上述智能合约作为数据提供方和数据分析者进行数据合作的可信第三方，协助完成数据计算服务以及费用结算，详细信息请参看其源码和示例。

   **注意2：** 上述合约地址是我们部署到 `Ropsten` 的合约地址，只有授权用户才能调用该函数，如果想使用该合约地址，请联系 `contact@yeez.tech` 获得授权。如果想编写自己的智能合约，可参考该[智能合约](https://ropsten.etherscan.io/address/0x45464EbC79186AA313A5c01D7E4447422ba36c97#code)，调用 register 函数时需要提供自己的合约地址。

   **注意3：** 可以通过 `EtherScan` 网站调用 `register` 函数, 传入的公钥需要添加 16 进制标识符 `0x`。 



## 示例

在本示例下， iris data 作为数据提供方提供的数据源，数据分析者使用 K-Means 算法对这些数据进行聚类分析。

7. (数据提供方) 使用SGX对称加密原始数据，加密后的结果文件为`sealed.output`, 该文件包含原始数据的哈希 (`data_id`) 。

   ```shell
   $ ./data_provider --data-url ./iris.data --sealed-data-url ./iris.data.sealed --sealer-path ../lib/edatahub.signed.so --plugin-path ../lib/libiris_reader.so --output sealed.output
   ```

8. (数据提供方) 将 iris data 注册到 `ypcd`。

   ```shell
   $ ./ypcshuttle --onchain --input=iris_data.ini
   ```

   **注意：** `iris_data.ini ` 的文件内容如下，用户需要将`$PATH_TO_YEEZ_PRIVACY_COMPUTING` 替换为自己的 `YeeZ-Privacy-Computing` 目录，同时将`$data_id` 替换为上一步骤中输出文件(`sealed.outpu`)中的 `data_id`。

   ```shell
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

9. (数据提供方) 将iris data 注册到智能合约(注册成功后会创建一个新的智能合约)。

   ```c++
   contract address: 0x4be440d8af3c2AbD7b6aE1eF6176b8C516f360B7
   contract function: createYZData(bytes32 hash, string name, string desc, bytes sample,
                                   string env_info, uint price, address cert_addr,
                                   address program_proxy)
   ```

   -  `hash` 是 iris data 的数据哈希值 `$data_id`，该哈希值在步骤 7 产生。
   -  `name`/`desc`/`sample`/`env_info`  分别描述了 iris data 的元数据信息。
   -  `price` 是使用该 iris data 进行数据分析的费用。
   -  `cert_addr` 和 `program_proxy` 分别是[编译步骤](#编译(源码编译))生成的 `CertifiedUsers` 合约和 `ProgramStore` 合约地址。 

   **注意：** 交易的发送方应该是 `$PROVIDER_ADDR`。

10. (数据提供方) 检查步骤 9 中的 `createYZData` 交易的事件日志，可以获取数据合约以及数据需求合约的地址。

    举例来说，一个执行成功的[create data transaction](https://ropsten.etherscan.io/tx/0x414ce1c2e1d89befa6e06cdb66389fedbdd3c8993d1545e070d15b6955a5eea3) 会产生 3 个 [event logs](https://ropsten.etherscan.io/tx/0x414ce1c2e1d89befa6e06cdb66389fedbdd3c8993d1545e070d15b6955a5eea3#eventlog), 这些日志中包含一个 [NewYZData contract](https://ropsten.etherscan.io/address/0xf9c4ce0eab21e5842fd8fb4a7501b95b85507a4a) 和一个 [NewYZDataRequest contract](https://ropsten.etherscan.io/address/0xe47667668fdc98eff6a690547a6399d875407b7d)。 将这两个合约的地址拷贝到`YeeZ-Privacy-Computing/toolkit/blockchain/ethereum/common/const.py` 文件中的 `$contract_YZData` 和 `$contract_YZDataRequest` 中。

11. (数据提供方)启动监听数据需求的守护进程。

    ```shell
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/ && pip3 install -r requirements.txt
    $ export MYSQL_URL=$MYSQL_URL MYSQL_USERNAME=$MYSQL_USERNAME MYSQL_PASSWORD=$MYSQL_PASSWORD YPCD_DB=$YPCD_DB
    $ export YPC_HOME=$PATH_TO_YEEZ_PRIVACY_COMPUTING SUBMITTER_PASSWD=$YPC_HOME
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/common && python3 db_tables.py
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/ && python3 daemon.py --host $HOST --project_id $PROJECT_ID
    ```

    - `$PATH_TO_YEEZ_PRIVACY_COMPUTING` 是 `YeeZ-Privacy-Computing` 所在的路径。
    - `$MYSQL_URL`/`$MYSQL_USERNAME`/`$MYSQL_PASSWORD`/`$YPCD_DB`需要用户替换为自己的值。
    - `$MYSQL_URL` 通常情况下指定为`127.0.0.1:3306`,  `YPCD_DB` 设置为 `ypcd`。
    - `$SUBMITTER_PASSWD` 用于加密私钥。
    - `$HOST` 代表以太坊网络名称，设置为 `ropsten`。
    - `$PROJECT_ID` 由 [Infura](https://infura.io/) 提供。可以在 [Infura](https://infura.io/) 注册一个账户，随后在账户中创建一个以太坊项目即可获得 `$PROJECT_ID` 。
    - 执行 `daemon.py` 文件时确保 `python3 ` 环境已经安装 `hexbytes` 、 `ethereum` 和 `web3`包。

12. (数据分析者) 首先，数据分析者可以浏览发布在区块链上的数据集。如果一个数据分析者看到 `iris data` 数据集并且意欲使用  K-Means 算法对该数据集进行数据分析。 他编写了一个分析程序，这里称之为 `iris_parser.signed.so`。他需要将该程序上传到一个服务器以便于其他人可以通过下载链接进行访问，随后调用合约函数 `upload_program(string,string,string,uint256,bytes32)` 将 `isrs_parser.signed.so` 的信息注册到智能合约中。

    ```shell
    contract address: 0xBC8214F07a3253091aE8fb0A01079ea7B4955768
    contract function: upload_program(string name, string desc, string url, uint256 price, bytes32 enclave_hash)
    ```

    - `url` 通过将程序上传到服务器后获得。
    - `price` 是使用这个程序的费用。
    - `enclave_hash` 来自执行下述命令后生成的`dump.out`文件中的 `enclave_hash`字段。

    ```shell
    $ cd YeeZ-Privacy-Computing/lib
    $ sgx_sign dump -enclave iris_parser.signed.so -dumpfile dump.out
    ```

    **注意：** 交易发送方应该是 `$CONSUMER_ADDR`。

    

13. (数据分析者) 在 `YeeZ-Privacy-Computing/toolkit/yprepare/sample.json` 文件中添加如下内容：

    ```json
    {                                                                                
      "data": [ 
          {
           "data-hash" : data_id,                     
           "provider-pkey" : CONSUMER_PKEY
          }                                                                                 
      ]                                 
    }
    ```

    - `$data_id` 是数据提供方的数据哈希。
    - `$CONSUMER_PKEY` 是数据分析者的公钥。

14. (数据分析者) 生成调用 YZDataFactory 合约的参数(`encrypted-input`, `program-enclave-hash` ) 。`$data_id` 和 `$CONSUMER_PKEY` 分别是步骤 7 和步骤 5 产生的参数。

    ```shell
    $ cd YeeZ-Privacy-Computing/bin
    $ ./yprepare --dhash=$data_id --use-pubkey=$CONSUMER_PKEY --use-param xxx --param-format text --use-enclave ../lib/iris_parser.signed.so --output params.json
    ```

15. (数据分析者) 请求进行数据分析。

    ```shell
    contract address: contract_YZDataRequest
    contract function: request_data(bytes secret，bytes input, bytes forward_sig, bytes32 program_hash, uint gas_price)
    ```

    - `contract_YZDataRequest` 由步骤 10 生成。
    - `secret`/`input`/`forward_sig`  由步骤14 中生成的文件 `params.json`  中的 `$encrypted-skey`/`encrypted-input`/`forward-sig` 指定。
    - `program_hash` 由步骤 12 中的交易日志中 `Data` 中的 `hash` 字段。

    在数据提供方一侧，步骤 11 产生的需求数据监听守护进程会监听到数据分析者发出的该交易，随后自动下载 `iris_parser.signed.so` 文件并运行数据分析程序。 运行结束后，分析结果会被自动发送到智能合约 `YZDataFactory` 中。

    **注意：** 交易发送方应该是 `$CONSUMER_ADDR`。在配置文件 `$PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/transaction/config/YZDataRequest.json` 中需要对 `$keystore_path`, `$sender` 和 `$contract_address` 进行修改。其中：

    - `$keystore_path` 是在步骤 11 中由 `$SUBMITTER_PASSWD` 加密私钥得到的以太坊密钥库。
    - `$sender` 设置为 `$CONSUMER_ADDR`。
    - `contract_address` 设置为 `$contract_YZDataRequest` 。


16. (数据分析者) 解密结果。加密后的分析结果会被自动发送到区块链智能合约中，数据分析者可以通过如下命令解密分析结果:

    ```shell
    $ ./keymgr_tool --decrypt $encrypted-result --decrypt.private-key $CONSUMER_SEALED_KEY
    ```

## 授权

`YeeZ-Privacy-computing` 库(即 `toolkit`目录下的所有代码) 根据 [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) 获得许可，同时也包括我们存储库中的 `COPYING.APACHE` 文件。

`YeeZ-Privacy-computing`  二进制文件(即toolkit目录下的所有代码) 根据 [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) 获得授权，同时也包括我们存储库中的`COPYING` 文件。