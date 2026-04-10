# Singleton Design Pattern

## What is the Singleton Pattern?

The Singleton Pattern is a creational design pattern that **ensures a class has only ONE instance** and provides a **global point of access** to that instance. No matter how many times you try to create the object, you always get the same instance.

Think of it as having **one and only one manager** in a company - everyone who needs to talk to the manager talks to the same person, not multiple managers.

## Real-World Analogy

Imagine a **country's president**:
- There can only be **one president** at a time
- Everyone in the country refers to the **same president**
- You can't create multiple presidents - there's only one official office
- Everyone accesses the president through the **official channel** (getInstance)

Other analogies:
- **Sun in the solar system** - there's only one sun that everything revolves around
- **CEO of a company** - one CEO that all departments report to
- **Printer spooler** - one manager handling all print jobs

## Problem it Solves

**Without Singleton:**
```cpp
// Anyone can create multiple instances
DatabaseConnection db1;  // Creates new connection
DatabaseConnection db2;  // Creates another connection
DatabaseConnection db3;  // Yet another connection!

// Problems:
// - Wasting resources (multiple connections)
// - Inconsistent state (different configs in each)
// - No central control
```

**With Singleton:**
```cpp
// Only one instance exists
DatabaseConnection& db1 = DatabaseConnection::getInstance();
DatabaseConnection& db2 = DatabaseConnection::getInstance();
// db1 and db2 point to THE SAME object!
```

## Structure

### High-Level Design Diagram

```
┌─────────────────────────────────────┐
│          <<Singleton>>              │
│         DatabaseConnection          │
├─────────────────────────────────────┤
│ - static instance: DatabaseConnection*│
│ - connectionString: string          │
├─────────────────────────────────────┤
│ - DatabaseConnection()              │  ← Private constructor
│ - DatabaseConnection(const &)       │  ← Deleted copy constructor
│ - operator=(const &)                │  ← Deleted assignment
├─────────────────────────────────────┤
│ + static getInstance(): DatabaseConnection& │
│ + connect(): void                   │
│ + query(sql: string): void          │
└─────────────────────────────────────┘
           ▲
           │
           │ Uses (always same instance)
           │
    ┌──────┴───────┐
    │    Client    │
    └──────────────┘
```

### Key Components

1. **Private Constructor**: Prevents direct instantiation
2. **Static Instance**: Holds the single instance
3. **Static getInstance()**: Provides global access point
4. **Deleted Copy Operations**: Prevents copying the instance

## Simple C++ Example: Configuration Manager

```cpp
#include <iostream>
#include <string>
#include <map>

class ConfigurationManager {
private:
    // Private constructor - can't create directly
    ConfigurationManager() {
        std::cout << "ConfigurationManager created!" << std::endl;
        // Load default settings
        settings_["theme"] = "dark";
        settings_["language"] = "en";
    }
    
    // Delete copy constructor
    ConfigurationManager(const ConfigurationManager&) = delete;
    
    // Delete assignment operator
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;
    
    std::map<std::string, std::string> settings_;
    
public:
    // Static method to get the single instance
    static ConfigurationManager& getInstance() {
        static ConfigurationManager instance;  // C++11 magic static
        return instance;
    }
    
    void setSetting(const std::string& key, const std::string& value) {
        settings_[key] = value;
        std::cout << "Setting '" << key << "' = '" << value << "'" << std::endl;
    }
    
    std::string getSetting(const std::string& key) const {
        auto it = settings_.find(key);
        return (it != settings_.end()) ? it->second : "";
    }
    
    void displaySettings() const {
        std::cout << "\n=== Current Settings ===" << std::endl;
        for (const auto& [key, value] : settings_) {
            std::cout << key << ": " << value << std::endl;
        }
    }
};

int main() {
    std::cout << "=== Singleton Pattern Demo ===" << std::endl;
    
    // Get the singleton instance (first time - creates it)
    ConfigurationManager& config1 = ConfigurationManager::getInstance();
    config1.displaySettings();
    
    // Change a setting
    config1.setSetting("theme", "light");
    config1.setSetting("font-size", "14");
    
    // Get instance again from different part of code
    ConfigurationManager& config2 = ConfigurationManager::getInstance();
    config2.displaySettings();  // Same settings!
    
    // Verify they're the same instance
    std::cout << "\nAre config1 and config2 the same? "
              << ((&config1 == &config2) ? "YES" : "NO") << std::endl;
    
    // This won't compile - copy constructor is deleted
    // ConfigurationManager config3 = config1;  // ERROR!
    
    return 0;
}
```

### Output:
```
=== Singleton Pattern Demo ===
ConfigurationManager created!

=== Current Settings ===
language: en
theme: dark
Setting 'theme' = 'light'
Setting 'font-size' = '14'

=== Current Settings ===
font-size: 14
language: en
theme: light

Are config1 and config2 the same? YES
```

## Thread-Safe Singleton Pattern

In multi-threaded applications, we need to ensure that only one instance is created even when multiple threads try to access it simultaneously.

### Problem with Naive Implementation

```cpp
// ❌ NOT THREAD-SAFE (Don't use this!)
class BadSingleton {
private:
    static BadSingleton* instance_;
    BadSingleton() {}
    
public:
    static BadSingleton* getInstance() {
        if (instance_ == nullptr) {      // Thread A checks
            // Thread B might also check here!
            instance_ = new BadSingleton();  // Both create instance!
        }
        return instance_;
    }
};
BadSingleton* BadSingleton::instance_ = nullptr;
```

**Problem**: If two threads call `getInstance()` simultaneously, both might see `instance_` as null and create two instances!

### Solution 1: Meyers' Singleton (C++11 Magic Statics) ⭐ **RECOMMENDED**

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

class Logger {
private:
    int instanceId_;
    static int instanceCount_;
    
    // Private constructor
    Logger() {
        instanceId_ = ++instanceCount_;
        std::cout << "Logger instance " << instanceId_ 
                  << " created by thread " 
                  << std::this_thread::get_id() << std::endl;
        
        // Simulate some initialization work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Delete copy and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
public:
    // Thread-safe singleton using C++11 magic statics
    static Logger& getInstance() {
        static Logger instance;  // ⭐ Guaranteed thread-safe in C++11!
        return instance;
    }
    
    void log(const std::string& message) {
        std::cout << "[Instance " << instanceId_ << "] " 
                  << message << std::endl;
    }
    
    int getInstanceId() const { return instanceId_; }
};

int Logger::instanceCount_ = 0;

// Test thread safety
void workerThread(int threadNum) {
    // Multiple threads try to get instance simultaneously
    Logger& logger = Logger::getInstance();
    logger.log("Hello from thread " + std::to_string(threadNum));
}

int main() {
    std::cout << "=== Thread-Safe Singleton Demo ===" << std::endl;
    
    // Launch multiple threads simultaneously
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(workerThread, i);
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify single instance
    Logger& logger = Logger::getInstance();
    std::cout << "\nFinal instance ID: " << logger.getInstanceId() << std::endl;
    std::cout << "✓ Only ONE instance was created!" << std::endl;
    
    return 0;
}
```

**Why is this thread-safe?**
- C++11 guarantees that static local variables are initialized in a **thread-safe manner**
- If multiple threads enter `getInstance()` simultaneously, only one will initialize the static variable
- Other threads will wait until initialization is complete
- This is called **"Magic Statics"** - the compiler handles synchronization for you!

### Solution 2: Double-Checked Locking Pattern (DCLP)

```cpp
#include <mutex>
#include <memory>
#include <iostream>

class DatabaseConnection {
private:
    static std::unique_ptr<DatabaseConnection> instance_;
    static std::mutex mutex_;
    std::string connectionString_;
    
    DatabaseConnection(const std::string& connStr) 
        : connectionString_(connStr) {
        std::cout << "Database connection created: " << connStr << std::endl;
    }
    
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    
public:
    static DatabaseConnection& getInstance(const std::string& connStr = "") {
        // First check (no locking) - fast path
        if (instance_ == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);  // Lock only if needed
            
            // Second check (with locking) - ensure still null
            if (instance_ == nullptr) {
                instance_.reset(new DatabaseConnection(connStr));
            }
        }
        return *instance_;
    }
    
    void query(const std::string& sql) {
        std::cout << "Executing: " << sql << " on " << connectionString_ << std::endl;
    }
};

// Static member initialization
std::unique_ptr<DatabaseConnection> DatabaseConnection::instance_ = nullptr;
std::mutex DatabaseConnection::mutex_;

int main() {
    std::cout << "=== Double-Checked Locking Demo ===" << std::endl;
    
    DatabaseConnection& db1 = DatabaseConnection::getInstance("localhost:5432");
    db1.query("SELECT * FROM users");
    
    DatabaseConnection& db2 = DatabaseConnection::getInstance();  // Uses existing
    db2.query("INSERT INTO logs VALUES (...)");
    
    return 0;
}
```

**When to use DCLP:**
- When you need to pass parameters to constructor
- When initialization is expensive and you want to defer it
- **BUT**: Prefer Meyers' Singleton for simplicity!

### Solution 3: Call_once Pattern

```cpp
#include <mutex>
#include <memory>
#include <iostream>

class ResourceManager {
private:
    static std::unique_ptr<ResourceManager> instance_;
    static std::once_flag initFlag_;
    
    ResourceManager() {
        std::cout << "ResourceManager initialized" << std::endl;
    }
    
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
public:
    static ResourceManager& getInstance() {
        std::call_once(initFlag_, []() {
            instance_.reset(new ResourceManager());
        });
        return *instance_;
    }
    
    void allocate(const std::string& resource) {
        std::cout << "Allocated: " << resource << std::endl;
    }
};

std::unique_ptr<ResourceManager> ResourceManager::instance_ = nullptr;
std::once_flag ResourceManager::initFlag_;

int main() {
    std::cout << "=== Call_once Pattern Demo ===" << std::endl;
    
    ResourceManager& rm1 = ResourceManager::getInstance();
    rm1.allocate("Memory Block 1");
    
    ResourceManager& rm2 = ResourceManager::getInstance();
    rm2.allocate("Memory Block 2");
    
    return 0;
}
```

## Comparison of Thread-Safe Implementations

| Method | Pros | Cons | Recommendation |
|--------|------|------|----------------|
| **Meyers' Singleton** | Simple, automatic thread-safety, no manual locking | Can't pass constructor params | ⭐ **Use this by default** |
| **Double-Checked Locking** | Lazy initialization, can pass params | Complex, easy to get wrong | Use if you need constructor params |
| **Call_once** | Explicit, clear intent | More boilerplate | Good alternative to DCLP |
| **Eager Initialization** | Simple, no threading issues | Created at startup (might not be needed) | Use for simple cases |

## Real-World Use Cases

### 1. **Logging System**
```cpp
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void log(LogLevel level, const std::string& message) {
        // Write to file, console, or remote server
    }
};

// Used everywhere in the application
Logger::getInstance().log(INFO, "Application started");
```

**Why Singleton?** 
- Single log file - can't have multiple loggers writing simultaneously
- Consistent logging format across entire application
- Centralized log configuration

### 2. **Database Connection Pool**
```cpp
class ConnectionPool {
public:
    static ConnectionPool& getInstance() {
        static ConnectionPool instance(10);  // 10 connections
        return instance;
    }
    
    Connection* getConnection() {
        // Return available connection from pool
    }
    
    void releaseConnection(Connection* conn) {
        // Return connection to pool
    }
};
```

**Why Singleton?**
- Limited database connections - need to manage them centrally
- Expensive to create connections - reuse them
- Single point of configuration

### 3. **Configuration Manager**
```cpp
class AppConfig {
public:
    static AppConfig& getInstance() {
        static AppConfig instance;
        return instance;
    }
    
    std::string getDatabaseUrl() const;
    int getMaxConnections() const;
    bool isDebugMode() const;
};

// Access anywhere
if (AppConfig::getInstance().isDebugMode()) {
    // debug code
}
```

**Why Singleton?**
- Single source of truth for configuration
- Consistent settings across all modules
- Easy to change settings globally

### 4. **Thread Pool Manager**
```cpp
class ThreadPool {
public:
    static ThreadPool& getInstance() {
        static ThreadPool instance(std::thread::hardware_concurrency());
        return instance;
    }
    
    void enqueue(std::function<void()> task) {
        // Add task to queue
    }
};
```

**Why Singleton?**
- Limited CPU cores - should match number of threads
- Centralized task scheduling
- Prevents thread explosion

### 5. **Device Drivers**
```cpp
class PrinterDriver {
public:
    static PrinterDriver& getInstance() {
        static PrinterDriver instance;
        return instance;
    }
    
    void print(const Document& doc) {
        // Send to printer
    }
};
```

**Why Singleton?**
- Only one physical printer
- Serialize print jobs
- Hardware resources are unique

### 6. **Cache Manager**
```cpp
class CacheManager {
public:
    static CacheManager& getInstance() {
        static CacheManager instance;
        return instance;
    }
    
    void put(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
};
```

**Why Singleton?**
- Single cache for entire application
- Memory management from one place
- Consistent cache policies

### 7. **Game State Manager**
```cpp
class GameStateManager {
public:
    static GameStateManager& getInstance() {
        static GameStateManager instance;
        return instance;
    }
    
    void setState(GameState state);
    GameState getState() const;
};
```

**Why Singleton?**
- One game state at a time
- Global access from all game systems
- Centralized state transitions

## When to Use Singleton Pattern

✅ **Use it when:**
- You need **exactly one instance** of a class
- That instance needs to be **accessible globally**
- You want **controlled access** to a shared resource
- Examples: Logger, Config, Database Pool, Hardware Driver

❌ **Don't use it when:**
- You might need **multiple instances** later (rigid design)
- It makes **testing difficult** (global state is hard to mock)
- It creates **hidden dependencies** (unclear who uses what)
- Simple **dependency injection** would work better

## Advantages

1. **Controlled instance creation** - Guarantee single instance
2. **Global access point** - Access from anywhere
3. **Lazy initialization** - Created only when needed (with Meyers')
4. **Thread-safe** - When implemented correctly
5. **Resource management** - Efficient use of shared resources

## Disadvantages

1. **Global state** - Can make code harder to understand and test
2. **Hidden dependencies** - Not clear from function signature
3. **Testing difficulties** - Hard to mock or replace
4. **Tight coupling** - Code depends on concrete class
5. **Violates Single Responsibility** - Class controls both its logic AND its instantiation

## Common Pitfalls

### ❌ Pitfall 1: Not Deleting Copy Operations
```cpp
// BAD - Can be copied!
class BadSingleton {
public:
    static BadSingleton& getInstance() {
        static BadSingleton instance;
        return instance;
    }
    // Forgot to delete copy constructor and assignment!
};

BadSingleton& s1 = BadSingleton::getInstance();
BadSingleton s2 = s1;  // Creates a copy! Now we have 2 instances!
```

### ❌ Pitfall 2: Non-Thread-Safe Implementation
```cpp
// BAD - Race condition!
class BadSingleton {
    static BadSingleton* instance;
public:
    static BadSingleton* getInstance() {
        if (instance == nullptr) {  // Multiple threads can pass this!
            instance = new BadSingleton();
        }
        return instance;
    }
};
```

### ❌ Pitfall 3: Overusing Singleton
```cpp
// BAD - Too many Singletons
UserManager::getInstance()
ProductManager::getInstance()
OrderManager::getInstance()
// ... everything is a Singleton!

// BETTER - Use dependency injection
class OrderService {
    UserManager& userMgr_;
    ProductManager& productMgr_;
public:
    OrderService(UserManager& um, ProductManager& pm) 
        : userMgr_(um), productMgr_(pm) {}
};
```

## Better Alternatives

Sometimes **dependency injection** is better than Singleton:

```cpp
// Instead of Singleton
class OrderProcessor {
    void process() {
        Logger::getInstance().log("Processing...");
        Database::getInstance().query("...");
    }
};

// Better - Dependency Injection
class OrderProcessor {
    Logger& logger_;
    Database& database_;
    
public:
    OrderProcessor(Logger& logger, Database& db) 
        : logger_(logger), database_(db) {}
    
    void process() {
        logger_.log("Processing...");
        database_.query("...");
    }
};

// Now easy to test with mock objects!
```

## Summary

The Singleton Pattern ensures **one and only one instance** of a class exists. It's like having:
- 🏛️ One president per country
- ☀️ One sun in the solar system
- 🎮 One game state manager
- 📝 One application logger

**Key Points:**
- ✅ Use Meyers' Singleton (magic statics) for thread-safety
- ✅ Delete copy constructor and assignment operator
- ✅ Make constructor private
- ⚠️ Don't overuse - consider dependency injection
- ⚠️ Be aware of testing challenges

**When in doubt**: Ask yourself, "Do I **really** need only one instance, or am I just being lazy with global state?"

---

## Quick Reference

```cpp
// Thread-Safe Singleton Template
class MySingleton {
private:
    MySingleton() = default;
    ~MySingleton() = default;
    MySingleton(const MySingleton&) = delete;
    MySingleton& operator=(const MySingleton&) = delete;
    
public:
    static MySingleton& getInstance() {
        static MySingleton instance;
        return instance;
    }
    
    // Your methods here
};
```

## Further Reading

- Design Patterns: Elements of Reusable Object-Oriented Software (GoF)
- Effective C++ by Scott Meyers
- C++ Concurrency in Action by Anthony Williams
- Refactoring.Guru - Singleton Pattern

