
#include <chrono>
#include <future>
#include <functional>
#include <string>
#include <atomic>

namespace server {
  class TimerThread {
  public:
    using Frequency = int;
    using Interval = std::chrono::milliseconds;
    using Callback = std::function<void(TimerThread& t)>;
    using TimeNamespace = std::chrono::system_clock;
    using TimePoint = TimeNamespace::time_point;
    explicit TimerThread(const Frequency& freq, Callback&& callback) noexcept;

    TimerThread(const TimerThread&) = delete;
    TimerThread& operator=(const TimerThread&) = delete;
    ~TimerThread() noexcept;

    void setFrequency(const Frequency& new_freq);
    void stop() noexcept;
  private:
    std::future<void> create_timer_thread() noexcept;
    Interval make_interval() noexcept;
    void updateInterval() noexcept;
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
    TimePoint take();
    void preaty_print(const std::string& message, const TimePoint& point);
  };
}