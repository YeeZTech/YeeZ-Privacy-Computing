#include "cmd_line.h"

std::tuple<boost::program_options::variables_map,
           std::function<uint32_t(ypc::terminus::crypto_pack *crypto)>>
parse_command_line(int argc, char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Terminus");
  bp::options_description general("General Options");
  bp::options_description key("ECC Key related operations");
  bp::options_description relay("Relay result to another enclave");
  bp::options_description request("Generate Request");
  bp::options_description forward("Generate Shu private key forward");
  bp::options_description allowance("Generate allowance");
  bp::options_description encrypt("Encrypt message");
  bp::options_description decrypt("Decrypt message");
  bp::options_description sha3("Sha3 message");

  // clang-format off
  general.add_options()
    ("gen-key", "generate an ECC key pair to encrypt/decrypt request")
    ("request", "generate request")
    ("forward", "forward private key to enclave")
    ("allowance", "generate allowance for param")
    ("encrypt", "to encrypt message ")
    ("decrypt", "to decrypt message ")
    ("sha3", "to SHA3-256 message ")
    ("relay", "generate relay info")
    ("help", "help message");

  key.add_options()
    ("no-password", "no password when gen-key")
    ("output", bp::value<std::string>(), "output result to file with JSON format");


  forward.add_options()
    ("tee-pubkey", bp::value<std::string>()->required(), "TEE public key, or Dian public key")
    ("use-privatekey-file", bp::value<std::string>(), "local (Shu) private key file")
    ("use-privatekey-hex", bp::value<std::string>(), "local (Shu) private key hex")
    ("use-enclave-hash", bp::value<std::string>(), "target enclave hash. It means 'any enclave' if absence.")
    ("output", bp::value<std::string>(), "output result to file with JSON format");


  allowance.add_options()
    ("tee-pubkey", bp::value<std::string>()->required(), "TEE public key, or Dian public key")
    ("use-param", bp::value<std::string>()->required(), "param hash")
    ("use-privatekey-file", bp::value<std::string>(), "local (Shu) private key file")
    ("use-privatekey-hex", bp::value<std::string>(), "local (Shu) private key hex")
    ("dhash", bp::value<std::string>()->required(), "data hash, or model hash")
    ("use-enclave-hash", bp::value<std::string>()->required(), "enclave hash")
    ("output", bp::value<std::string>(), "output result to file with JSON format");


  encrypt.add_options()
    ("use-publickey-file", bp::value<std::string>(), "local (Shu) public key file")
    ("use-publickey-hex", bp::value<std::string>(), "local (Shu) public key hex")
    ("use-param", bp::value<std::string>()->required(), "message to encrypt")
    ("param-format", bp::value<std::string>()->default_value("hex"), "param format, [ hex | text ]")
    ("output", bp::value<std::string>(), "output result to file with JSON format");

  decrypt.add_options()
    ("use-privatekey-file", bp::value<std::string>(), "local (Shu) private key file")
    ("use-privatekey-hex", bp::value<std::string>(), "local (Shu) private key hex")
    ("use-param", bp::value<std::string>()->required(), "message to decrypt")
    ("param-format", bp::value<std::string>()->default_value("hex"), "param format, [ hex | text ]")
    ("output", bp::value<std::string>()->multitoken(), "output result to file with JSON format");

  sha3.add_options()
    ("use-param", bp::value<std::string>()->required(), "data to sha")
    ("param-format", bp::value<std::string>()->default_value("hex"), "param format, [ hex | text ]")
    ("output", bp::value<std::string>(), "output result to file with JSON format");

  request.add_options()
    ("use-param", bp::value<std::string>()->required(), "param data")
    ("param-format", bp::value<std::string>()->default_value("hex"), "param format, [ hex | text ]")
    ("use-publickey-file", bp::value<std::string>(), "local (Shu) public key file")
    ("use-publicey-hex", bp::value<std::string>(), "local (Shu) public key hex")
    ("output", bp::value<std::string>(), "output result to file with JSON format");

  relay.add_options()
    ("use-param", bp::value<std::string>()->required(), "param data")
    ("param-format", bp::value<std::string>()->default_value("hex"), "param format, [ hex | text ]")
    ("relay-tee-pubkey", bp::value<std::string>()->required(), "relay TEE public key, or Dian public key")
    ("relay-enclave-hash", bp::value<std::string>()->required(), "relay enclave hash")
    ("target-tee-pubkey", bp::value<std::string>()->required(), "target TEE public key, or Dian public key")
    ("target-enclave-hash", bp::value<std::string>()->required(), "target enclave hash")
    ("use-privatekey-file", bp::value<std::string>(), "local (Shu) private key file")
    ("use-privatekey-hex", bp::value<std::string>(), "local (Shu) private key hex")
    ("output", bp::value<std::string>(), "output result to file with JSON format");

  // clang-format on

  all.add(general)
      .add(key)
      .add(forward)
      .add(allowance)
      .add(relay)
      .add(encrypt)
      .add(decrypt)
      .add(sha3)
      .add(request);
  bp::variables_map vm;
  auto parsedOptions = bp::command_line_parser(argc, argv)
                           .options(general)
                           .allow_unregistered()
                           .run();
  bp::store(parsedOptions, vm);
  bp::notify(vm);
  typedef std::tuple<
      std::string, bp::options_description,
      std::function<int(ypc::terminus::crypto_pack *,
                        const boost::program_options::variables_map &)>>
      next_desc_item;

  std::vector<next_desc_item> nds;
  nds.push_back({"gen-key", key, gen_key});
  nds.push_back({"request", request, generate_request});
  nds.push_back({"decrypt", decrypt, decrypt_message});
  nds.push_back({"forward", forward, forward_private_key});
  nds.push_back({"encrypt", encrypt, encrypt_message});
  nds.push_back({"allowance", allowance, generate_allowance});
  nds.push_back({"sha3", sha3, sha3_message});
  nds.push_back({"relay", relay, gen_relay_result_proof});

  for (auto it : nds) {
    if (vm.count(std::get<0>(it))) {
      auto unregistered = bp::collect_unrecognized(parsedOptions.options,
                                                   bp::include_positional);
      auto parsed_options =
          bp::command_line_parser(unregistered).options(std::get<1>(it)).run();
      bp::variables_map specific_vm;
      bp::store(parsed_options, specific_vm);
      bp::notify(specific_vm);
      return {vm,
              std::bind(std::get<2>(it), std::placeholders::_1, specific_vm)};
    }
  }

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }
  std::cerr << "no options specified" << std::endl;
  exit(-1);
}
