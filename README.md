  ![image](https://github.com/YeeZTech/YeeZ-Privacy-Computing/assets/5715598/0a388380-381e-4a49-b679-eb9c5c60cf26)

# Fidelius - 基于可信执行环境的熠智隐私计算中间件
English / [中文](doc/README_ZH.md)

# Fidelius - YeeZ Privacy Computing
## Introduction
Fidelius has launched an all-in-one privacy protection solution for data collaboration based on the idea of "data availability but not visible." This solution effectively ensures the consistency of original data, controllability of computing logic, correctness of computing results, and privacy protection.

The following figure describes the abstract process of data collaboration based on Fidelius. The participants include the data provider and the data user. Fidelius middleware runs on both the data provider and the data user, and the two parties interact with Fidelius to achieve data collaboration. The original data will not leave the Fidelius middleware of the data provider, fundamentally avoiding the problem of privacy data leakage.

<p align="center">
  <img src="doc/Fidelius-Infr.png" alt="drawing" width="61.8%"/>
</p>

In the figure, the trusted third party can choose to be a blockchain network. As the blockchain itself has characteristics such as a decentralized network and public verifiability, Fidelius can use it as a trusted transmission channel and data computing verification platform.

## Documentation
- [Fidelius: YeeZ Privacy Protection for Data Collaboration - A Blockchain based Solution](https://download.yeez.tech/doc/Fidelius_Introduction.pdf)

## Quick Start
Fidelius runs on Intel SGX and requires confirmation that the hardware environment is equipped with a supported central processing unit (CPU), BIOS settings, and the installation of drivers and related software. Even without the required hardware environment, the debug version can still run after installing the Intel SGX SDK.

If you have the relevant hardware, please refer to the [documentation](./doc/Release_EN.md) and run the Fidelius release version.

If you do not have the relevant hardware, please refer to the [documentation](./doc/Debug_EN.md) and run the Fidelius debug version.

## Advanced Use
- [doxygen documentation](https://doc-fidelius.yeez.tech/index.html)
- [wiki documentation](https://doc-dianshu.yeez.tech/index.php/Fidelius)

## License
The `YeeZ-Privacy-Computing` library (i.e. all code outside of the `toolkit` directory) is licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0), also included in our repository in the `COPYING.APACHE` file.

The `YeeZ-Privacy-Computing` binaries (i.e. all code inside of the `toolkit` directory) is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html), also included in our repository in the `COPYING` file.

## How to contribute
If you want to contribute to this project, feel free to create an issue at our Issue page (e.g., documentation, new idea and proposal).

This is an active open source project for everyone, and we are open to everyone who want to use this system or contribute to it.
## Contributors
<a href="https://github.com/YeeZTech/YeeZ-Privacy-Computing/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=YeeZTech/YeeZ-Privacy-Computing" />
</a>

Made with [contrib.rocks](https://contrib.rocks).

## Community

- Wechat Official Account:
- <img width="213" alt="CleanShot 2023-02-02 at 14 21 19@2x" src="https://user-images.githubusercontent.com/5715598/216247527-d60fbcc1-0de8-4a6c-88ba-9a1f51f903fc.png">

