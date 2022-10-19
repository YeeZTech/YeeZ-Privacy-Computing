#include "cmd_line.h"

int getch() {
  int ch;
  struct termios t_old, t_new;

  tcgetattr(STDIN_FILENO, &t_old);
  t_new = t_old;
  t_new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
  return ch;
}

std::string getpass(const char *prompt, bool show_asterisk = true) {
  const char BACKSPACE = 127;
  const char RETURN = 10;

  std::string password;
  unsigned char ch = 0;

  std::cout << prompt << std::endl;

  while ((ch = getch()) != RETURN) {
    if (ch == BACKSPACE) {
      if (password.length() != 0) {
        if (show_asterisk)
          std::cout << "\b \b";
        password.resize(password.length() - 1);
      }

    } else {
      password += ch;
      if (show_asterisk)
        std::cout << '*';
    }
  }
  std::cout << std::endl;
  return password;
}

int gen_key(ypc::terminus::crypto_pack *crypto,
            const boost::program_options::variables_map &vm) {
  std::string password;

  if (!vm.count("no-password")) {
    // TODO we should encrypt the private key with password
    std::cerr << "We do not support password mode yet." << std::endl;
    exit(-1);
  }

  if (!vm.count("no-password")) {
    password = getpass("Please enter password:", false);
    std::string repass = getpass("Please enter password again: ", false);
    if (password != repass) {
      std::cerr << "Incorrect password. " << std::endl;
      exit(-1);
    }
  }
  auto private_key = crypto->gen_ecc_private_key();
  if (private_key.size() == 0) {
    std::cerr << "failed to generate private key" << std::endl;
    return -1;
  }
  auto public_key = crypto->gen_ecc_public_key_from_private_key(private_key);
  if (public_key.size() == 0) {
    std::cerr << "failed to generate public key " << std::endl;
    return -1;
  }
  ypc_key_t k;
  k.set<ntt::pkey, ntt::private_key>(public_key, private_key);

  if (!vm.count("output")) {
    std::cout << ypc::ntjson::to_json(k) << std::endl;
  } else {

    std::string output_path =
        ypc::complete_path(vm["output"].as<std::string>());

    ypc::ntjson::to_json_file(k, output_path);
  }
  return 0;
}
