中文 / [English](../README.md)

Fidelius - 熠智隐私计算中间件
=====================================



## 引言

为了赋能企业间的数据合作，助力企业利用数据提升自身核心竞争力，熠智科技推出了面向数据合作的一站式隐私保护解决方案 Fidelius。Fidelius 基于“数据可用不可见”思想，有效保证了原始数据的一致性、计算逻辑的可控性、计算结果的正确性及隐私性。

下图描述了基于 Fidelius 实现数据合作的抽象流程。与传统的数据合作模式类似，参与方包括了数据提供方和数据使用方。Fidelius 中间件分别运行在数据提供方和数据使用方中，双方通过与 Fidelius 交互实现数据合作操作。数据提供方和数据使用方之间没有直接的数据交互，并且原始数据不会离开数据提供方的 Fidelius 中间件，这从根本上避免了隐私数据泄露的问题。

![](Fidelius-Infr.png)

相比传统的数据合作模式，Fidelius 引入了区块链网络。由于区块链本身具有去中心化网络、公开可验证等特性，Fidelius 将其作为可信的传输通道和数据计算验证平台。

**注意：** 这是一个社区版本。尽管它与我们的企业版共享相似的组件，但它有不同的特性。了解企业版更多信息，请联系 `contact@yeez.tech`。



## 文档

- [Fidelius: YeeZ Privacy Protection for Data Collaboration - A Blockchain based Solution](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)



## 编译 Fidelius

我们提供 Fidelius 两种编译模式，Debug 模式和 Release 模式。Debug 模式请按照本指导进行编译。

由于 Fidelius 中使用到了英特尔公司的 `Software Guard Extention (SGX)`，Release 模式下编译需要开发者事先完成产品授权，否则将无法正常工作，如果想使用 Release 版本的 Fidelius，请查看英特尔 [ SGX 开发文档](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions/attestation-services.html)，或者也可以通过联系 `contact@yeez.tech` 取得帮助。



### 环境依赖

- 确保拥有如下操作系统：

  - Ubuntu\* 18.04 LTS Server 64bits

- 确保 BIOS 和 CPU 启用 SGX，请遵循 [SGX-hardware](https://github.com/ayeks/SGX-hardware) 中的 [README.md](https://github.com/ayeks/SGX-hardware/blob/master/README.md) 。

- 按照 [linux-sgx](https://github.com/intel/linux-sgx) 项目中 [README.md](https://github.com/intel/linux-sgx/blob/master/README.md) 文档进行编译并安装 `Intel(R) SGX` 驱动、`SDK` 和 `PSW`。

- 安装依赖工具（protobuf, glog, boost, cppconn等）：

  ```shell
  $ sudo apt install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl
  $ sudo apt install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip
  $ sudo apt install mysql-server libgoogle-glog-dev libboost-all-dev libmysqlcppconn-dev
  ```

- 安装 cryptopp：

  ```shell
  $ git clone https://github.com/weidai11/cryptopp
  $ cd cryptopp && make -j8 && sudo make install
  ```

- 安装以太坊智能合约环境的依赖工具：[Solidity](https://docs.soliditylang.org/en/v0.8.2/installing-solidity.html) and [Truffle](https://www.trufflesuite.com/)。

  ```
  $ npm install -g solc
  $ npm install -g truffle
  ```

- 确保拥有两个以太坊测试网络 `Ropsten` 上的账户（分别作为数据提供方和数据使用方），且每个账户至少有 1 个 `ETH`。账户可以用 `MetaMask` 钱包生成，`ETH` 可以访问[该网址](https://faucet.ropsten.be/)申请得到。

- 确保拥有一个 `INFURA_API_KEY`，可以通过 [Infura](https://infura.io/) 网站获取。

- 确保拥有一个 `ETHERSACAN_API_KEY`, 可以通过 [EtherScan](https://ropsten.etherscan.io/) 网站获取。

### 编译（源码编译）

- 下载 Fidelius 源码：

  ```shell
  $ git clone https://github.com/YeeZTech/YeeZ-Privacy-Computing.git
  ```



- 编译安装其依赖的子模块：

  ```shell
  $ git submodule update --init
  $ cd YeeZ-Privacy-Computing/vendor/fflib
  $ mkdir -p build
  $ cd build
  $ cmake .. && make
  ```



- 以 Debug 模式编译 Fidelius，执行如下命令：

  ```shell
  $ cd YeeZ-Privacy-Computing
  $ mkdir -p build && cd build
  $ cmake -DSGX_MODE=Debug -DSGX_HW=OFF ..
  $ make -j8
  ```

  编译成功后可以在 `YeeZ-Privacy-Computing/lib` 和 `YeeZ-Privacy-Computing/bin` 目录下中找到库文件和二进制文件。



- 编写部署智能合约所需的配置文件。在 `YeeZ-Privacy-Computing/contracts/backend` 目录下创建 `.env` 文件，其内容如下：

  ```shell
  ROPSTEN_PK = $Private_key
  ETHERSCAN_API_KEY = $EtherScan_API_KEY
  INFURA_API_KEY = $Infura_API_KEY
  ```

  - `$Private_key` 是用户用于部署智能合约的密钥。
  - `$EtherScan_API_KEY` 是在  [EtherScan](https://ropsten.etherscan.io/) 网站获取的 API Key。
  - `$Infura_API_KEY` 是在  [Infura](https://infura.io/) 网站获取的 API Key 。




- 安装部署智能合约所需的依赖  ` truffle-privatekey-provider` 和 `dotenv` , 执行如下命令：

  ```shell
  $ cd YeeZ-Privacy-Computing/contracts/backend
  $ npm install truffle-privatekey-provider
  $ npm install dotenv
  ```



- 编译和部署以太坊智能合约，执行如下命令：

  ```shell
  $ cd YeeZ-Privacy-Computing/contracts/backend && truffle migrate --network ropsten
  ```

  **注意：** 上述命令执行大约需要 15 分钟左右，执行完毕之后将部署 4 个智能合约，分别是 `CertifiedUsers` 、`ProgramStore`、`YZDataRequestFactory` 和 `YZDataFactory`。用户需要记录 这4 个智能合约的地址以方便后续使用。

  - `CertifiedUsers` 合约用于存储数据提供方和数据使用方的注册信息。

  - `ProgramStore` 合约用于存储数据使用方的分析程序的元信息（程序名称、描述信息、下载地址等）。

  - `YZDataRequestFactory` 合约用于存储数据使用方对数据提供方数据的请求信息（程序哈希、程序输入参数等），处理数据合作请求并完成交易结算。

  - `YZDataFactory` 合约用于存储数据提供方的数据元信息（数据哈希，数据名称、数据描述、数据样例、数据价格等）。

  我们使用以太坊作为底层区块链系统，并将相关智能合约部署到以太坊测试网络，即 [Ropsten](https://ropsten.etherscan.io/)。

  具体的智能合约存储在 `YeeZ-Privacy-Computing/contracts` 目录下。



## 运行 Fidelius

基于 Fidelius 的编译模式，Fidelius 可以在 Debug 和 Release 模式下运行。Debug 模式请按照本指导运行，以 Release 模式运行，请查看英特尔 [ SGX 开发文档](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions/attestation-services.html)，或者也可以通过联系 `contact@yeez.tech` 取得帮助。


由于 Fidelius 系统中有两个角色，数据提供方和数据使用方，因此运行 Fidelius 时，我们会指定命令在哪一方执行。

### 初始化

Fidelius 初始化步骤如下：

1. （数据提供方）创建数据库。

   ```sql
   mysql> create database ypcd;
   ```



2. （数据提供方）在 `YeeZ-Privacy-Computing/bin` 目录下创建配置文件 `ypcd.conf`，文件内容如下所示。

   ```shell
   [mysql]
   url = $MYSQL_URL
   usr-name = $USER_NAME
   usr-password = $USER_PASSWD
   database = ypcd

   [net]
   control-port = $PORT
   ```

   - `$MYSQL_URL`，`$USER_NAME`，`$USER_PASSWD`，`$PORT` 需要用户替换为自己的用户名和密码。
   - 默认 `$MYSQL_URL` 为 `tcp://127.0.0.1:3306` 。
   - 默认 `$PORT` 为 `7068` 。



3. （数据提供方）初始化 `ypcd` 数据库。

   ```shell
   $ cd YeeZ-Privacy-Computing/bin && ./ypcd --init
   ```



4. （数据提供方）启动 Fidelius 守护进程（ypcd）。

   ```shell
   ./ypcd --start
   ```



5. （数据提供方/数据使用方）创建一对公私钥对，创建完成后存储在 `$HOME/.yeez.key/` 目录下，公私钥以 `json` 文件格式进行存储。

   ```shell
    $ ./keymgr_tool --create
   ```

    **注意：** 使用 Fidelius 时， 数据提供方和数据使用方都需要生成自己的公私钥对。



6. （数据提供方/数据使用方）注册公钥。通过调用 `CertifiedUsers` 智能合约中的 `register` 函数注册公钥。
   `CertifiedUsers` 智能合约在[编译步骤](#编译（源码编译）) 中的部署智能合约得到。
   `register` 函数格式如下：

   ```shell
   register(address addr, bytes pkey)
   ```

   - `addr` 是用户以太坊账户地址。

   - `pkey` 是用户在步骤 5 生成的公钥。

   **注意1：** 上述智能合约作为数据提供方和数据使用方进行数据合作的可信第三方，协助完成数据计算服务以及费用结算，详细信息请参看其源码和示例。

   **注意2：** 上述合约地址是我们部署到 `Ropsten` 的合约地址，只有授权用户才能调用该函数，如果想使用该合约地址，请联系 `contact@yeez.tech` 获得授权。如果想编写自己的智能合约，可参考 [该智能合约](https://ropsten.etherscan.io/address/0x45464EbC79186AA313A5c01D7E4447422ba36c97#code)。

   **注意3：** 可以通过 `EtherScan` 网站调用 `register` 函数，传入的公钥需要添加十六进制标识符 `0x`。



## 示例

经过上述初始化步骤，我们搭建好了 `Fidelius` 的运行环境，下面以一个具体的示例进行说明。

在本示例下，数据提供方提供 `iris data`, 数据使用方使用 `K-Means` 算法，双方通过与智能合约交互完成数据合作，合作流程如下：

- 数据提供方将自身 `iris data` 的元数据信息注册到 `YZData` 智能合约中。

- 数据使用方从 `YZData` 智能合约浏览到了其提供的 `iris data` 数据描述信息并编写了 `K-Means` 分析程序。

- 数据使用方将分析程序的元信息上传到 `ProgramStore ` 智能合约中。

- 数据使用方将数据分析请求发送至 `YZDataRequest` 智能合约中请求数据分析。

- 数据提供方监听到数据使用方的分析请求，随后下载分析程序到本地，在本地 `Trusted Execution Environment (TEE)` 环境下执行分析程序。

- 数据提供方执行完毕后将执行结果加密上传到 `YZDataRequest` 智能合约中。

- 数据使用方从 `YZDataRequest` 获取加密的执行结果，随后解密得到分析结果的明文数据。

  **注意1：** 在第 7 步开始进行，必须保证前述的 6 个步骤正确完成。

  **注意2：** 如无路径说明， 示例中命令默认在 `YeeZ-Privacy-Computing/bin` 目录下执行。



7. （数据提供方）使用 `SGX` 对称密钥加密 `iris data`，加密后的结果文件为 `iris.data.sealed`，同时输出加密过程的描述文件 `sealed.output`，`sealed.output` 文件包含原始数据的哈希（`data_id`），执行命令如下：

   ```shell
   $ ./data_provider --data-url ./iris.data --sealed-data-url ./iris.data.sealed --sealer-path ../lib/edatahub.signed.so --plugin-path ../lib/libiris_reader.so --output sealed.output
   ```



8. （数据提供方）将 `iris data` 的相关信息注册到其数据库 `ypcd` 中。

   ```shell
   $ ./ypcshuttle --onchain --input=iris_data.ini
   ```

   **注意：** `iris_data.ini` 的文件内容如下所示。

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

   用户需要将 `$PATH_TO_YEEZ_PRIVACY_COMPUTING` 替换为自己的 `YeeZ-Privacy-Computing` 目录。

   `$data_id` 是上一步骤中输出文件 `sealed.output` 中的 `data_id`。



9. （数据提供方）调用 `YZDataFactory` 智能合约中的 `createYZData` 函数注册 `iris data` 的相关信息。

    `createYZData` 函数格式如下：

   ```solidity
   createYZData(bytes32 hash, string name, string desc, bytes sample,
                string env_info, uint price, address cert_addr, address program_proxy)
   ```

   - `hash` 是 `iris data` 数据的哈希值 `$data_id`，该哈希值在步骤 7 产生。

   - `name`/`desc`/`sample`/`env_info`  分别描述了 `iris data` 的元数据信息。

   - `price` 是使用该 `iris data` 进行数据分析的费用，可以填写为0。

   - `cert_addr` 和 `program_proxy` 分别是 [编译步骤](#编译（源码编译）)生成的 `CertifiedUsers` 合约地址和 `ProgramStore` 合约地址。

   交易执行成功后会产生 3 个事件日志，日志中包含新生成的 `YZData` 和 `YZDataRequest` 这两个智能合约的地址。

   这里是一个成功执行的 [创建数据交易](https://ropsten.etherscan.io/tx/0x414ce1c2e1d89befa6e06cdb66389fedbdd3c8993d1545e070d15b6955a5eea3)，以及其 [事件日志](https://ropsten.etherscan.io/tx/0x414ce1c2e1d89befa6e06cdb66389fedbdd3c8993d1545e070d15b6955a5eea3#eventlog)，该日志包含 [YZData](https://ropsten.etherscan.io/address/0xf9c4ce0eab21e5842fd8fb4a7501b95b85507a4a) 和 [YZDataRequest](https://ropsten.etherscan.io/address/0xe47667668fdc98eff6a690547a6399d875407b7d) 的地址。



10. （数据提供方）将 `YZData` 和 `YZDataRequest` 两个合约的地址拷贝到 `YeeZ-Privacy-Computing/toolkit/blockchain/ethereum/common/const.py` 文件中的 `$contract_YZData` 和 `$contract_YZDataRequest` 中。



11. （数据提供方）启动监听进程，该进程会从区块链网络中获取数据使用方发送的数据分析需求。

    ```shell
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/
    $ pip3 install -r requirements.txt
    $ export MYSQL_URL=$MYSQL_URL MYSQL_USERNAME=$MYSQL_USERNAME MYSQL_PASSWORD=$MYSQL_PASSWORD YPCD_DB=$YPCD_DB
    $ export YPC_HOME=$PATH_TO_YEEZ_PRIVACY_COMPUTING SUBMITTER_PASSWD=$SUBMITTER_PASSWD
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/common
    $ python3 db_tables.py
    $ cd $PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/
    $ python3 daemon.py --host $HOST --project_id $PROJECT_ID
    ```

    - `$PATH_TO_YEEZ_PRIVACY_COMPUTING` 是 `YeeZ-Privacy-Computing` 所在的路径。
    - `$MYSQL_URL`/`$MYSQL_USERNAME`/`$MYSQL_PASSWORD`/`$YPCD_DB` 需要用户替换为自己的值。
    - `$MYSQL_URL` 通常情况下指定为`127.0.0.1:3306`，`$YPCD_DB` 设置为 `ypcd`。
    - `$SUBMITTER_PASSWD` 为解密 `Keystore` 的密码。
    - `$HOST` 代表以太坊网络名称，设置为 `ropsten`。
    - `$PROJECT_ID` 由 [Infura](https://infura.io/) 提供。可以在 [Infura](https://infura.io/) 注册一个账户，随后在账户中创建一个以太坊项目即可获得 `$PROJECT_ID` 。
    - 执行 `daemon.py` 文件时确保 `python3` 环境已经安装 `hexbytes`、`ethereum` 和 `web3` 包。


    经过上述步骤，数据提供方已经将自身数据成功注册到区块链中并开始监听数据使用方的分析请求，一旦监听到数据使用方的分析需求，监听进程便自动下载其分析程序并开始执行。
    
    下面介绍数据使用方如何根据数据提供方的数据信息发起数据分析请求。




12. （数据使用方）生成自身的公私钥对，该步骤参考第 5 步骤。



13. （数据使用方）将公钥注册到 `CertifiedUsers` 智能合约中，参考第 6 步骤。



14. （数据使用方）数据使用方浏览发布在区块链上的数据集，看到 `iris data` 数据集，想使用 `K-Means` 算法对该数据集进行数据分析。

    数据使用方编写分析程序 `iris_parser.signed.so`， 将该程序上传到一个服务器以便于数据提供方下载。

    例如，在本示例中，该分析程序位于 `https://download.yeez.tech/bin/iris_parser.signed.so`。



15. （数据使用方）生成 `enclave_hash`，指令如下命令：

    ```shell
    $ cd YeeZ-Privacy-Computing/lib
    $ sgx_sign dump -enclave iris_parser.signed.so -dumpfile dump.out
    ```

    执行成功后会生成 `dump.out` 文件，其中包含 `enclave_hash` 字段，该字段用于后续发起数据分析请求。



16. （数据使用方）调用 `ProgramStore` 智能合约中的 `upload_program` 函数注册分析程序 `iris_parser.signed.so` 的相关信息。

    `upload_program` 函数格式如下：

    ```shell
    upload_program(string name, string desc, string url, uint256 price, bytes32 enclave_hash)
    ```

    - `name` 和 `desc` 是该分析程序的名称和描述信息。
    - `url` 是数据分析方存储其数据分析程序的地址，例如为 `https://download.yeez.tech/bin/iris_parser.signed.so`。
    - `price` 是使用这个程序的费用，这里设置为 0 。
    - `enclave_hash` 来自前一步骤生成的 `dump.out` 文件中的 `enclave_hash` 字段。

    执行成功后，该交易日志中的 `Data` 字段中存在一个 `hash` 字段，数据使用方需要进行记录，将用于后续步骤。



17. （数据使用方）在 `YeeZ-Privacy-Computing/toolkit/yprepare/sample.json` 文件中添加如下内容：

    ```json
    {
      "data": [
          {
           "data-hash" : $data_id,
           "provider-pkey" : $CONSUMER_PKEY
          }
      ]
    }
    ```

    - `$data_id` 是数据哈希。

    - `$CONSUMER_PKEY` 是数据使用方的公钥。



18. （数据使用方）生成调用 `YZDataRequest` 合约的参数（`encrypted-input`，`program-enclave-hash`），执行如下命令：

    ```shell
    $ cd YeeZ-Privacy-Computing/bin
    $ ./yprepare --dhash=$data_id --use-pubkey=$CONSUMER_PKEY --use-param xxx --param-format text --use-enclave ../lib/iris_parser.signed.so --output params.json
    ```

    - `$data_id` 是数据提供方 `iris data` 的数据哈希值，其值由数据提供方在步骤 7 生成。
    - `$CONSUMER_PKEY` 是数据使用方的公钥。
    - 其他参数无需修改。



19. 对配置文件 `$PATH_TO_YEEZ_PRIVACY_COMPUTING/toolkit/blockchain/ethereum/transaction/config/YZDataRequest.json` 中的 `$keystore_path`，`$sender` 和 `$contract_address` 进行修改。其中：

    - `$keystore_path` 是在步骤 11 中由 `$SUBMITTER_PASSWD` 加密私钥得到的以太坊密钥库路径。

    - `$sender` 设置为数据使用方的以太坊账户地址。

    - `contract_address` 设置为 `YZDataRequest` 的合约地址，该合约由步骤 9 生成。



20. （数据使用方）调用 `YZDataRequest` 智能合约中的 `request_data` 函数发起数据分析请求交易。

    `request_data` 函数格式如下：

    ```shell
    request_data(bytes secret, bytes input, bytes forward_sig, bytes32 program_hash, uint gas_price)
    ```

    - 智能合约 `YZDataRequest` 由步骤 9 生成。
    - `secret`/`input`/`forward_sig` 由步骤 18 中生成的文件 `params.json` 中的 `$encrypted-skey`/`encrypted-input`/`forward-sig` 指定。
    - `program_hash` 由步骤 16 中的交易日志中 `Data` 的 `hash` 字段。



21. （数据使用方）解密结果。在数据提供方一侧，步骤 11 产生的请求数据，监听守护进程会监听到数据使用方发出的该交易，随后自动下载 `iris_parser.signed.so` 文件并运行数据分析程序。运行结束后，分析结果会被自动发送到智能合约 `YZDataRequest` 中。

    数据使用方可以通过如下命令解密分析结果:

    ```shell
    $ ./keymgr_tool --decrypt $encrypted-result --decrypt.private-key $CONSUMER_SEALED_KEY
    ```



## 授权

`YeeZ-Privacy-Computing` 库(即 `toolkit`目录下的所有代码) 根据 [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) 获得许可，同时也包括我们存储库中的 `COPYING.APACHE` 文件。

`YeeZ-Privacy-Computing`  二进制文件(即toolkit目录下的所有代码) 根据 [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) 获得授权，同时也包括我们存储库中的`COPYING` 文件。
