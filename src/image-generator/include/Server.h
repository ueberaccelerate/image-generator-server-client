#ifndef SENDER_INCLUDE_SERVER_HPP
#define SENDER_INCLUDE_SERVER_HPP

#include "Connection.h"

#include <resource/config.hpp>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>


namespace server {
  using boost::asio::ip::tcp;

  class ImageGeneratorServer
  {
  public:
    ImageGeneratorServer(boost::asio::io_context& io_context);

  private:
    void start_accept();

    void handle_accept(Connection::pointer new_connection,
      const boost::system::error_code& error);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
  };
}

#endif
