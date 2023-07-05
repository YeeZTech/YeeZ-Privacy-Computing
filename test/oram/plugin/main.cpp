#include "ypc/core/privacy_data_reader.h"
#include "ypc/corecommon/package.h"

define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;

int main(int argc, char *argv[]) {
  std::string plugin = "/home/lumj/YeeZ-Privacy-Computing/./lib/libperson_reader_oram.so";
  std::string file = "/home/lumj/YeeZ-Privacy-Computing/person_list";
  ypc::privacy_data_reader reader(plugin, file);

  uint64_t item_number = reader.get_item_number();
  
  std::cout << item_number << std::endl;

  uint counter = 0;
  // ypc::bytes item_data = reader.read_item_data();

  // while (!item_data.empty() && counter < item_number) {
  //   item_data = reader.read_item_data();
  //   std::cout << item_data.size() << std::endl;
  //   ++counter;
  // }

  ypc::bytes item_index_field = reader.get_item_index_field();
  while (!item_index_field.empty() && counter < item_number) {
    
    std::cout << item_index_field.size() << std::endl;
    item_index_field = reader.get_item_index_field();
    ++counter;
  }

  // for(uint64_t i = 0; i < item_number; ++i) {
  //   ypc::bytes item_index_field = reader.get_item_index_field();
  //   std::cout << item_index_field << std::endl;
  // }
  


  return 0;
}