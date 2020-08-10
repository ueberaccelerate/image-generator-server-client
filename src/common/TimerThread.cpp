#include <async/TimerThread.h>

#include <iostream>

#include <ctime>

namespace async {

  TimerThread::TimerThread(const Frequency& freq, Callback&& callback) noexcept :
    isRunning_{ true },
    frequency_{ freq },
    callback_{ callback },
    timer_future_{ create_timer_thread() }
  {

  }

  TimerThread::TimerThread(const Interval& interval, Callback&& callback) noexcept 
    : isRunning_ {true}
    , frequency_ {0}
    , callback_interval_{ interval }
    , callback_{ callback },
      timer_future_{ create_timer_thread() }
  {
  }

  TimerThread::~TimerThread() noexcept {
    stop();
    timer_future_.wait();
    preaty_print("destroy time", take());
  }

  void TimerThread::setFrequency(const Frequency& new_freq) {
    frequency_ = new_freq;
    updateInterval();
  }
  void TimerThread::stop() noexcept {
    isRunning_ = false;
  }

  std::future<void> TimerThread::create_timer_thread() noexcept {
    creation_time = take();
    preaty_print("creation time", creation_time);
    std::future<void> task;
    try {
      task = std::async(std::launch::async, [&]() {
        start_task_time = take();
        preaty_print("start task", start_task_time);
        if (frequency_ != 0) {
          updateInterval();
        }
        while (isRunning_)
        {
          callback_(*this);
          std::this_thread::sleep_for(callback_interval_);
        }
      });
    }
    catch (...) {
      std::cerr << "error: create timer thread failture\n";
      throw;
    }
    return std::move(task);
  }
  TimerThread::Interval TimerThread::make_interval() noexcept {
    if (frequency_ <= 0 || frequency_ > 120) {
      std::cerr << "error: frequency > 0 && frequency < 120 (set default value 1)\n";
      frequency_ = 1;
    }
    return Interval{ (1 / frequency_) * 1000 };
  }
  void TimerThread::updateInterval() noexcept {
    // milliseconds
    callback_interval_ = make_interval();
  }

  TimerThread::TimePoint TimerThread::take() {
    return TimeNamespace::now();
  }

  std::string TimerThread::timestamp() const {
    std::time_t t = TimeNamespace::to_time_t(TimeNamespace::now());
    tm ltm;
    //    localtime_s(&ltm, &t);
    std::string timestr = ctime(&t);
    return timestr;
  }
  void TimerThread::preaty_print(const std::string& message, const TimePoint& point) {
    std::time_t t = TimeNamespace::to_time_t(point);
    tm ltm;
    //    localtime_s(&ltm, &t);
    std::string_view timestr = ctime(&t);
    printf("%s %s", message.data(), timestr.data() );
  }

}