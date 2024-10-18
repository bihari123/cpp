#pragma once
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
class Config {
private:
  std::unordered_map<std::string, std::string> config;
  mutable std::shared_mutex mx; // In C++, mutable allows a member variable to
                                // be modified even in const member functions.
  // private constructor to prevent instantiation
  Config() = default;

public:
  // delete copy constructor and assignment operator
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  // Static method to access the singleton instance
  static Config &getInstance() {
    static Config instance;
    return instance;
  }

  void setValue(const std::string &key, const std::string &value) {
    std::unique_lock<std::shared_mutex> lock(mx);
    config[key] = value;
  }
  std::string getValue(const std::string &key,
                       const std::string &defaultValue = "") const {
    std::shared_lock<std::shared_mutex> lock(mx);
    auto it = config.find(key);
    return (it != config.end()) ? it->second : defaultValue;
  }
  bool hasKey(const std::string &key) const {
    std::shared_lock<std::shared_mutex> lock(mx);

    return config.find(key) != config.end();
  }
  void removeValue(const std::string &key) {
    std::unique_lock<std::shared_mutex> lock(mx);
    config.erase(key);
  }
  void clear() {
    std::unique_lock<std::shared_mutex> lock(mx);
    config.clear();
  }
};
