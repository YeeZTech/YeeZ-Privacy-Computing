> 中文 / [English](../README.md)

# Fidelius - 熠智隐私计算中间件
## 特性
Fidelius 基于“数据可用不可见”思想，推出了面向数据合作的一站式隐私保护解决方案，有效保证了原始数据的一致性、计算逻辑的可控性、计算结果
的正确性及隐私性。

下图描述了基于 Fidelius 实现数据合作的抽象流程。图中参与方包括数据提供方和数据使用方。Fidelius 中间件分别运行在数据提供方和数据使用方中，双方通过与 Fidelius 交互实现数据合作。原始数据不会离开数据提供方的 Fidelius 中间件，这从根本上避免了隐私数据泄露的问题。

<p align="center">
  <img src="Fidelius-Infr.png" alt="drawing" width="61.8%"/>
</p>

相比传统的数据合作模式，Fidelius 可选择引入区块链网络。由于区块链本身具有去中心化网络、公开可验证等特性，Fidelius 将其作为可信的传输通道和数据计算验证平台。

## 文档
- [Fidelius：面向数据合作的隐私保护区块链解决方案](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)

## 快速开始
Fidelius 基于 Intel SGX 运行，需确认硬件环境配备了支持的中央处理器（CPU），对 BIOS 进行设置，并安装驱动和相关软件。即使不具备该硬件环境，仍能在安装了 Intel SGX SDK 之后运行 Debug 版本。

具备相关硬件，运行 Fidelius Release 版本请参考[这里](./Release_ZH.md)。

不具备相关硬件，运行 Fidelius Debug 版本请参考[这里](./Debug_ZH.md)。

## 进阶使用
- Fidelius 的 doxygen 文档：https://doc-fidelius.yeez.tech/index.html
- Fidelius 的 wiki 文档： https://doc-dianshu.yeez.tech/index.php/Fidelius
### 测试
如果开发者想在修改之后测试，可以使用如下命令生成一份测试报告 [CDash](https://my.cdash.org/index.php?project=Fidelius)
```
$ cd build_debug && ctest --dashboard Experimental
```

## 授权
`YeeZ-Privacy-Computing`库(即`toolkit`目录下的所有代码) 根据 [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) 获得许可，同时也包括我们存储库中的`COPYING.APACHE`文件。

`YeeZ-Privacy-Computing`二进制文件(即`toolkit`目录下的所有代码) 根据 [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) 获得授权，同时也包括我们存储库中的`COPYING`文件。

## 贡献
如果您想参与 Fidelius 项目，可以在 [Issue](https://github.com/YeeZTech/YeeZ-Privacy-Computing/issues) 页面随意开启一个新的话题，比如文档、创意、Bug等。
我们是一个开放共建的开源项目，欢迎参与到我们的项目中～

### 贡献者
<a href="https://github.com/YeeZTech/YeeZ-Privacy-Computing/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=YeeZTech/YeeZ-Privacy-Computing" />
</a>

## 社区
* 微信：

![wechat_helper](./wechat_image.JPG)

## Fidelius 企业版
这是一个社区版本。了解企业版更多信息，请联系`contact@yeez.tech`。
