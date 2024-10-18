#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class CronJob {
public:
  CronJob(std::string name, std::function<void()> task, int interval_seconds)
      : name(std::move(name)), task(std::move(task)),
        interval_seconds(interval_seconds), is_running(false),
        last_run(std::chrono::steady_clock::now()) {}

  // Delete copy constructor and assignment operator
  CronJob(const CronJob &) = delete;
  CronJob &operator=(const CronJob &) = delete;

  // Implement move constructor and assignment operator
  CronJob(CronJob &&other) noexcept
      : name(std::move(other.name)), task(std::move(other.task)),
        interval_seconds(other.interval_seconds),
        is_running(other.is_running.load()), last_run(other.last_run) {}

  CronJob &operator=(CronJob &&other) noexcept {
    if (this != &other) {
      name = std::move(other.name);
      task = std::move(other.task);
      interval_seconds = other.interval_seconds;
      is_running = other.is_running.load();
      last_run = other.last_run;
    }
    return *this;
  }

  std::string name;
  std::function<void()> task;
  int interval_seconds;
  std::atomic<bool> is_running;
  std::chrono::steady_clock::time_point last_run;
};

class CronJobManager {
public:
  void addJob(std::string name, std::function<void()> task,
              int interval_seconds) {
    jobs.emplace_back(std::make_unique<CronJob>(
        std::move(name), std::move(task), interval_seconds));
  }

  void start() {
    running = true;
    worker_thread = std::thread(&CronJobManager::run, this);
  }

  void stop() {
    running = false;
    cv.notify_all();
    if (worker_thread.joinable()) {
      worker_thread.join();
    }
  }

  void printStatus() {
    std::lock_guard<std::mutex> lock(mutex);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");

    std::cout << "Current status at " << ss.str() << ":\n";
    for (const auto &job : jobs) {
      std::cout << job->name << ": "
                << (job->is_running ? "Running" : "Not running") << "\n";
    }
    std::cout << std::endl;
  }

private:
  void run() {
    while (running) {
      auto now = std::chrono::steady_clock::now();

      for (auto &job : jobs) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           now - job->last_run)
                           .count();
        if (!job->is_running && elapsed >= job->interval_seconds) {
          std::thread([this, &job, now]() {
            job->is_running = true;
            job->task();
            job->is_running = false;
            job->last_run = now;
            cv.notify_all();
          }).detach();
        }
      }

      std::unique_lock<std::mutex> lock(mutex);
      cv.wait_until(lock, now + std::chrono::seconds(1),
                    [this]() { return !running; });
    }
  }

  std::vector<std::unique_ptr<CronJob>> jobs;
  std::atomic<bool> running{false};
  std::thread worker_thread;
  std::mutex mutex;
  std::condition_variable cv;
};

// Example usage
int main() {
  CronJobManager manager;

  manager.addJob(
      "Job 1",
      []() {
        std::cout << "Executing Job 1\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
      },
      15);

  manager.addJob(
      "Job 2",
      []() {
        std::cout << "Executing Job 2\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
      },
      3);

  manager.start();

  // Run for 60 seconds
  for (int i = 0; i < 60; ++i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    manager.printStatus();
  }

  manager.stop();

  return 0;
}
