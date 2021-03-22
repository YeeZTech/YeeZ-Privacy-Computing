#include "iris_parser.h"
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

int parse_item_data(const char *data, int len, void *item) {
  user_item_t *uitem = (user_item_t *)item;
  std::string str(data, len);
  std::vector<std::string> result = split(str, ",");
  // boost::split(result, str, boost::is_any_of(","));
  if (result.size() != 5) {
    return -1;
  }
  iris_data_t d;
  d.set<sepal_len, sepal_wid>(std::stod(result[0]), std::stod(result[1]));
  d.set<petal_len, petal_wid>(std::stod(result[2]), std::stod(result[3]));
  uitem->set<iris_data>(d);
  uitem->set<species>(result[4]);
  return 0;
}
