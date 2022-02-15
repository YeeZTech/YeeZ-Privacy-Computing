#include "corecommon/package.h"
#include "ypc/byte.h"
#include <iostream>

define_nt(v1, bool);
define_nt(v2, int8_t);
define_nt(v3, uint8_t);
define_nt(v4, int16_t);
define_nt(v5, uint16_t);
define_nt(v6, int32_t);
define_nt(v7, uint32_t);
define_nt(v8, int64_t);
define_nt(v9, uint64_t);
define_nt(va, float);
define_nt(vb, double);

define_nt(vstring, std::string);
define_nt(vbytes, ypc::bytes);

typedef ff::util::ntobject<v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb> mt;
typedef ff::util::ntobject<vstring, vbytes> ct;

int main(int argc, char *argv[]) {
  mt m;
  m.set<v1>(1);
  m.set<v2>(-12);
  m.set<v3>(123);
  m.set<v4>(-1212);
  m.set<v5>(1212);
  m.set<v6>(-12121212);
  m.set<v7>(12121212);
  m.set<v8>(12);
  m.set<v9>(12);
  m.set<va>(1.0);
  m.set<vb>(1.3);

  typename ypc::cast_obj_to_package<mt>::type p = m;
  auto ret = ypc::make_bytes<ypc::bytes>::for_package(p);
  std::cout << ret << std::endl;

  ct c;
  c.set<vstring>("hello world");
  c.set<vbytes>(ypc::bytes("hello world"));
  typename ypc::cast_obj_to_package<ct>::type pc = c;
  ret = ypc::make_bytes<ypc::bytes>::for_package(pc);
  std::cout << ret << std::endl;

  return 0;
}
