# 编译Fidelius

注意：本文内容仅适合Fidelius的开发者，如果您只是需要使用Fidelius或基于Fidelius进行开发，请直接安装二进制文件！

## 简介
Fidelius的构建系统使用CMake（>=3.12)，

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
使用CTest运行所有单元测试

`cd ./build_debug`
`ctest `

也可以在`bin/`目录下执行相应的测试程序

## 安装
在生成编译脚本时，可以通过`-DCMAKE_INSTALL_PREFIX` 指定安装目录，若不指定，则会安装到系统目录。

可以同时安装`Debug`版本和`Release`版本，安装文件不会冲突。

## FAQ

1. Q: 每次构建CMake时GoogleTest的检查太慢

    A: 每次构建脚本时，都会检查GoogleTest是否更新了，在网络状况不太好的情况下，会比较慢。可以在另一个目录下clone整个GoogleTest，然后讲这个目录配置到环境变量GTEST_REPO中，例如：
    `export GTEST_REPO=/home/user/git/googletest`



