#include "ypc/common/crypto_prefix.h"
#include "ypc/terminus/crypto_pack.h"
#include "ypc/terminus/enclave_interaction.h"
#include "ypc/terminus/interaction.h"
#include "ypc/terminus/single_data_onchain_result.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;

enum crypto_pack_type { intel_sgx_and_eth, chinese_sm };
enum crypto_prefix_type {
  forward = ypc::utc::crypto_prefix_forward,
  arbitrary = ypc::utc::crypto_prefix_arbitrary
};

class crypto_pack_wrapper {
public:
  crypto_pack_wrapper(crypto_pack_type cpt) : m_type(cpt) {
    switch (cpt) {
    case crypto_pack_type::intel_sgx_and_eth:
      m_crypto = ypc::terminus::intel_sgx_and_eth_compatible();
      break;
    case crypto_pack_type::chinese_sm:
      m_crypto = ypc::terminus::sm_compatible();
      break;
    default:
      throw std::runtime_error("not support type");
    }
  }

  ypc::bytes gen_ecc_private_key() {
    auto s = m_crypto->gen_ecc_private_key();
    return s;
  }
  ypc::bytes
  gen_ecc_public_key_from_private_key(const ypc::bytes &private_key) {
    auto s = m_crypto->gen_ecc_public_key_from_private_key(private_key);
    return s;
  }
  ypc::bytes hash_256(const ypc::bytes &msg) { return m_crypto->hash_256(msg); }

  ypc::bytes encrypt(const ypc::bytes &msg, const ypc::bytes &public_key,
                     uint32_t prefix) {
    return m_crypto->ecc_encrypt(msg, public_key, prefix);
  }

  ypc::bytes decrypt(const ypc::bytes &msg, const ypc::bytes &private_key,
                     uint32_t prefix) {
    return m_crypto->ecc_decrypt(msg, private_key, prefix);
  }

  ypc::bytes sign(const ypc::bytes &msg, const ypc::bytes &private_key) {
    return m_crypto->sign_message(msg, private_key);
  }

  bool verify_sig(const ypc::bytes &sig, const ypc::bytes &msg,
                  const ypc::bytes &public_key) {
    return m_crypto->verify_message_signature(sig, msg, public_key);
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
    ypc::bytes encrypted_skey;
    ypc::bytes signature;
  } forward_info;

  enclave_interaction_wrapper(std::shared_ptr<crypto_pack_wrapper> crypto)
      : m_crypto(crypto), m_instance(crypto.get()->m_crypto.get()) {}

  ypc::bytes generate_allowance(const ypc::bytes &private_key,
                                const ypc::bytes &param_hash,
                                const ypc::bytes &target_enclave_hash,
                                const ypc::bytes &dian_pkey,
                                const ypc::bytes &dhash) {
    auto r = m_instance.generate_allowance(
        private_key, param_hash, target_enclave_hash, dian_pkey, dhash);
    return r;
  }

  forward_info forward_private_key(const ypc::bytes &private_key,
                                   const ypc::bytes &dian_pkey,
                                   const ypc::bytes &enclave_hash) {
    auto r =
        m_instance.forward_private_key(private_key, dian_pkey, enclave_hash);
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

  ypc::bytes generate_request(const ypc::bytes &param,
                              const ypc::bytes &public_key) {
    auto r = m_instance.generate_request(param, public_key);
    return r;
  }

  ypc::bytes decrypt_result(const ypc::bytes &result,
                            const ypc::bytes &private_key) {
    auto r = m_instance.decrypt_result(result, private_key);
    return r;
  }

protected:
  std::shared_ptr<crypto_pack_wrapper> m_crypto;
  ypc::terminus::single_data_onchain_result m_instance;
};

PYBIND11_MODULE(pyterminus, m) {
  m.doc() = "Python module of Fidelius toolkit - terminus";

  py::class_<ypc::bytes>(m, "YPCBytes", py::buffer_protocol())
      .def_buffer([](ypc::bytes &m) -> py::buffer_info {
        return py::buffer_info(m.data(), sizeof(uint8_t),
                               py::format_descriptor<uint8_t>::format(), 1,
                               {m.size()}, {1});
      })
      .def(py::init([](py::buffer b) -> ypc::bytes {
        py::buffer_info info = b.request();
        if (info.format != py::format_descriptor<uint8_t>::format()) {
          throw std::runtime_error(
              "Incompatiable format: expected a byte array!");
        }
        return ypc::bytes((uint8_t *)info.ptr, info.shape[0]);
      }))
      .def("__len__", &ypc::bytes::size)
      .def("__str__", [](const ypc::bytes &b) {
        auto k = b.as<ypc::bytes::hex_bytes_t>();
        return std::string((const char *)k.data(), k.size());
      });

  py::enum_<crypto_pack_type>(m, "CryptoPackType")
      .value("IntelSGXAndEthCompatible", crypto_pack_type::intel_sgx_and_eth)
      .value("ChineseSM", crypto_pack_type::chinese_sm)
      .export_values();
  py::enum_<crypto_prefix_type>(m, "CryptoPrefixType")
      .value("forward", crypto_prefix_type::forward)
      .value("arbitrary", crypto_prefix_type::arbitrary)
      .export_values();

  py::class_<crypto_pack_wrapper, std::shared_ptr<crypto_pack_wrapper>>(
      m, "CryptoPack")
      .def(py::init<crypto_pack_type>())
      .def("gen_ecc_private_key", &crypto_pack_wrapper::gen_ecc_private_key)
      .def("gen_ecc_public_key_from_private_key",
           &crypto_pack_wrapper::gen_ecc_public_key_from_private_key)
      .def("hash_256", &crypto_pack_wrapper::hash_256)
      .def("encrypt", &crypto_pack_wrapper::encrypt)
      .def("decrypt", &crypto_pack_wrapper::decrypt)
      .def("sign", &crypto_pack_wrapper::sign)
      .def("verify_sig", &crypto_pack_wrapper::verify_sig);

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
