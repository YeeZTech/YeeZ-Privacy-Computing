#pragma once

namespace ypc {
class version {
public:
  inline version() { m_data.m_data = 0; }
  version(uint32_t data) { m_data.m_data = data; };
  version(uint16_t major_version, uint8_t minor_version,
          uint8_t patch_version) {
    m_data.m_detail.m_major_version = major_version;
    m_data.m_detail.m_minor_version = minor_version;
    m_data.m_detail.m_patch_version = patch_version;
  }

  version(const version &) = default;
  version &operator=(const version &) = default;

  inline uint16_t major_version() const {
    return m_data.m_detail.m_major_version;
  }
  inline uint8_t minor_version() const {
    return m_data.m_detail.m_minor_version;
  }
  inline uint8_t patch_version() const {
    return m_data.m_detail.m_patch_version;
  }
  inline uint16_t &major_version() { return m_data.m_detail.m_major_version; }
  inline uint8_t &minor_version() { return m_data.m_detail.m_minor_version; }
  inline uint8_t &patch_version() { return m_data.m_detail.m_patch_version; }

  inline uint32_t data() const { return m_data.m_data; }

  friend inline bool operator<(const version &v1, const version &v2) {
    if (v1.m_data.m_detail.m_major_version <
        v2.m_data.m_detail.m_major_version) {
      return true;
    } else if (v1.m_data.m_detail.m_minor_version <
               v2.m_data.m_detail.m_minor_version) {
      return true;
    } else if (v1.m_data.m_detail.m_patch_version <
               v2.m_data.m_detail.m_patch_version) {
      return true;
    }
    return false;
  }

  friend inline bool operator>(const version &v1, const version &v2) {
    return v2 < v1;
  }

  friend inline bool operator>=(const version &v1, const version &v2) {
    return !(v1 < v2);
  }

  friend inline bool operator<=(const version &v1, const version &v2) {
    return !(v1 > v2);
  }

  friend inline bool operator==(const version &v1, const version &v2) {
    return v1 >= v2 && v2 >= v1;
  }

  friend inline bool operator!=(const version &v1, const version &v2) {
    return !(v1 == v2);
  }

protected:
  union _version_data {
    uint32_t m_data;
    struct _version_detail {
      uint16_t m_major_version;
      uint8_t m_minor_version;
      uint8_t m_patch_version;
    };
    _version_detail m_detail;
  };

  _version_data m_data;
}; // end class version
} // namespace ypc
