#include "cmd_line.h"
#include "ypc/terminus/enclave_interaction.h"

int sign_message(ypc::terminus::crypto_pack *crypto,
                        const boost::program_options::variables_map &vm) {
  ypc::bytes message = get_param_use_param(vm);
  ypc::bytes private_key = get_param_privatekey(vm);
  ypc::bytes signature = crypto->sign_message(message, private_key);

  std::cout << "message " << message << std::endl;
  std::cout << "signature " << signature << std::endl;

  return 0;
}
