#!/bin/bash
clean_build_path() {
  now_path=$1
  build_path=$2
  if [ -d "$build_path" ]; then
    cd $build_path && make clean
    cd $now_path && rm -rf $build_path
  fi
}

build_project() {
  cmake -DCMAKE_INSTALL_PREFIX=$1 -DCMAKE_BUILD_TYPE=$2 -DCMAKE_DEBUG_POSTFIX=$3 -B $4
  cmake --build $4 -j
  #cmake --install $4
}

# build in Debug
NOW_PATH=`pwd`
DEBUG_BUILD_PATH=$NOW_PATH/build/debug
clean_build_path $NOW_PATH $DEBUG_BUILD_PATH
build_project $HOME "Debug" "_debug" $DEBUG_BUILD_PATH

# build in PreRelease
RELEASE_BUILD_PATH=$NOW_PATH/build/release
clean_build_path $NOW_PATH $RELEASE_BUILD_PATH
build_project $HOME "RelWithDebInfo" "" $RELEASE_BUILD_PATH
