#ifndef SENDER_INCLUDE_CONNECTION_HPP
#define SENDER_INCLUDE_CONNECTION_HPP

#include <resource/config.hpp>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace server {
  using boost::asio::ip::tcp;
  struct Size {
    size_t width;
    size_t height;
  };
  std::vector<unsigned char> generate_image(const Size& size);
  std::string make_daytime_string();

  class Connection
    : public boost::enable_shared_from_this<Connection>
  {
  public:
    using pointer = boost::shared_ptr<Connection>;

    static pointer create(boost::asio::io_context& io_context, const resource::Config& config);

    tcp::socket& socket();
    void start();
    ~Connection();

  private:
    Connection(boost::asio::io_context& io_context, const resource::Config& config);
    void handle_write(const boost::system::error_code& error,
      size_t /*bytes_transferred*/);
    resource::Config config_;
    std::atomic_int tick_count_;
    tcp::socket socket_;
  };
}

#endif // SENDER_INCLUDE_CONNECTION_HPP
