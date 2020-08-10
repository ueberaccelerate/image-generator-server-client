#include "Connection.h"

#include <iostream>
#include <vector>
#include <random>

#include <boost/array.hpp>
#include <boost/bind.hpp>

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

  Connection::pointer Connection::create(boost::asio::io_context& io_context, const resource::Config& config, HandlerError&& handler)
  {
    return pointer(new Connection(io_context, config, std::forward<HandlerError>(handler)));
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

  void Connection::sendGenerateImage(async::TimerThread& t)
  {
    boost::system::error_code error;
    auto start = async::TimerThread::FastTimeNamespace::now();
    auto raw_data = generate_image(Size{ config_.getWidth(), config_.getHeight() });
    auto len = boost::asio::write(socket_, boost::asio::buffer(raw_data), error);

    handler_(error);

    tick_count_++;
    auto duratio_ms = std::chrono::duration_cast<std::chrono::milliseconds>(async::TimerThread::FastTimeNamespace::now() - start).count(); 
    tick_ms_ = duratio_ms;
  }

  void Connection::updateStatistics(async::TimerThread& e)
  {
    const auto real_fps_ms = tick_ms_.load();

    const auto fps = config_.getFramerate();
    const auto fps_ms = (1000.0 / config_.getFramerate());
    const auto real_fps = (real_fps_ms == 0) ? fps : (1000.0) / real_fps_ms;

    std::cout << "Update statistic " << e.timestamp()
              << "config fps: " << fps      << " one frame: " << fps_ms     << " ms\n"
              << "real   fps: " << real_fps << " one frame: " << real_fps_ms << " ms\n";
  }

  void Connection::start()
  {
    tick_count_ = 0;
    tick_ms_ = 0;

    sendConfig();

    generator_ = std::make_unique<async::TimerThread>(config_.getFramerate(), boost::bind(&Connection::sendGenerateImage, this, _1));
    statistics_ = std::make_unique<async::TimerThread>(async::TimerThread::Interval(5000), boost::bind(&Connection::updateStatistics, this, _1));
  }
  Connection::~Connection() {
    std::cout << "close connection: tick.total_number = " << tick_count_ << '\n';
  }
  Connection::Connection(boost::asio::io_context& io_context, const resource::Config& config, HandlerError&& handler)
    : config_(config)
    , tick_count_(0)
    , socket_(io_context)
    , handler_(handler)
  {
  }
}
