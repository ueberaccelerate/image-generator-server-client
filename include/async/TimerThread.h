
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <string>

namespace async {
class TimerThread {
public:
  using Frequency = int;
  using Interval = std::chrono::milliseconds;
  using Callback = std::function<void(TimerThread &t)>;
  using TimeNamespace = std::chrono::system_clock;
  using FastTimeNamespace = std::chrono::steady_clock;
  using TimePoint = TimeNamespace::time_point;

  /// Start an asynchronous loop with specific frequency.
  /**
   * @param freq is a period of call @callback
   *
   * @param callback The handler to be called when the timer fire:
   * @code void handler(
   *   TimerThread& t // retrunt this object
   * ); @endcode
   *
   * @par Example
   * @code
   * void handler(TimerThread& t)
   * {
   *   t.stop();  // call handler once time and stop
   * }
   * @endcode
   */
  explicit TimerThread(const Frequency &freq, Callback &&callback) noexcept;

  /// Start an asynchronous loop with specific interval .
/**
 * @param interval is a period of call @callback
 *
 * @param callback The handler to be called when the timer fire:
 * @code void handler(
 *   TimerThread& t // retrunt this object
 * ); @endcode
 *
 * @par Example
 * @code
 * void handler(TimerThread& t)
 * {
 *   t.stop();  // call handler once time and stop
 * }
 * @endcode
 */
  explicit TimerThread(const Interval& freq, Callback&& callback) noexcept;

  TimerThread(const TimerThread &) = delete;
  TimerThread &operator=(const TimerThread &) = delete;
  ~TimerThread() noexcept;

  void setFrequency(const Frequency &new_freq);
  void stop() noexcept;

  std::string timestamp() const;

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
  void preaty_print(const std::string &message, const TimePoint &point);
};
} // namespace async
