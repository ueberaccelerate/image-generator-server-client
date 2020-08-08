#include "Connection.h"
#include "TimerThread.h"

#include <iostream>
#include <vector>
#include <random>

#include <boost/array.hpp>

namespace server {
  using boost::asio::ip::tcp;

  std::vector<unsigned char> generate_image(const Size& size) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(1, 255);
    std::vector<unsigned char> data(size.height*size.width, static_cast<unsigned char>(distrib(gen)));

    return data;
  }
  std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
  }

  Connection::pointer Connection::create(boost::asio::io_context& io_context)
  {
    return pointer(new Connection(io_context));
  }

  tcp::socket& Connection::socket()
  {
    return socket_;
  }

  void Connection::start()
  {
    auto shared_pointer = shared_from_this();

    boost::system::error_code error;
    boost::array<size_t, 2> resolutin_buffer{ 512, 512 };
    size_t len = boost::asio::write(socket_, boost::asio::buffer(resolutin_buffer), error);
    tick_count = 0;
    TimerThread generator{ 1,[&](TimerThread& t) {
        auto raw_data = generate_image(Size{ 512, 512 });

        len = boost::asio::write(socket_, boost::asio::buffer(raw_data), error);
        if (error) {
          t.stop();
        }
        tick_count++;
    } };

    //boost::asio::async_write(socket_, boost::asio::buffer(raw_data),
    //  boost::bind(&tcp_connection::handle_write, shared_pointer,
    //    boost::asio::placeholders::error,
    //    boost::asio::placeholders::bytes_transferred));
  }
  Connection::~Connection() {
    std::cout << "close connection: tick.number = " << tick_count << '\n';
  }
  Connection::Connection(boost::asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  void Connection::handle_write(const boost::system::error_code& error,
    size_t /*bytes_transferred*/)
  {

  }
}