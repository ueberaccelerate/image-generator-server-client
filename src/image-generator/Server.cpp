#include "Server.h"

#include <iostream>


#include <ctime>

#include <boost/bind/bind.hpp>

namespace server {

  ImageGeneratorServer::ImageGeneratorServer(boost::asio::io_context& io_context, const resource::Config& config)
    : 
     io_context_(io_context)
    , config_(config)
    , acceptor_(io_context, tcp::endpoint(tcp::v4(), config.getPort()))
  {
    start_accept();
  }

  void ImageGeneratorServer::start_accept()
  {
    Connection::pointer new_connection = Connection::create(io_context_, config_, [&](const boost::system::error_code& ec) {
      if (ec) {
        current_connection_ = nullptr;
      }
    });
    acceptor_.async_accept(new_connection->socket(),
      boost::bind(&ImageGeneratorServer::handle_accept, this, new_connection,
        boost::asio::placeholders::error));
  }

  void ImageGeneratorServer::handle_accept(Connection::pointer new_connection,
    const boost::system::error_code& error)
  {
    if (!error)
    {
      try {
        new_connection->start();
        current_connection_ = new_connection;
      }
      catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
      }
    }

    start_accept();
  }
}
