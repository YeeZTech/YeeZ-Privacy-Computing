#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <hpda/extractor/internal/https_request.h>
#include <iostream>

namespace hpda {
namespace extractor {
namespace internal {
using boost::asio::ip::tcp;
class https_request_client {
public:
  https_request_client(boost::asio::io_service &io_service,
                       boost::asio::ssl::context &context,
                       const std::string &server, const std::string &path)
      : resolver_(io_service), socket_(io_service, context), server_(server) {

    if (!SSL_set_tlsext_host_name(socket_.native_handle(), server.c_str())) {
      boost::system::error_code ec((int)ERR_get_error(),
                                   boost::asio::error::get_ssl_category());
      throw boost::system::system_error(ec);
    }
    std::ostream request_stream(&request_);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    tcp::resolver::query query(server, "https");
    resolver_.async_resolve(query,
                            boost::bind(&https_request_client::handle_resolve,
                                        this, boost::asio::placeholders::error,
                                        boost::asio::placeholders::iterator));
  }

  const std::string &result() const { return m_result; }

private:
  void handle_resolve(const boost::system::error_code &err,
                      tcp::resolver::iterator endpoint_iterator) {
    if (!err) {
      socket_.set_verify_mode(boost::asio::ssl::verify_none);
      socket_.set_verify_callback(
          boost::bind(&https_request_client::verify_certificate, this, _1, _2));
      // std::cout << "server: " << server_.c_str() << std::endl;
      // socket_.set_verify_callback(
      // boost::asio::ssl::rfc2818_verification(server_.c_str()));

      boost::asio::async_connect(
          socket_.lowest_layer(), endpoint_iterator,
          boost::bind(&https_request_client::handle_connect, this,
                      boost::asio::placeholders::error));
    } else {
      LOG(ERROR) << "Error resolve: " << err.message();
      // std::cout << "Error resolve: " << err.message() << "\n";
    }
  }

  bool verify_certificate(bool preverified,
                          boost::asio::ssl::verify_context &ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    // std::cout << "Verifying " << subject_name << "\n";

    return true;
    // return preverified;
  }

  void handle_connect(const boost::system::error_code &err) {
    if (!err) {
      // std::cout << "Connect OK "
      //<< "\n";
      socket_.async_handshake(
          boost::asio::ssl::stream_base::client,
          boost::bind(&https_request_client::handle_handshake, this,
                      boost::asio::placeholders::error));
    } else {
      LOG(ERROR) << "Connect failed: " << err.message();
      // std::cout << "Connect failed: " << err.message() << "\n";
    }
  }

  void handle_handshake(const boost::system::error_code &error) {
    if (!error) {
      const char *header =
          boost::asio::buffer_cast<const char *>(request_.data());
      LOG(INFO) << "Handshake OK\nRequest: " << header;
      // std::cout << header << "\n";

      // The handshake was successful. Send the request.
      boost::asio::async_write(
          socket_, request_,
          boost::bind(&https_request_client::handle_write_request, this,
                      boost::asio::placeholders::error));
    } else {
      LOG(ERROR) << "Handshake failed: " << error.message();
    }
  }

  void handle_write_request(const boost::system::error_code &err) {
    if (!err) {
      // Read the response status line. The response_ streambuf will
      // automatically grow to accommodate the entire line. The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      boost::asio::async_read_until(
          socket_, response_, "\r\n",
          boost::bind(&https_request_client::handle_read_status_line, this,
                      boost::asio::placeholders::error));
    } else {
      LOG(ERROR) << "Error write req: " << err.message();
      // std::cout << "Error write req: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code &err) {
    if (!err) {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        LOG(ERROR) << "Invalid response";
        // std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200) {
        LOG(ERROR) << "Response returned with status code " << status_code;
        return;
      }
      LOG(ERROR) << "Status code: " << status_code;

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(
          socket_, response_, "\r\n\r\n",
          boost::bind(&https_request_client::handle_read_headers, this,
                      boost::asio::placeholders::error));
    } else {
      LOG(ERROR) << "Error: " << err.message();
    }
  }

  void handle_read_headers(const boost::system::error_code &err) {
    if (!err) {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r") {
        LOG(INFO) << header;
      }

      // Write whatever content we already have to output.
      if (response_.size() > 0) {
        LOG(INFO) << &response_;
      }

      // Start reading remaining data until EOF.
      boost::asio::async_read(
          socket_, response_, boost::asio::transfer_at_least(1),
          boost::bind(&https_request_client::handle_read_content, this,
                      boost::asio::placeholders::error));
    } else {
      LOG(ERROR) << "Error: " << err << "\n";
    }
  }

  void handle_read_content(const boost::system::error_code &error) {
    if (!error) {
      // Write all of the data that has been read so far.
      // std::cout << &response_;
      m_result += std::string((std::istreambuf_iterator<char>(&response_)),
                              std::istreambuf_iterator<char>());

      // Continue reading remaining data until EOF.
      boost::asio::async_read(
          socket_, response_, boost::asio::transfer_at_least(1),
          boost::bind(&https_request_client::handle_read_content, this,
                      boost::asio::placeholders::error));
    } else if (error != boost::asio::error::eof) {
      std::string err = error.message();
      if (error.category() == boost::asio::error::get_ssl_category()) {
        err = std::string(" (") +
              boost::lexical_cast<std::string>(ERR_GET_LIB(error.value())) +
              "," +
              boost::lexical_cast<std::string>(ERR_GET_FUNC(error.value())) +
              "," +
              boost::lexical_cast<std::string>(ERR_GET_REASON(error.value())) +
              ") ";
        // ERR_PACK /* crypto/err/err.h */
        char buf[128];
        ::ERR_error_string_n(error.value(), buf, sizeof(buf));
        err += buf;
      }
      LOG(ERROR) << "Error: " << err << "\n";
    }
  }

protected:
  tcp::resolver resolver_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
  std::string server_;
  std::string m_result;
};

https_request::https_request(const std::string &domain, const std::string &path)
    : m_ctx(boost::asio::ssl::context::sslv23), m_io_service(),
      m_domain(domain), m_path(path) {
  m_ctx.set_default_verify_paths();
  // m_ctx.set_options(boost::asio::ssl::context::default_workarounds);
}

const std::string &https_request::result() const {
  if (m_result.empty()) {
    https_request_client client(m_io_service, m_ctx, m_domain, m_path);
    m_io_service.run();
    m_result = client.result();
  }
  return m_result;
}

} // namespace internal
} // namespace extractor
} // namespace hpda
