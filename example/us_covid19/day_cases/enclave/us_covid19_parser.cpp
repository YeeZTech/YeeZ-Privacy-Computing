#include "us_covid19_parser.h"
#include "user_type.h"
#include <string>

std::vector<std::string> split(const std::string &str,
                               const std::string &delim) {
  std::vector<std::string> res;
  if ("" == str)
    return res;
  std::string::size_type s = 0;
  std::string::size_type e = str.find(delim, s);
  while (e != str.npos) {
    auto ts = std::string(&str[s], e - s);
    res.push_back(ts);
    s = e + 1;
    e = str.find(delim, s);
  }
  res.push_back(std::string(&str[s], str.size() - s));
  return res;
}

int parse_item_data(const uint8_t *data, int len, void *item) {
  user_item_t *uitem = (user_item_t *)item;
  std::string str((const char *)data, len);
  std::vector<std::string> result = split(str, ",");
  if (result.size() != 6) {
    return -1;
  }
  uitem->set<date>(result[0]);
  uitem->set<county, state, fips>(result[1], result[2], result[3]);
  uitem->set<cases, deaths>(std::stoi(result[4]), std::stoi(result[5]));
  return 0;
}
