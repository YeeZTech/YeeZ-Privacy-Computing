# 编译Fidelius

注意：本文内容仅适合Fidelius的开发者，如果您只是需要使用Fidelius或基于Fidelius进行开发，请直接安装二进制文件！

## 简介
Fidelius的构建系统使用CMake（>=3.12)。

## 获取源代码

`git clone https://github.com/YeeZTech/YeeZ-Privacy-Computing.git`

`git submodule update --init`

## 编译Fidelius

### 编译Debug版本

生成编译脚本

`cmake -DSGX_MODE=Debug -S . -B ./build_debug `

除了生成相应的编译脚本（Makefile）之外，这一命令还会下载必要的依赖项，如`fflib`, `pybind11`, `google-test`等，并对依赖项进行编译。
接下来，可以进行编译：

`cmake --build ./build_debug`

如果需要并行编译，可以增加`-j`选项。

### 编译其他版本

在生成编译脚本时，可以指定`SGX_MODE`为`Debug`， `PreRelease`及`Release`，相应的，`SGX_HW`分别设置为`Off`, `On`及`On`。
更进一步的，可以通过`CMAKE_BUILD_TYPE`指定非SGX部分的版本，若不指定，则根据SGX_MODE指定。

## 测试

### 单元测试
使用CTest运行所有单元测试

`cd ./build_debug`
`ctest `

也可以在`bin/`目录下执行相应的测试程序

### 测试覆盖
在已经安装了`locv`的情况下，会在`CMAKE_BUILD_TYPE`为`Debug`时自动开启测试覆盖功能，可以执行
```
cmake --build ./build_debug -t coverage
```
获得测试覆盖报告（位于`build_debug/coverage/index.html`），可以使用浏览器打开该报告。

### CDash
可以使用如下命令生成一份测试报告，并将测试报告上传至[CDash](https://my.cdash.org/index.php?project=Fidelius)。

仅提交测试报告时的命令如下
```
cd build_debug && ctest --dashboard Experimental
```
同时进行构建、测试及提交测试报告的命令如下
```
cmake --build ./build_debug --target Experimental
```

TODO: 需要进一步定期生成测试报告

## 安装
在生成编译脚本时，可以通过`-DCMAKE_INSTALL_PREFIX` 指定安装目录，若不指定，则会安装到系统目录。

可以同时安装`Debug`版本和`Release`版本，安装文件不会冲突。

## 打包
使用CPack进行打包，
```
cd ./build_debug && cpack -G "DEB;7Z;ZIP"
```
注意，该命令为打包debug版本。

由于可以在SGX和非SGX环境下运行，因此打包时需要生成两个不同的安装包。

TODO: 需要设置`CPACK_DEBIAN_PACKAGE_DEPENDS`，否则不能自动安装依赖项。

## FAQ

1. Q: 每次构建CMake时GoogleTest的检查太慢

    A: 每次构建脚本时，都会检查GoogleTest是否更新了，在网络状况不太好的情况下，会比较慢。可以在另一个目录下clone整个GoogleTest，然后讲这个目录配置到环境变量GTEST_REPO中，例如：
    `export GTEST_REPO=/home/user/git/googletest`



