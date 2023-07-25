#include "ypc/core/privacy_data_reader.h"
#include "ypc/corecommon/package.h"


int main(int argc, char *argv[]) {
  std::string plugin = "/home/lumj/YeeZ-Privacy-Computing/./lib/libperson_reader_oram_debug.so";
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

  

  std::string item_index_field = reader.get_item_index_field();

  while (!item_index_field.empty() && counter < item_number) {
    
    item_index_field = reader.get_item_index_field();
    ++counter;
  }

  


  return 0;
}