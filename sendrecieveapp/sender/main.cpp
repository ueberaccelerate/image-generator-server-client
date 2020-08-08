#include <future>
#include <thread>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string_view>
#include <random>

#include <boost/asio.hpp>

class TimerThread {
public:
  using Frequency = int;
  using Interval  = std::chrono::milliseconds;
  using Callback  = std::function<void(TimerThread& t)>;
  using TimeNamespace = std::chrono::system_clock;
  using TimePoint = TimeNamespace::time_point;
  explicit TimerThread(const Frequency& freq, Callback&& callback) noexcept :
    isRunning_{true},
    frequency_{freq},
    callback_{callback},
    timer_future_{create_timer_thread()}
  {
    
  }
  TimerThread(const TimerThread&) = delete;
  TimerThread &operator=(const TimerThread&) = delete;

  ~TimerThread() noexcept {
    timer_future_.wait();
  }

  void setFrequency(const Frequency& new_freq) {
    frequency_ = new_freq;
    updateInterval();
  }
  void stop() noexcept {
    isRunning_ = false;
  }
private:
  std::future<void> create_timer_thread() noexcept {
    creation_time = take();
    preaty_print("creation time", creation_time );
    std::future<void> task;
    try {
      task = std::async(std::launch::async, [&]() {
        start_task_time = take();
        preaty_print("start task", start_task_time);
        updateInterval();
        while (isRunning_)
        {
          preaty_print("tick", take());
          callback_(*this);
          std::this_thread::sleep_for(callback_interval_);
        }
      });
    }
    catch (...) {
      std::cerr << "error: create timer thread failture\n";
    }
    return std::move(task);
  }
  Interval make_interval() noexcept {
    if (frequency_ <= 0 || frequency_ > 120) {
      std::cerr << "error: frequency > 0 && frequency < 120 (set default value 1)\n";
      frequency_ = 1;
    }
    return Interval{ (1 / frequency_) * 1000 };
  }
  void updateInterval() noexcept{
    // milliseconds
    callback_interval_ = make_interval();
  }

private:

  // timer properties
  std::atomic_bool isRunning_;
  Frequency frequency_;
  Interval callback_interval_;
  Callback callback_;
  std::future<void> timer_future_;

  // telemetry
  TimePoint creation_time;
  TimePoint start_task_time;
  TimePoint destoy_time;
private:
  TimePoint take() {
    return TimeNamespace::now();
  }
  void preaty_print(const std::string& message, const TimePoint& point) {
    std::time_t t = TimeNamespace::to_time_t(point);
    tm ltm;
    localtime_s(&ltm, &t);
    printf("[%02d:%02d:%02d] %s\n", ltm.tm_hour, ltm.tm_min, ltm.tm_sec, message.data());
  }
};

namespace server {
  struct Size {
    size_t width;
    size_t height;
  };
  auto generate_image(const Size& size) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(1, 255);
    std::vector<unsigned char> data(size.height*size.width, static_cast<unsigned char>(distrib(gen)));
    return data;
  }
}

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_context& io_context)
  {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    auto shared_pointer = shared_from_this();

    boost::system::error_code error;
    boost::array<size_t, 2> resolutin_buffer{ 512, 512 };
    size_t len = boost::asio::write(socket_, boost::asio::buffer(resolutin_buffer), error);
    tick_count = 0;
    TimerThread generator{ 1,[&](TimerThread& t) {
        auto raw_data = server::generate_image(server::Size{ 512, 512 });

        len = boost::asio::write(socket_, boost::asio::buffer(raw_data), error);
        if (error) {
          t.stop();
        }
        tick_count++;
    }};

    //boost::asio::async_write(socket_, boost::asio::buffer(raw_data),
    //  boost::bind(&tcp_connection::handle_write, shared_pointer,
    //    boost::asio::placeholders::error,
    //    boost::asio::placeholders::bytes_transferred));
  }
  ~tcp_connection() {
    std::cout << "close connection: tick.number = " << tick_count << '\n';
  }
private:
  tcp_connection(boost::asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  void handle_write(const boost::system::error_code& error,
    size_t /*bytes_transferred*/)
  {

  }

  std::atomic_int tick_count;
  tcp::socket socket_;
  std::string message_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_context& io_context)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), 1234))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    auto connection = tcp_connection::create(io_context_);
    tcp_connection::pointer new_connection = connection;
    acceptor_.async_accept(new_connection->socket(),
      boost::bind(&tcp_server::handle_accept, this, new_connection,
        boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
    const boost::system::error_code& error)
  {
    std::cout << "new connection\n";
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

int main_thread() {
  

  try
  {
    boost::asio::io_context io_context;
    tcp_server server(io_context);
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
int main() {
  return main_thread();
}