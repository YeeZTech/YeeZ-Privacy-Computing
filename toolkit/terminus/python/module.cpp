#include "ypc/terminus/crypto_pack.h"
#include "ypc/terminus/enclave_interaction.h"
#include "ypc/terminus/interaction.h"
#include "ypc/terminus/single_data_onchain_result.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;

enum crypto_pack_type { intel_sgx_and_eth, chinese_sm };

auto cast = [](const py::bytes &p) -> ypc::bytes {
  std::string b = std::string(p);
  ypc::bytes sk(b.c_str(), b.size());
  return sk;
};
class crypto_pack_wrapper {
public:
  crypto_pack_wrapper(crypto_pack_type cpt) : m_type(cpt) {
    switch (cpt) {
    case crypto_pack_type::intel_sgx_and_eth:
      m_crypto = ypc::terminus::intel_sgx_and_eth_compatible();
      break;
    default:
      throw std::runtime_error("not support type");
    }
  }

  py::bytes gen_ecc_private_key() {
    auto s = m_crypto->gen_ecc_private_key();
    return py::bytes((const char *)s.data(), s.size());
  }
  py::bytes gen_ecc_public_key_from_private_key(const py::bytes &private_key) {
    std::string b = std::string(private_key);
    ypc::bytes sk(b.c_str(), b.size());
    auto s = m_crypto->gen_ecc_public_key_from_private_key(sk);
    return py::bytes((const char *)s.data(), s.size());
  }

  std::unique_ptr<ypc::terminus::crypto_pack> m_crypto;

protected:
  crypto_pack_type m_type;
};

class enclave_interaction_wrapper {
public:
  typedef struct _forward {
    inline _forward(const ypc::bytes &_encrypted_skey,
                    const ypc::bytes &_signature)
        : encrypted_skey((const char *)_encrypted_skey.data(),
                         _encrypted_skey.size()),
          signature((const char *)_signature.data(), _signature.size()) {}
    py::bytes encrypted_skey;
    py::bytes signature;
  } forward_info;

  enclave_interaction_wrapper(std::shared_ptr<crypto_pack_wrapper> crypto)
      : m_crypto(crypto), m_instance(crypto.get()->m_crypto.get()) {}

  py::bytes generate_allowance(const py::bytes &private_key,
                               const py::bytes &param_hash,
                               const py::bytes &target_enclave_hash,
                               const py::bytes &dian_pkey,
                               const py::bytes &dhash) {
    auto r = m_instance.generate_allowance(cast(private_key), cast(param_hash),
                                           cast(target_enclave_hash),
                                           cast(dian_pkey), cast(dhash));
    return py::bytes((const char *)r.data(), r.size());
  }

  forward_info forward_private_key(const py::bytes &private_key,
                                   const py::bytes &dian_pkey,
                                   const py::bytes &enclave_hash){
    auto r = m_instance.forward_private_key(cast(private_key), cast(dian_pkey),
                                            cast(enclave_hash));
    return forward_info(r.encrypted_skey, r.signature);
  }

protected:
  std::shared_ptr<crypto_pack_wrapper> m_crypto;
  ypc::terminus::enclave_interaction m_instance;
};

class single_data_onchain_result_wrapper {
public:
  single_data_onchain_result_wrapper(
      std::shared_ptr<crypto_pack_wrapper> crypto)
      : m_crypto(crypto), m_instance(crypto.get()->m_crypto.get()) {}

  py::bytes generate_request(const py::bytes &param,
                             const py::bytes &public_key) {
    auto r = m_instance.generate_request(cast(param), cast(public_key));
    return py::bytes((const char *)r.data(), r.size());
  }

  py::bytes decrypt_result(const py::bytes &result,
                           const py::bytes &private_key) {
    auto r = m_instance.decrypt_result(cast(result), cast(private_key));
    return py::bytes((const char *)r.data(), r.size());
  }

protected:
  std::shared_ptr<crypto_pack_wrapper> m_crypto;
  ypc::terminus::single_data_onchain_result m_instance;
};

PYBIND11_MODULE(pyterminus, m) {
  m.doc() = "Python module of Fidelius toolkit - terminus";
  py::enum_<crypto_pack_type>(m, "CryptoPackType")
      .value("IntelSGXAndEthCompatible", crypto_pack_type::intel_sgx_and_eth)
      .value("ChineseSM", crypto_pack_type::chinese_sm)
      .export_values();

  py::class_<crypto_pack_wrapper, std::shared_ptr<crypto_pack_wrapper>>(
      m, "CryptoPack")
      .def(py::init<crypto_pack_type>())
      .def("gen_ecc_private_key", &crypto_pack_wrapper::gen_ecc_private_key)
      .def("gen_ecc_public_key_from_private_key",
           &crypto_pack_wrapper::gen_ecc_public_key_from_private_key);

  py::class_<single_data_onchain_result_wrapper,
             std::shared_ptr<single_data_onchain_result_wrapper>>
      sdor(m, "SingleDataOnchainResult");
  sdor.def(py::init<std::shared_ptr<crypto_pack_wrapper>>())
      .def("generate_request",
           &single_data_onchain_result_wrapper::generate_request,
           "Generate request to process", py::arg("param"),
           py::arg("public_key"))
      .def("decrypt_result",
           &single_data_onchain_result_wrapper::decrypt_result,
           "Decrypt result", py::arg("result"), py::arg("private_key"));

  py::class_<enclave_interaction_wrapper,
             std::shared_ptr<enclave_interaction_wrapper>>
      edor(m, "EnclaveInteraction");
  edor.def(py::init<std::shared_ptr<crypto_pack_wrapper>>())
      .def("generate_allowance",
           &enclave_interaction_wrapper::generate_allowance,
           "Generate allowance", py::arg("private_key"), py::arg("param_hash"),
           py::arg("target_enclave_hash"), py::arg("dian_pkey"),
           py::arg("dhash"))
      .def("forward_private_key",
           &enclave_interaction_wrapper::forward_private_key,
           "Forward private (Shu) key", py::arg("private_key"),
           py::arg("dian_pkey"), py::arg("target_enclave_hash"));
  py::class_<enclave_interaction_wrapper::forward_info>(
      edor, "EnclaveInteractionForwardInfo");
}
