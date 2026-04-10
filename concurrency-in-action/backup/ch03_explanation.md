# Chapter 3: Sharing Data Between Threads - Explained Simply

## Introduction

When multiple threads access the same data simultaneously, problems can occur. This chapter teaches you how to safely share data between threads without corrupting your program.

---

## 1. The Problem with Sharing Data

### 1.1 Race Conditions

**Simple Explanation:**
Imagine two people trying to edit the same Google Doc at the exact same time. They might overwrite each other's changes. In programming, when threads access shared data without coordination, you get unpredictable results.

**Example:**

```cpp
#include <iostream>
#include <thread>

int counter = 0; // Shared data

void increment() {
    for (int i = 0; i < 100000; ++i) {
        counter++; // NOT thread-safe!
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
  
    t1.join();
    t2.join();
  
    std::cout << "Counter: " << counter << std::endl;
    // Expected: 200000, but you'll get less due to race condition!
    return 0;
}
```

**Why it fails:**

- Thread 1 reads counter (value: 10)
- Thread 2 reads counter (value: 10) - before Thread 1 writes back
- Thread 1 increments and writes back (value: 11)
- Thread 2 increments and writes back (value: 11)
- Result: We lost one increment!

---

## 2. Protecting Shared Data with Mutex

### 2.1 What is a Mutex?

**Simple Explanation:**
A mutex (mutual exclusion) is like a bathroom key. Only one person can use the bathroom at a time. When you're done, you return the key for the next person.

**Example: Basic Mutex Usage**

```cpp
#include <iostream>
#include <thread>
#include <mutex>

int counter = 0;
std::mutex mtx; // The "key" to protect counter

void safe_increment() {
    for (int i = 0; i < 100000; ++i) {
        mtx.lock();      // Get the key (wait if someone else has it)
        counter++;       // Safe to modify
        mtx.unlock();    // Return the key
    }
}

int main() {
    std::thread t1(safe_increment);
    std::thread t2(safe_increment);
  
    t1.join();
    t2.join();
  
    std::cout << "Counter: " << counter << std::endl;
    // Now you'll always get 200000!
    return 0;
}
```

### 2.2 RAII with lock_guard

**Simple Explanation:**
Manually calling `lock()` and `unlock()` is error-prone. What if an exception happens? The mutex stays locked forever! `lock_guard` automatically unlocks when it goes out of scope (like a smart pointer for mutexes).

**Example:**

```cpp
#include <mutex>

std::mutex mtx;
int counter = 0;

void safe_increment_better() {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mtx); // Locks here
        counter++;
        // Automatically unlocks when 'lock' goes out of scope
    }
}

void risky_operation() {
    std::lock_guard<std::mutex> lock(mtx);
    counter++;
  
    if (counter > 100) {
        throw std::runtime_error("Error!");
        // No problem! lock_guard unlocks automatically
    }
}
```

### 2.3 Flexible Locking with unique_lock

**Simple Explanation:**
`unique_lock` is like `lock_guard` but more flexible. You can unlock it early, lock it again, or transfer ownership to another `unique_lock`.

**Example:**

```cpp
#include <mutex>
#include <thread>

std::mutex mtx;

void flexible_locking() {
    std::unique_lock<std::mutex> lock(mtx); // Locks immediately
  
    // Do some work with the lock held
  
    lock.unlock(); // Unlock early if needed
  
    // Do some work without holding the lock
  
    lock.lock(); // Lock again when needed
  
    // More protected work
  
    // Automatically unlocks at end of scope
}

void deferred_locking() {
    // Don't lock immediately
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
  
    // Do some setup work without holding the lock
  
    lock.lock(); // Lock when ready
  
    // Protected work
}
```

---

## 3. Deadlock: The Silent Killer

### 3.1 What is Deadlock?

**Simple Explanation:**
Imagine two people at a narrow bridge, each refusing to back up. They're stuck forever! In programming, deadlock happens when two threads are waiting for each other to release locks.

**Example: Deadlock Scenario**

```cpp
#include <mutex>
#include <thread>

std::mutex mutex1, mutex2;

void thread1_function() {
    std::lock_guard<std::mutex> lock1(mutex1); // Thread 1 gets mutex1
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(mutex2); // Waits for mutex2 (held by thread 2)
    // DEADLOCK!
}

void thread2_function() {
    std::lock_guard<std::mutex> lock2(mutex2); // Thread 2 gets mutex2
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock1(mutex1); // Waits for mutex1 (held by thread 1)
    // DEADLOCK!
}

int main() {
    std::thread t1(thread1_function);
    std::thread t2(thread2_function);
  
    t1.join(); // Will never finish!
    t2.join(); // Will never finish!
    return 0;
}
```

### 3.2 Preventing Deadlock with std::lock

**Simple Explanation:**
`std::lock()` is like a smart traffic controller that locks multiple mutexes simultaneously without causing deadlock.

**Example: Deadlock Prevention**

```cpp
#include <mutex>
#include <thread>

std::mutex mutex1, mutex2;

void safe_transfer() {
    // Lock both mutexes atomically (all or nothing)
    std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mutex2, std::defer_lock);
  
    std::lock(lock1, lock2); // Deadlock-free locking!
  
    // Both locks are now held
    // Do your work
  
    // Both automatically unlock at end of scope
}
```

**C++17 Improvement: scoped_lock**

```cpp
#include <mutex>

std::mutex mutex1, mutex2;

void even_safer_transfer() {
    // C++17 way - one line, no deadlock possible
    std::scoped_lock lock(mutex1, mutex2);
  
    // Both locks held
    // Automatically unlock at end of scope
}
```

---

## 4. Protecting Data Structures

### 4.1 Problem: Interface Race Conditions

**Simple Explanation:**
Even if individual operations are thread-safe, combining them can still cause problems.

**Example: The Stack Problem**

```cpp
#include <stack>
#include <mutex>

template<typename T>
class ThreadUnsafeStack {
    std::stack<T> data;
    mutable std::mutex mtx;
  
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push(value);
    }
  
    // PROBLEM: This interface is not thread-safe!
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.empty();
    }
  
    T top() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.top();
    }
  
    void pop() {
        std::lock_guard<std::mutex> lock(mtx);
        data.pop();
    }
};

// Usage (UNSAFE):
ThreadUnsafeStack<int> stack;

// Thread 1:
if (!stack.empty()) {          // Check: not empty
    // Thread 2 could pop here!
    int value = stack.top();   // CRASH! Stack is now empty!
    stack.pop();
}
```

### 4.2 Solution: Better Interface Design

**Example: Thread-Safe Stack**

```cpp
#include <stack>
#include <mutex>
#include <memory>
#include <exception>

template<typename T>
class ThreadSafeStack {
    std::stack<T> data;
    mutable std::mutex mtx;
  
public:
    ThreadSafeStack() {}
  
    // Copy constructor with locking
    ThreadSafeStack(const ThreadSafeStack& other) {
        std::lock_guard<std::mutex> lock(other.mtx);
        data = other.data;
    }
  
    ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;
  
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push(std::move(value));
    }
  
    // Combine top() and pop() into one atomic operation
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(mtx);
      
        if (data.empty()) {
            throw std::runtime_error("Stack is empty");
        }
      
        // Allocate return value before modifying stack
        std::shared_ptr<T> result(std::make_shared<T>(data.top()));
        data.pop();
        return result;
    }
  
    // Alternative pop with reference
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
      
        if (data.empty()) {
            throw std::runtime_error("Stack is empty");
        }
      
        value = data.top();
        data.pop();
    }
  
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.empty();
    }
};

// Safe usage:
ThreadSafeStack<int> stack;

// Thread-safe pop - no race condition
std::shared_ptr<int> value = stack.pop(); // Atomic operation
```

---

## 5. Alternative Synchronization Mechanisms

### 5.1 Protecting Shared Data During Initialization

**Problem: One-time Initialization**

**Simple Explanation:**
Sometimes you need to initialize something once, but multiple threads might try to initialize it simultaneously.

**Example: std::call_once**

```cpp
#include <mutex>
#include <memory>

class DatabaseConnection {
public:
    void connect() { /* expensive operation */ }
};

std::shared_ptr<DatabaseConnection> connection_ptr;
std::once_flag connection_init_flag;

void get_connection() {
    // Initialize only once, even if called by multiple threads
    std::call_once(connection_init_flag, []() {
        connection_ptr.reset(new DatabaseConnection);
        connection_ptr->connect();
    });
  
    // Use connection_ptr
}
```

### 5.2 Reader-Writer Lock (shared_mutex)

**Simple Explanation:**
Sometimes multiple threads can read data simultaneously (safe), but only one can write (must be exclusive). Like a library: many can read books, but only one person can organize the shelf at a time.

**Example: std::shared_mutex (C++17)**

```cpp
#include <shared_mutex>
#include <map>
#include <string>

class PhoneBook {
    std::map<std::string, std::string> data;
    mutable std::shared_mutex mtx;
  
public:
    // Multiple readers can access simultaneously
    std::string lookup(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(mtx); // Shared (read) lock
        auto it = data.find(name);
        return (it != data.end()) ? it->second : "";
    }
  
    // Only one writer at a time, blocks all readers
    void add_entry(const std::string& name, const std::string& number) {
        std::unique_lock<std::shared_mutex> lock(mtx); // Exclusive (write) lock
        data[name] = number;
    }
};
```

---

## 6. Recursive Locking

**Simple Explanation:**
Sometimes a function needs to lock a mutex, then call another function that also needs the same mutex. `std::recursive_mutex` allows the same thread to lock it multiple times.

**Example:**

```cpp
#include <mutex>

class BankAccount {
    double balance;
    std::recursive_mutex mtx;
  
public:
    void deposit(double amount) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        balance += amount;
    }
  
    void transfer_from_savings() {
        std::lock_guard<std::recursive_mutex> lock(mtx); // First lock
      
        // This calls deposit, which locks the same mutex again
        deposit(100.0); // Second lock - OK with recursive_mutex!
    }
};
```

**Warning:** Recursive mutexes can hide design problems. Usually better to refactor your code.

---

## 7. Key Guidelines for Safe Data Sharing

### Rule 1: Don't Pass Pointers/References to Protected Data Outside the Scope

```cpp
// BAD:
class DataHolder {
    int data;
    std::mutex mtx;
  
public:
    int* get_data() {
        return &data; // DANGER! Bypasses mutex protection
    }
};

// GOOD:
class SafeDataHolder {
    int data;
    std::mutex mtx;
  
public:
    int get_data_copy() {
        std::lock_guard<std::mutex> lock(mtx);
        return data; // Return a copy
    }
};
```

### Rule 2: Lock Mutexes in the Same Order

```cpp
// Always lock mutex1 before mutex2 across all functions
void function_a() {
    std::scoped_lock lock(mutex1, mutex2); // Good!
}

void function_b() {
    std::scoped_lock lock(mutex1, mutex2); // Same order - Good!
}
```

### Rule 3: Minimize Time Holding Locks

```cpp
void slow_operation() {
    // Do expensive work WITHOUT the lock
    std::string result = expensive_computation();
  
    {
        std::lock_guard<std::mutex> lock(mtx);
        // Only lock when actually modifying shared data
        shared_data = result;
    } // Lock released immediately
  
    // Do more work without the lock
}
```

---

## Summary

| Concept                   | Simple Analogy    | Use Case                              |
| ------------------------- | ----------------- | ------------------------------------- |
| **Mutex**           | Bathroom key      | Protect single resource               |
| **lock_guard**      | Auto-locking door | Simple, exception-safe locking        |
| **unique_lock**     | Manual door lock  | Need flexibility (early unlock, etc.) |
| **scoped_lock**     | Multi-key holder  | Lock multiple mutexes safely          |
| **shared_mutex**    | Library access    | Many readers, one writer              |
| **call_once**       | One-time setup    | Initialize exactly once               |
| **recursive_mutex** | Re-entrant lock   | Function calls itself                 |

---

## Common Mistakes to Avoid

1. **Forgetting to lock:** Always protect shared data
2. **Locking too much:** Hold locks for minimal time
3. **Wrong lock order:** Always lock in same order
4. **Returning protected data:** Don't give out pointers/references
5. **Interface races:** Make operations atomic at interface level
6. **Exception safety:** Use RAII (lock_guard, unique_lock)

---

## Real-World Analogy Summary

Think of concurrent programming like managing a restaurant kitchen:

- **Shared data** = cooking station
- **Mutex** = "station in use" sign
- **lock_guard** = automatic door that locks/unlocks
- **Deadlock** = two cooks waiting for each other's stations
- **Race condition** = two cooks grabbing the same ingredient
- **Thread-safe design** = proper kitchen workflow rules

The key is: **coordinate access, minimize wait times, and design safe workflows!**
