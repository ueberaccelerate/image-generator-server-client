#ifndef THREADPOOL_THREADSAFE_QUEUE_HPP
#define THREADPOOL_THREADSAFE_QUEUE_HPP

// thread libraries
#include <condition_variable>
#include <mutex>

// utils libraries
#include <exception>
#include <memory>
#include <queue>

namespace async {

#define NODISCARD [[nodiscard]]
template <typename T, typename Container = std::queue<T>>
class threadsafe_queue final {
  class threadsafe_empty_error : public std::exception {};

 public:
  using value_type = typename Container::value_type;
  using value_type_ptr = std::shared_ptr<value_type>;
  using size_type = typename Container::size_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;

  threadsafe_queue &operator=(const threadsafe_queue &) = delete;

  void push(const value_type &value) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push(value);
    queue_cv_.notify_one();
  }

  void push(value_type &&value) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push(value);
    queue_cv_.notify_one();
  }

  NODISCARD bool try_pop(T &element) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.empty()) {
      return false;
    }
    element = queue_.front();
    queue_.pop();
    return true;
  }

  NODISCARD value_type_ptr try_pop() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.empty()) {
      return value_type_ptr();
    }
    auto result = std::make_shared<value_type>(queue_.front());
    queue_.pop();
    return result;
  }
  void wait_and_pop(T &element) {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_cv_.wait(lock, [this]() { return !queue_.empty(); });
    element = queue_.front();
    queue_.pop();
  }

  NODISCARD value_type_ptr wait_and_pop() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_cv_.wait(lock, [this]() { return !queue_.empty(); });
    auto result = std::make_shared<value_type>(queue_.front());
    queue_.pop();
    return result;
  }

  NODISCARD bool empty() {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      return queue_.empty();
  }

  void interrupt_wait() { queue_cv_.notify_one(); }
  ~threadsafe_queue() { interrupt_wait(); }

 private:
  std::mutex queue_mutex_;
  std::condition_variable queue_cv_;
  Container queue_;
};

}

#endif  // THREADPOOL_THREADSAFE_QUEUE_HPP
