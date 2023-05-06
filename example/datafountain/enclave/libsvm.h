#pragma once
#include "type.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/log.h"

#include <unordered_map>
#include <unordered_set>

class libsvm {
public:
  libsvm(const std::vector<train_merge_item_t> &rows) {
    for (auto &r : rows) {
      std::vector<std::string> rs;

      rs.push_back(r.get<::flag>());
      rs.push_back(r.get<::id>());

      rs.push_back(r.get<::A1>());
      rs.push_back(r.get<::A2>());
      rs.push_back(r.get<::A3>());
      rs.push_back(r.get<::A4>());
      rs.push_back(r.get<::A5>());
      rs.push_back(r.get<::A6>());

      rs.push_back(r.get<::B1>());
      rs.push_back(r.get<::B2>());
      rs.push_back(r.get<::B3>());
      rs.push_back(r.get<::B4>());
      rs.push_back(r.get<::B5>());
      rs.push_back(r.get<::B6>());
      rs.push_back(r.get<::B7>());
      rs.push_back(r.get<::B8>());
      rs.push_back(r.get<::B9>());
      rs.push_back(r.get<::B10>());
      rs.push_back(r.get<::B11>());
      rs.push_back(r.get<::B12>());
      rs.push_back(r.get<::B13>());

      rs.push_back(r.get<::C1>());
      rs.push_back(r.get<::C2>());
      rs.push_back(r.get<::C3>());
      rs.push_back(r.get<::C4>());
      rs.push_back(r.get<::C5>());
      rs.push_back(r.get<::C6>());
      rs.push_back(r.get<::C7>());

      rs.push_back(r.get<::D1>());
      rs.push_back(r.get<::D2>());
      rs.push_back(r.get<::D3>());
      rs.push_back(r.get<::D4>());
      rs.push_back(r.get<::D5>());
      rs.push_back(r.get<::D6>());
      rs.push_back(r.get<::D7>());
      rs.push_back(r.get<::D8>());
      rs.push_back(r.get<::D9>());
      rs.push_back(r.get<::D10>());
      rs.push_back(r.get<::D11>());
      rs.push_back(r.get<::D12>());
      rs.push_back(r.get<::D13>());
      m_rows.push_back(rs);
    }
  }

  libsvm(const std::vector<pred_merge_item_t> &rows) {
    for (auto &r : rows) {
      std::vector<std::string> rs;
      rs.push_back(r.get<::id>());

      rs.push_back(r.get<::A1>());
      rs.push_back(r.get<::A2>());
      rs.push_back(r.get<::A3>());
      rs.push_back(r.get<::A4>());
      rs.push_back(r.get<::A5>());
      rs.push_back(r.get<::A6>());

      rs.push_back(r.get<::B1>());
      rs.push_back(r.get<::B2>());
      rs.push_back(r.get<::B3>());
      rs.push_back(r.get<::B4>());
      rs.push_back(r.get<::B5>());
      rs.push_back(r.get<::B6>());
      rs.push_back(r.get<::B7>());
      rs.push_back(r.get<::B8>());
      rs.push_back(r.get<::B9>());
      rs.push_back(r.get<::B10>());
      rs.push_back(r.get<::B11>());
      rs.push_back(r.get<::B12>());
      rs.push_back(r.get<::B13>());

      rs.push_back(r.get<::C1>());
      rs.push_back(r.get<::C2>());
      rs.push_back(r.get<::C3>());
      rs.push_back(r.get<::C4>());
      rs.push_back(r.get<::C5>());
      rs.push_back(r.get<::C6>());
      rs.push_back(r.get<::C7>());

      rs.push_back(r.get<::D1>());
      rs.push_back(r.get<::D2>());
      rs.push_back(r.get<::D3>());
      rs.push_back(r.get<::D4>());
      rs.push_back(r.get<::D5>());
      rs.push_back(r.get<::D6>());
      rs.push_back(r.get<::D7>());
      rs.push_back(r.get<::D8>());
      rs.push_back(r.get<::D9>());
      rs.push_back(r.get<::D10>());
      rs.push_back(r.get<::D11>());
      rs.push_back(r.get<::D12>());
      rs.push_back(r.get<::D13>());
      m_rows.push_back(rs);
    }
  }

  void ignore_one_hot_for_some_columns(const std::vector<int> &columns) {
    // column idx: 10, 11, 14, 17, 19, 29, 30, 35, 38, 40
    // column: B3, B4, B7, B10, B12, D2, D3, D8, D11, D13
    // std::vector<int> columns{10, 11, 14, 17, 19, 29, 30, 35, 38, 40};
    for (auto &r : m_rows) {
      for (auto c : columns) {
        ignore_one_hot_for_column(r, c);
      }
    }
  }

  void show_raw_rows() {
    for (const auto &r : m_rows) {
      std::string s;
      for (const auto &ele : r) {
        s += ele;
        s += ',';
      }
      LOG(INFO) << s;
    }
  }

  void show_libsvm_rows() {
    for (const auto &r : m_libsvm_rows) {
      std::string s;
      for (const auto &ele : r) {
        s += std::to_string(ele.first);
        s += ":";
        s += std::to_string(ele.second);
        s += " ";
      }
      LOG(INFO) << s;
    }
  }

  const std::vector<std::vector<std::pair<int, float>>> &
  get_libsvm_rows() const {
    return m_libsvm_rows;
  }

  const std::vector<std::string> &get_ids() const { return m_ids; }

  void convert_to_libsvm(const std::vector<int> &columns, int offset) {
    // handle one-hot columns, key: column, val: all values of one column
    // std::vector<int> columns{8,  9,  12, 13, 15, 16, 18, 20,
    // 28, 31, 32, 33, 34, 36, 37, 39};
    std::unordered_map<int, std::unordered_set<std::string>> idx_val_set;
    for (auto &r : m_rows) {
      for (auto c : columns) {
        if (idx_val_set.find(c) != idx_val_set.end()) {
          if (!r[c].empty()) {
            idx_val_set[c].insert(r[c]);
          }
        } else {
          std::unordered_set<std::string> s;
          s.insert(r[c]);
          idx_val_set.insert(std::make_pair(c, s));
        }
      }
    }
    std::unordered_map<int, std::vector<std::string>> idx_val_v;
    for (auto it = idx_val_set.begin(); it != idx_val_set.end(); it++) {
      std::vector<std::string> v;
      for (auto &ele : it->second) {
        v.push_back(ele);
      }
      idx_val_v.insert(std::make_pair(it->first, v));
    }

    // record starting index if extending columns
    int s = 0;
    std::unordered_map<int, int> idx_s;
    for (int i = 0; i < m_rows[0].size(); i++) {
      if (i < 2 + offset) {
        continue;
      }
      idx_s.insert(std::make_pair(i, s));
      bool flag = false;
      for (auto c : columns) {
        if (c == i) {
          s += idx_val_v[i].size();
          flag = true;
          break;
        }
      }
      if (!flag) {
        s += 1;
      }
    }

    // output libsvm format
    for (auto &r : m_rows) {
      std::vector<std::pair<int, float>> v;
      for (int c = 0; c < r.size(); c++) {
        if (r[c].empty()) {
          continue;
        }
        if (c == 1 + offset) {
          m_ids.push_back(r[c]);
          continue;
        }
        bool flag = false;
        for (auto &col : columns) {
          if (c == col) {
            int offset = 0;
            for (auto &ele : idx_val_v[c]) {
              if (ele == r[c]) {
                break;
              }
              offset++;
            }
            v.push_back(std::make_pair(idx_s[c] + offset, 1));
            flag = true;
            break;
          }
        }
        if (!flag) {
          v.push_back(std::make_pair(idx_s[c], std::stof(r[c])));
        }
      }
      m_libsvm_rows.push_back(v);
    }
  }

  void show_libsvm() {
    for (auto &r : m_libsvm_rows) {
      std::string s;
      for (auto &ele : r) {
        s += std::to_string(ele.first);
        s += ":";
        s += std::to_string(ele.second);
        s += " ";
      }
      LOG(INFO) << s;
    }
  }

protected:
  void ignore_one_hot_for_column(std::vector<std::string> &r, int c) {
    auto &val = r[c];
    if (val.empty()) {
      return;
    }
    auto n = 1.0 * from_hex(val) / 0xffffffff;
    r[c] = std::to_string(n);
  }

  long from_hex(const std::string &s) {
    char *ptr;
    long n = strtol(s.c_str(), &ptr, 16);
    if (!ptr) {
      return -1;
    }
    return n;
  }

protected:
  std::vector<std::vector<std::string>> m_rows;
  std::vector<std::string> m_ids;
  std::vector<std::vector<std::pair<int, float>>> m_libsvm_rows;
};
