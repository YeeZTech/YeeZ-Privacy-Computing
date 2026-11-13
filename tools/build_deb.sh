#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/package-release}"
DIST_DIR="${DIST_DIR:-${ROOT_DIR}/dist}"
STAGE_DIR=""
SIGNING_DIR=""
PREPARE_SIGNING_DIR=""

# These packages are intentionally absent from the generated Depends field.
# The target machine owner installs the Intel SGX SDK and PSW separately.
SGX_PACKAGES=(
  sgx-sdk
  sgx-aesm-service
  libsgx-ae-id-enclave
  libsgx-ae-le
  libsgx-dcap-default-qpl
  libsgx-dcap-ql
  libsgx-dcap-quote-verify
  libsgx-enclave-common
  libsgx-launch
  libsgx-uae-service
  libsgx-ukey-exchange
  libsgx-urts
)

SIGNED_ENCLAVES=(keymgr keymgr_gmssl)
SIGNED_ENCLAVE_CONFIGS=(
  "ypc/keymgr/default/enclave/ekeymgr.config.xml"
  "ypc/keymgr/default/enclave/ekeymgr.config.xml"
)

cleanup() {
  [[ -z "${STAGE_DIR}" ]] || rm -rf "${STAGE_DIR}"
}
trap cleanup EXIT

fail() {
  echo "ERROR: $*" >&2
  exit 1
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || fail "missing required command: $1"
}

usage() {
  cat <<'EOF'
Usage:
  tools/build_deb.sh --prepare-signing DIR
  tools/build_deb.sh --signing-dir DIR

--prepare-signing DIR  Build the Release enclave hashes and copy the hashes
                       required for offline signing into DIR.
--signing-dir DIR      Use public_key.pem and *_sig.hex files in DIR to create
                       signed YPC enclaves and build the Debian package.
EOF
}

while (($# > 0)); do
  case "$1" in
    --prepare-signing)
      shift
      (($# > 0)) || fail "missing directory after --prepare-signing"
      PREPARE_SIGNING_DIR="$1"
      ;;
    --signing-dir)
      shift
      (($# > 0)) || fail "missing directory after --signing-dir"
      SIGNING_DIR="$1"
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      fail "unknown argument: $1"
      ;;
  esac
  shift
done

[[ -z "${PREPARE_SIGNING_DIR}" || -z "${SIGNING_DIR}" ]] || \
  fail "--prepare-signing and --signing-dir cannot be used together"
[[ -n "${PREPARE_SIGNING_DIR}${SIGNING_DIR}" ]] || \
  fail "specify --prepare-signing or --signing-dir"

for command in cmake cpack dpkg dpkg-deb dpkg-shlibdeps file find grep mktemp paste install readelf sed; do
  require_command "${command}"
done

[[ "$(dpkg --print-architecture)" == "amd64" ]] || \
  fail "this package is supported only on amd64"

SGX_SDK_PATH="${SGX_SDK:-/opt/intel/sgxsdk}"
[[ -x "${SGX_SDK_PATH}/bin/x64/sgx_sign" ]] || \
  fail "Intel SGX SDK signer not found at ${SGX_SDK_PATH}/bin/x64/sgx_sign"
[[ -x "${SGX_SDK_PATH}/bin/x64/sgx_edger8r" ]] || \
  fail "Intel SGX SDK edger8r not found at ${SGX_SDK_PATH}/bin/x64/sgx_edger8r"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${BUILD_DIR}" --parallel

if [[ -n "${PREPARE_SIGNING_DIR}" ]]; then
  mkdir -p "${PREPARE_SIGNING_DIR}"
  for enclave in "${SIGNED_ENCLAVES[@]}"; do
    hash_file="${ROOT_DIR}/lib/${enclave}_hash.hex"
    [[ -f "${hash_file}" ]] || fail "missing generated enclave hash: ${hash_file}"
    install -m 0644 "${hash_file}" "${PREPARE_SIGNING_DIR}/${enclave}_hash.hex"
  done
  echo "Signing hashes written to ${PREPARE_SIGNING_DIR}"
  exit 0
fi

[[ -f "${SIGNING_DIR}/public_key.pem" ]] || \
  fail "missing signing public key: ${SIGNING_DIR}/public_key.pem"
for index in "${!SIGNED_ENCLAVES[@]}"; do
  enclave="${SIGNED_ENCLAVES[${index}]}"
  config="${ROOT_DIR}/${SIGNED_ENCLAVE_CONFIGS[${index}]}"
  unsigned_enclave="${ROOT_DIR}/lib/lib${enclave}.so"
  hash_file="${ROOT_DIR}/lib/${enclave}_hash.hex"
  signature_file="${SIGNING_DIR}/${enclave}_sig.hex"

  [[ -f "${unsigned_enclave}" ]] || fail "missing unsigned enclave: ${unsigned_enclave}"
  [[ -f "${hash_file}" ]] || fail "missing generated enclave hash: ${hash_file}"
  [[ -f "${signature_file}" ]] || fail "missing enclave signature: ${signature_file}"
  "${SGX_SDK_PATH}/bin/x64/sgx_sign" catsig \
    -enclave "${unsigned_enclave}" \
    -config "${config}" \
    -out "${ROOT_DIR}/lib/${enclave}.signed.so" \
    -key "${SIGNING_DIR}/public_key.pem" \
    -sig "${signature_file}" \
    -unsigned "${hash_file}"
done

STAGE_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ypc-deb-stage.XXXXXX")"
DESTDIR="${STAGE_DIR}" cmake --install "${BUILD_DIR}"

mapfile -d '' ELF_FILES < <(
  while IFS= read -r -d '' path; do
    if file -Lb "${path}" | grep -q 'ELF.*dynamically linked'; then
      printf '%s\0' "${path}"
    fi
  done < <(find "${STAGE_DIR}/usr" -type f -print0)
)

(( ${#ELF_FILES[@]} > 0 )) || fail "no dynamically linked ELF files were staged"

SHLIBDEPS_ARGS=(-O "-l${STAGE_DIR}/usr/lib")
SHLIBDEPS_ARGS+=(-xypc)
for package in "${SGX_PACKAGES[@]}"; do
  SHLIBDEPS_ARGS+=("-x${package}")
done

pushd "${STAGE_DIR}" >/dev/null
mkdir -p debian DEBIAN
: > debian/control

# dpkg-shlibdeps must recognize libraries supplied by this same package before
# it can strictly resolve the remaining system libraries. -xypc suppresses the
# otherwise redundant dependency generated from this file.
LOCAL_SHLIBS_FILE="${STAGE_DIR}/debian/ypc.shlibs"
while IFS= read -r -d '' shared_library; do
  if ! file -Lb "${shared_library}" | grep -q 'ELF.*shared object'; then
    continue
  fi

  soname="$(readelf -d "${shared_library}" 2>/dev/null | sed -n 's/.*SONAME.*\[\([^]]*\)\].*/\1/p')"
  [[ "${soname}" == *.so.* ]] || continue
  library_name="${soname%%.so.*}"
  soname_version="${soname#*.so.}"
  printf '%s %s ypc\n' "${library_name}" "${soname_version}" >> "${LOCAL_SHLIBS_FILE}"
done < <(find "${STAGE_DIR}/usr/lib" -type f -print0)

[[ -s "${LOCAL_SHLIBS_FILE}" ]] || fail "no YPC shared-library SONAMEs were staged"
SHLIBDEPS_ARGS+=("-L${LOCAL_SHLIBS_FILE}")
SHLIBDEPS_OUTPUT="$(dpkg-shlibdeps "${SHLIBDEPS_ARGS[@]}" "${ELF_FILES[@]}")"
popd >/dev/null

PACKAGE_DEPENDS="$(sed -n 's/^shlibs:Depends=//p' <<<"${SHLIBDEPS_OUTPUT}" | paste -sd, -)"

mkdir -p "${DIST_DIR}"
rm -f "${DIST_DIR}/ypc_"*.deb

CPACK_ARGS=(
  --config "${BUILD_DIR}/CPackConfig.cmake"
  -G DEB
  -B "${DIST_DIR}"
)
if [[ -n "${PACKAGE_DEPENDS}" ]]; then
  CPACK_ARGS+=(-D "CPACK_DEBIAN_PACKAGE_DEPENDS=${PACKAGE_DEPENDS}")
fi
cpack "${CPACK_ARGS[@]}"

shopt -s nullglob
DEB_PACKAGES=("${DIST_DIR}"/ypc_*.deb)
shopt -u nullglob
(( ${#DEB_PACKAGES[@]} == 1 )) || \
  fail "expected one package under ${DIST_DIR}, found ${#DEB_PACKAGES[@]}"
DEB_PACKAGE="${DEB_PACKAGES[0]}"

PACKAGE_NAME="$(dpkg-deb -f "${DEB_PACKAGE}" Package)"
PACKAGE_VERSION="$(dpkg-deb -f "${DEB_PACKAGE}" Version)"
PACKAGE_ARCH="$(dpkg-deb -f "${DEB_PACKAGE}" Architecture)"
[[ "${PACKAGE_NAME}" == "ypc" ]] || fail "unexpected package name: ${PACKAGE_NAME}"
[[ "${PACKAGE_ARCH}" == "amd64" ]] || fail "unexpected package architecture: ${PACKAGE_ARCH}"

PACKAGE_DEPENDS_FIELD="$(dpkg-deb -f "${DEB_PACKAGE}" Depends 2>/dev/null || true)"
if grep -Eiq '(^|[ ,|])[^,| ]*(sgx|aesm)[^,| ]*' <<<"${PACKAGE_DEPENDS_FIELD}"; then
  fail "package Depends must not include Intel SGX packages: ${PACKAGE_DEPENDS_FIELD}"
fi

PACKAGE_CONTENTS="$(dpkg-deb -c "${DEB_PACKAGE}")"
if grep -Eq '/(libsgx[^/]*\.so[^/]*|sgx_sign|sgx_edger8r)( |$)' <<<"${PACKAGE_CONTENTS}"; then
  fail "package must not contain Intel SGX SDK or PSW binaries"
fi
grep -Eq ' ./usr/include/ypc/' <<<"${PACKAGE_CONTENTS}" || \
  fail "package is missing YPC headers"
grep -Eq ' ./usr/lib(/[^/]+)?/cmake/YPC/YPCConfig\.cmake$' <<<"${PACKAGE_CONTENTS}" || \
  fail "package is missing its CMake package configuration"
grep -Eq ' ./usr/lib/.*\.signed\.so$' <<<"${PACKAGE_CONTENTS}" || \
  fail "package is missing a signed enclave"

echo "Deb package: ${DEB_PACKAGE}"
echo "Version: ${PACKAGE_VERSION}"
if [[ -n "${PACKAGE_DEPENDS_FIELD}" ]]; then
  echo "Depends: ${PACKAGE_DEPENDS_FIELD}"
fi
