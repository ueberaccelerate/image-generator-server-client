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

  void Connection::start()
  {
    auto shared_pointer = shared_from_this();

    boost::system::error_code error;

    const std::string serdata = config_.data();
    boost::int32_t serdata_size = serdata.size();

    //size_t len = boost::asio::write(socket_, serdata_size, error);
    boost::array<boost::int32_t, 1> serdata_size_buffer{ serdata_size };
    size_t len = boost::asio::write(socket_, boost::asio::buffer(serdata_size_buffer), error);

    if (error) {
      throw std::runtime_error("error send serdata_size_buffer");
    }
    std::cout << "send config size: " << len << '\n';
    len = boost::asio::write(socket_, boost::asio::buffer(serdata), error);
    if (error) {
      throw std::runtime_error("error send serdata");
    }
    std::cout << "send config data: " << len << '\n';

    tick_count_ = 0;
    async::TimerThread generator{ config_.getFramerate(),[&](async::TimerThread& t) {

        auto raw_data = generate_image(Size{ config_.getWidth(), config_.getHeight() });
        len = boost::asio::write(socket_, boost::asio::buffer(raw_data), error);
        if (error) {
          t.stop();
          throw std::runtime_error("error generator send data");
        }
        tick_count_++;
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

  void Connection::handle_write(const boost::system::error_code& error,
    size_t /*bytes_transferred*/)
  {

  }
}
