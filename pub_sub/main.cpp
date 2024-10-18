#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>
#include <random>
#include <thread>
std::queue<int> data_queue;
std::mutex queue_mutex;
std::condition_variable cv;
bool finished{false};

void producer(int items) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 100);

  for (int i = 0; i < items; ++i) {
    int value = dis(gen);
    {
      std::lock_guard<std::mutex> lock(queue_mutex);
      data_queue.push(value);
      std::cout << "Produced " << value << std::endl;
    }
    cv.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    finished = true;
  }
  cv.notify_one();
}
void consumer() {
  while (true) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv.wait(lock, [] { return !data_queue.empty() || finished; });
    if (finished && data_queue.empty()) {
      std::cout << "Consumer finished" << std::endl;
      return;
    }
    int value = data_queue.front();
    data_queue.pop();
    std::cout << "Consumed: " << value << std::endl;
    lock.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
int main(int argc, char *argv[]) {
  std::thread producer_thread(producer, 10);
  std::thread consumer_thread(consumer);

  producer_thread.join();
  consumer_thread.join();
  return 0;
}
