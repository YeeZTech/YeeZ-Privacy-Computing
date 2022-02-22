
#include "common/crypto_prefix.h"
#include "corecommon/nt_cols.h"
#include "ypc/filesystem.h"
#include "ypc/ntjson.h"
#include "ypc/terminus/crypto_pack.h"
#include "ypc/terminus/interaction.h"
#include "ypc/terminus/single_data_onchain_result.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>
#include <ypc/byte.h>
#include <ypc/poption_require.h>

typedef ypc::nt<ypc::bytes> ntt;
typedef ff::util::ntobject<ntt::pkey, ntt::private_key> ypc_key_t;

ypc::bytes
get_param_privatekey(const boost::program_options::variables_map &vm);

ypc::bytes get_param_publickey(const boost::program_options::variables_map &vm);

ypc::bytes
get_param_tee_pubkey(const boost::program_options::variables_map &vm);

ypc::bytes get_param_use_param(const boost::program_options::variables_map &vm);

int gen_key(ypc::terminus::crypto_pack *crypto,
            const boost::program_options::variables_map &vm);

int decrypt_message(ypc::terminus::crypto_pack *crypto,
                    const boost::program_options::variables_map &vm);

int encrypt_message(ypc::terminus::crypto_pack *crypto,
                    const boost::program_options::variables_map &vm);

int sha3_message(ypc::terminus::crypto_pack *crypto,
                 const boost::program_options::variables_map &vm);

int forward_private_key(ypc::terminus::crypto_pack *crypto,
                        const boost::program_options::variables_map &vm);

int generate_allowance(ypc::terminus::crypto_pack *crypto,
                       const boost::program_options::variables_map &vm);

int generate_request(ypc::terminus::crypto_pack *crypto,
                     const boost::program_options::variables_map &vm);

int gen_relay_result_proof(ypc::terminus::crypto_pack *crypto,
                           const boost::program_options::variables_map &vm);

std::tuple<boost::program_options::variables_map,
           std::function<uint32_t(ypc::terminus::crypto_pack *crypto)>>
parse_command_line(int argc, char *argv[]);
