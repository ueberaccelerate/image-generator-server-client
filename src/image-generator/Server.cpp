#include "Server.h"

#include <iostream>


#include <ctime>

#include <boost/bind/bind.hpp>

namespace server {

  ImageGeneratorServer::ImageGeneratorServer(boost::asio::io_context& io_context)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), 1234))
  {
    start_accept();
  }

  void ImageGeneratorServer::start_accept()
  {
    auto connection = Connection::create(io_context_);
    Connection::pointer new_connection = connection;
    acceptor_.async_accept(new_connection->socket(),
      boost::bind(&ImageGeneratorServer::handle_accept, this, new_connection,
        boost::asio::placeholders::error));
  }

  void ImageGeneratorServer::handle_accept(Connection::pointer new_connection,
    const boost::system::error_code& error)
  {
    std::cout << "new connection\n";
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }
}