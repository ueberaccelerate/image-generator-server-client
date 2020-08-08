#include "gui/recivermainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ReciverMainWindow w;
    w.show();
    return a.exec();
}

/*
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

int main_thread(int argc, char* argv[]) {
  setlocale(LC_ALL, "Russian");
  using boost::asio::ip::tcp;
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::socket socket(io_context);
    socket.connect(tcp::endpoint(boost::asio::ip::address(boost::asio::ip::address_v4::from_string(argv[1])),1234));
    boost::system::error_code error;
    std::vector<unsigned char> data;

    boost::array<size_t, 2> resolutin_buffer;
    size_t len = socket.read_some(boost::asio::buffer(resolutin_buffer), error);
    if (error) {
      throw boost::system::system_error(error); // Some other error.
    }
    const size_t buffer_size = resolutin_buffer[0] * resolutin_buffer[1];
    size_t recived = 0;
    while (true)
    {
      //boost::array<char, 1024> buf;
      std::vector<char> buf(socket.available());
      len = socket.read_some(boost::asio::buffer(buf), error);
      std::copy(buf.begin(), buf.begin() + len, std::back_inserter(data));
      if (data.size() == buffer_size) {
        // flush
        recived++;
        std::cout << recived << "\n";
        data = std::vector<unsigned char>();
      }
      if (error) throw boost::system::system_error(error); // Some other error.
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

int main(int argc, char* argv[]) {
  return main_thread(argc, argv);
}
*/
