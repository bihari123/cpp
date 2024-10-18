# Thread-safe Configuration Class

## Overview

This project provides a thread-safe Configuration class implemented in C++. It's designed to store and manage configuration settings in a multi-threaded, asynchronous environment. The class follows the Singleton pattern, ensuring a single point of access for configuration data across your application.

## Features

- Thread-safe operations for reading and writing configuration data
- Singleton pattern implementation for global access
- Support for asynchronous environments
- Efficient read operations with shared mutex
- Easy to integrate into existing C++ projects

## Requirements

- C++17 or later
- A C++ compiler with support for `<shared_mutex>` (available in C++17)

## Usage

### Basic Usage

```cpp
#include "Configuration.h"

int main() {
    auto& config = Configuration::getInstance();
    
    // Set a configuration value
    config.setValue("server_url", "http://example.com");
    
    // Get a configuration value
    std::string url = config.getValue("server_url");
    
    // Check if a key exists
    if (config.hasKey("port")) {
        // Use the port
    }
    
    return 0;
}
```

### Asynchronous Usage

```cpp
#include "Configuration.h"
#include <future>

void asyncConfigUpdate(const std::string& key, const std::string& value) {
    Configuration::getInstance().setValue(key, value);
}

void asyncConfigRead(const std::string& key) {
    std::string value = Configuration::getInstance().getValue(key);
    // Use the value
}

int main() {
    auto future1 = std::async(std::launch::async, asyncConfigUpdate, "server", "192.168.1.1");
    auto future2 = std::async(std::launch::async, asyncConfigRead, "server");
    
    // Wait for async operations to complete
    future1.wait();
    future2.wait();
    
    return 0;
}
```

## API Reference

- `static Configuration& getInstance()`: Get the singleton instance of the Configuration class.
- `void setValue(const std::string& key, const std::string& value)`: Set a configuration value.
- `std::string getValue(const std::string& key, const std::string& defaultValue = "")`: Get a configuration value. Returns the default value if the key doesn't exist.
- `bool hasKey(const std::string& key)`: Check if a configuration key exists.
- `void removeValue(const std::string& key)`: Remove a configuration key-value pair.
- `void clear()`: Clear all configuration data.

## Thread Safety

All operations on the Configuration class are thread-safe. Read operations (getValue, hasKey) can occur concurrently, while write operations (setValue, removeValue, clear) are exclusive.

## Considerations

1. Performance: While thread-safe, the use of mutexes introduces some overhead. For extremely performance-critical applications, consider lock-free alternatives.
2. Error Handling: The current implementation doesn't include extensive error handling. Consider adding exception handling or error codes for production use.
3. Persistence: This implementation stores configuration in memory. For configurations that need to persist between application runs, consider adding save/load functionality.

## Contributing

Contributions to improve the Configuration class are welcome. Please submit pull requests or open issues to discuss proposed changes.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
