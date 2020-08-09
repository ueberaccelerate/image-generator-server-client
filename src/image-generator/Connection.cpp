#include "Connection.h"
#include <async/TimerThread.h>

#include <iostream>
#include <vector>
#include <random>

#include <boost/array.hpp>

namespace server {
  using boost::asio::ip::tcp;

  std::vector<unsigned char> generate_image(const Size& size) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(33, 196);
    std::vector<unsigned char> data(size.height*size.width, static_cast<unsigned char>(distrib(gen)));

    return data;
  }
  std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
  }

  Connection::pointer Connection::create(boost::asio::io_context& io_context, const resource::Config& config)
  {
    return pointer(new Connection(io_context, config));
  }

  tcp::socket& Connection::socket()
  {
    return socket_;
  }
  void Connection::sendConfig() {
    boost::system::error_code error;

    auto shared_pointer = shared_from_this();

    const std::string serdata = config_.data();
    const boost::int32_t serdata_size = serdata.size();

    boost::array<boost::int32_t, 1> serdata_size_buffer{ serdata_size };
    size_t len = boost::asio::write(socket_, boost::asio::buffer(serdata_size_buffer), error);
    if (error) {
      throw std::runtime_error("error send serdata_size_buffer");
    }
    std::cout << "send config size bytes: " << len << " should send: " << sizeof(boost::int32_t) << " bytes\n";

    len = boost::asio::write(socket_, boost::asio::buffer(serdata), error);
    if (error) {
      throw std::runtime_error("error send serdata");
    }

    std::cout << "send config data bytes: " << len << " should send: " << serdata.size() << " bytes\n";
  }

  void Connection::start()
  {
    sendConfig();
    tick_count_ = 0;
    async::TimerThread generator{ config_.getFramerate(),[&](async::TimerThread& t) {
        boost::system::error_code error;
        auto start = async::TimerThread::FastTimeNamespace::now();
        auto raw_data = generate_image(Size{ config_.getWidth(), config_.getHeight() });
        auto len = boost::asio::write(socket_, boost::asio::buffer(raw_data), error);
        if (error) {
          t.stop();
        }
        tick_count_++;
        auto duratio_ms = std::chrono::duration_cast<std::chrono::milliseconds>(async::TimerThread::FastTimeNamespace::now() - start).count(); 
        const auto fps = config_.getFramerate();
        const auto fps_ms = (1000.0 / config_.getFramerate());

        const auto real_fps = (1000.0) / duratio_ms;
        const auto real_fps_ms = duratio_ms;


        std::cout << "config fps: " << fps      << " one frame: " << fps_ms     << " ms\n";
        std::cout << "real   fps: " << real_fps << " one frame: " << duratio_ms << " ms\n";
    } };
  }
  Connection::~Connection() {
    std::cout << "close connection: tick.number = " << tick_count_ << '\n';
  }
  Connection::Connection(boost::asio::io_context& io_context, const resource::Config& config)
    : config_(config)
    , tick_count_(0)
    , socket_(io_context)
  {
  }

  void Connection::handle_write(const boost::system::error_code& /*error*/,
    size_t /*bytes_transferred*/)
  {

  }
}
