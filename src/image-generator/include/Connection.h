#ifndef SENDER_INCLUDE_CONNECTION_HPP
#define SENDER_INCLUDE_CONNECTION_HPP

#include <resource/config.hpp>
#include <async/TimerThread.h>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace server {

  struct Record {
    int total_frame_send;
    float real_fps;

  };

  using boost::asio::ip::tcp;
  struct Size {
    int width;
    int height;
  };
  std::vector<unsigned char> generate_image(const Size& size);
  std::string make_daytime_string();

  class Connection
    : public boost::enable_shared_from_this<Connection>
  {
  public:
    using pointer = boost::shared_ptr<Connection>;
    using HandlerError = std::function<void(const boost::system::error_code&)>;

    static pointer create(boost::asio::io_context& io_context, const resource::Config& config, HandlerError&&);

    tcp::socket& socket();
    void start();
    ~Connection();

  private:
    Connection(boost::asio::io_context& io_context, const resource::Config& config, HandlerError&&);

    resource::Config config_;
    std::atomic_int tick_count_;
    tcp::socket socket_;

    std::unique_ptr<async::TimerThread>   generator_;
    std::unique_ptr<async::TimerThread> statistics_;

    HandlerError handler_;

    void sendConfig();
    void sendGenerateImage(async::TimerThread&);
    void updateStatistics(async::TimerThread&);
  };
}

#endif // SENDER_INCLUDE_CONNECTION_HPP
