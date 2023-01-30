#!/bin/bash
sign_enclave_hash() {
  private_key_path=$1
  enclave_hash_dir=$2

  declare -a enclave_libs=("iris_classifier" "iris_parser_for_offchain" "iris_parser"
                           "keymgr_gmssl" "keymgr"
                           "person_first_match" "person_first_match_multi" "person_first_match_multi_offchain"
                          )
  # 1. Generate a 3072-bit RSA private key.
  # openssl genrsa -out private_key.pem -3 3072
  # 2. Produce the public part of a private RSA key.
  # openssl rsa -in private_key.pem -pubout -out public_key.pem
  # 3. Sign the file containing the enclave signing material.
  # openssl dgst -sha256 -out signature.hex -sign private_ key.pem -keyform PEM enclave_hash.hex

  public_key_path=${enclave_hash_dir}/public_key.pem
  openssl rsa -in $private_key_path -pubout -out $public_key_path

  for filename in ${enclave_libs[@]}; do
    enclave_hash=${enclave_hash_dir}/${filename}_hash.hex
    enclave_sig=${enclave_hash_dir}/${filename}_sig.hex
    openssl dgst -sha256 -out $enclave_sig -sign $private_key_path -keyform PEM $enclave_hash
  done
}

create_signed_so() {
  enclave_hash_dir=$1
  ypc_home=`pwd`
  ypc_lib_path=${ypc_home}/lib
  public_key_path=${enclave_hash_dir}/public_key.pem

  declare -A enclave_libs=(["iris_classifier"]="example/iris/classifier/enclave/enclave.config.xml"
                           ["iris_parser_for_offchain"]="example/iris/analyzer/enclave_for_offchain/enclave.config.xml"
                           ["iris_parser"]="example/iris/analyzer/enclave/enclave.config.xml"
                           ["keymgr_gmssl"]="ypc/keymgr/default/enclave/ekeymgr.config.xml"
                           ["keymgr"]="ypc/keymgr/default/enclave/ekeymgr.config.xml"
                           ["person_first_match"]="example/personlist/first_match/enclave/enclave.config.xml"
                           ["person_first_match_multi"]="example/multi_personlist/first_match/enclave/enclave.config.xml"
                           ["person_first_match_multi_offchain"]="example/multi_personlist/first_match/enclave_for_offchain/enclave.config.xml"
                          )

  for filename in ${!enclave_libs[@]}; do
    enclave_hash=${enclave_hash_dir}/${filename}_hash.hex
    enclave_sig=${enclave_hash_dir}/${filename}_sig.hex
    /opt/intel/sgxsdk/bin/x64/sgx_sign catsig -enclave ${ypc_lib_path}/lib${filename}.so -config ${ypc_home}/${enclave_libs[$filename]} -out ${ypc_lib_path}/${filename}.signed.so -key $public_key_path -sig $enclave_sig -unsigned $enclave_hash
  done
}

clean_build_path() {
  now_path=$1
  build_path=$2
  if [ -d "$build_path" ]; then
    cd $build_path && make clean
    cd $now_path && rm -rf $build_path
  fi
}

compile_project() {
  ypc_home=`pwd`
  mode=$1
  case "$mode" in
    "DEBUG" | "Debug" | "debug")
      build_path=${ypc_home}/build/debug
      build_type="Debug"
      suffix="_debug"
      ;;
    "PRERELEASE" | "PreRelease" | "Prerelease" | "prerelease" | "pre_release")
      build_path=${ypc_home}/build/prerelease
      build_type="RelWithDebInfo"
      suffix=""
      ;;
    "RELEASE" | "Release" | "release")
      build_path=${ypc_home}/build/release
      build_type="Release"
      suffix=""
      ;;
    *)
      echo "Please specify build type \"Debug\" | \"PreRelease\" | \"Release\" !"
      echo "Usage -- \"./build.sh build-project \${BUILD_TYPE}\""
      exit
      ;;
  esac
  clean_build_path $ypc_home $build_path

  cmake -DCMAKE_INSTALL_PREFIX=$HOME -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_DEBUG_POSTFIX=$suffix -B $build_path
  cmake --build $build_path -j
  #cmake --install $build_path

  if [[ "$build_type" == "Release" ]]; then
    hash_dir=${ypc_home}/hash_hex
    rm -rf $hash_dir
    mkdir -p $hash_dir
    cp ./lib/*_hash.hex $hash_dir
  fi
}

case "$1" in
  compile-project)
    compile_project $2
    ;;
  sign-enclave-hash)
    sign_enclave_hash $2 $3
    ;;
  create-signed-so)
    create_signed_so $2
    ;;
esac

cpack -G "DEB"
