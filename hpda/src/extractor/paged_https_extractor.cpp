#include <curl/curl.h>
#include <glog/logging.h>
#include <hpda/extractor/paged_https_extractor.h>

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
  data->append((char *)ptr, size * nmemb);
  return size * nmemb;
}

namespace hpda {
namespace extractor {
namespace internal {

paged_https_extractor_impl::paged_https_extractor_impl(
    const std::string &domain, const std::string &path, int page_limit,
    const next_page_request_t &func,
    const start_index_picker_t &start_index_picker,
    const end_index_picker_t &end_index_picker)
    : m_index(0), m_page_limit(page_limit), m_domain(domain), m_path(path),
      m_next_page_request(func), m_start_index_picker(start_index_picker),
      m_end_index_picker(end_index_picker), m_reach_end(false){
  curl_global_init(CURL_GLOBAL_DEFAULT);
};

paged_https_extractor_impl::~paged_https_extractor_impl() {
  curl_global_cleanup();
}

bool paged_https_extractor_impl::process() {
  m_response.clear();
  try {
    if (m_reach_end) {
      // return false;
      return false;
    }

    std::string path =
        m_next_page_request(m_domain + m_path, m_index, m_page_limit);
    m_response = request(path);
    std::stringstream ss;
    ss << m_response;
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(ss, tree);
    m_index = m_start_index_picker(tree);
    auto end_index = m_end_index_picker(tree);
    if (end_index - m_index < m_page_limit) {
      m_reach_end = true;
    } else {
      m_reach_end = false;
    }
    m_index = end_index;
    return true;
  } catch (std::exception &e) {
    return false;
  }
  // if (m_response.empty())
  // return;
  // return;
}

std::string paged_https_extractor_impl::output_value() { return m_response; }
std::string paged_https_extractor_impl::request(const std::string &url) {

  CURL *curl;
  CURLcode res;


  curl = curl_easy_init();
  std::string response_string;
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      LOG(ERROR) << "curl_easy_perform() failed: %s\n", curl_easy_strerror(res);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return response_string;
}
} // namespace internal
} // namespace extractor
} // namespace hpda
