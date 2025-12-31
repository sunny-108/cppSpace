# Assignment 04: Mutexes and Locks

## Overview
Master synchronization primitives including std::mutex, lock_guard, unique_lock, scoped_lock, and understand lock management strategies.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 5-6 hours  
**Prerequisites:** Assignments 01-03

---

## Learning Objectives
- Understand mutual exclusion and critical sections
- Master different mutex types (mutex, recursive_mutex, timed_mutex)
- Use lock wrappers (lock_guard, unique_lock, scoped_lock)
- Understand lock ownership and transfer
- Identify and fix race conditions

---

## Part 1: Multiple Choice Questions (12 MCQs)

### Q1. What is the primary purpose of a mutex?
A) To speed up concurrent code  
B) To provide mutual exclusion for shared resources  
C) To create threads  
D) To pass data between threads  

**Answer:** B

### Q2. std::lock_guard vs std::unique_lock - which is more flexible?
A) lock_guard, it has more features  
B) unique_lock, it allows manual unlock and relock  
C) They are identical  
D) Neither can be unlocked manually  

**Answer:** B

### Q3. What happens if you try to lock a std::mutex that's already locked by the same thread?
A) It returns immediately  
B) It waits (deadlock)  
C) It throws an exception  
D) Undefined behavior  

**Answer:** B - Standard mutex causes deadlock; use recursive_mutex instead

### Q4. std::scoped_lock (C++17) is designed to:
A) Lock multiple mutexes atomically  
B) Replace std::lock_guard  
C) Provide timed locking  
D) Create recursive locks  

**Answer:** A - Prevents deadlock when locking multiple mutexes

### Q5. Can you transfer ownership of a unique_lock?
A) No, it's not movable  
B) Yes, using std::move()  
C) Only if the mutex is unlocked  
D) Only between threads  

**Answer:** B

### Q6. What is a critical section?
A) Code that must run fast  
B) Code that accesses shared resources and must be protected  
C) Code in the main thread  
D) Code that creates threads  

**Answer:** B

### Q7. std::recursive_mutex allows:
A) Multiple threads to lock it simultaneously  
B) The same thread to lock it multiple times  
C) Faster locking  
D) Automatic deadlock prevention  

**Answer:** B

### Q8. If you forget to unlock a mutex:
A) It's automatically unlocked  
B) Other threads waiting for it will block indefinitely  
C) An exception is thrown  
D) The program terminates  

**Answer:** B - This is why RAII locks are essential

### Q9. std::try_lock on a mutex:
A) Always locks the mutex  
B) Tries to lock without blocking, returns true if successful  
C) Waits for 1 second  
D) Forces the mutex to unlock  

**Answer:** B

### Q10. Which lock wrapper should you use for simple critical sections?
A) unique_lock always  
B) lock_guard (simplest, lowest overhead)  
C) scoped_lock always  
D) Manual lock/unlock  

**Answer:** B - Unless you need unique_lock's flexibility

### Q11. Can you manually unlock a std::lock_guard?
A) Yes, using unlock()  
B) No, it only unlocks in destructor  
C) Yes, but only once  
D) Only if explicitly requested  

**Answer:** B

### Q12. What is granularity in the context of locking?
A) The size of the mutex object  
B) The amount of code/data protected by a lock  
C) The number of threads  
D) The lock priority  

**Answer:** B - Coarse-grained = large critical section; fine-grained = small

---

## Part 2: Code Review Exercises

### Exercise 2.1: Race Condition

```cpp
#include <iostream>
#include <thread>
#include <vector>

int counter = 0;

void increment(int times) {
    for (int i = 0; i < times; ++i) {
        counter++;  // Not thread-safe!
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(increment, 10000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Expected: 100000, Got: " << counter << "\n";
    return 0;
}
```

**Questions:**
1. Run this multiple times. Is the result always 100000?
2. Why not? Explain the race condition.
3. Fix it using std::mutex and std::lock_guard.
4. What is the performance impact?

**Solution:**
```cpp
#include <mutex>

std::mutex counterMutex;
int counter = 0;

void increment(int times) {
    for (int i = 0; i < times; ++i) {
        std::lock_guard<std::mutex> lock(counterMutex);
        counter++;
    }
}
```

---

### Exercise 2.2: Improper Lock Scope

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
std::vector<int> sharedData;

void addData(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Expensive computation (should be outside lock!)
    int result = 0;
    for (int i = 0; i < 1000000; ++i) {
        result += i * value;
    }
    
    sharedData.push_back(result);  // Only this needs protection
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(addData, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Data size: " << sharedData.size() << "\n";
    return 0;
}
```

**Questions:**
1. What is wrong with the locking strategy?
2. Why is performance poor?
3. Fix it by reducing critical section size.
4. What is the principle here?

**Solution:**
```cpp
void addData(int value) {
    // Computation outside lock
    int result = 0;
    for (int i = 0; i < 1000000; ++i) {
        result += i * value;
    }
    
    // Only lock for shared data access
    {
        std::lock_guard<std::mutex> lock(mtx);
        sharedData.push_back(result);
    }
}
```

---

### Exercise 2.3: Wrong Lock Type

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void recursiveFunction(int depth) {
    if (depth == 0) return;
    
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Depth: " << depth << "\n";
    
    recursiveFunction(depth - 1);  // Tries to lock again!
}

int main() {
    std::thread t(recursiveFunction, 3);
    t.join();
    return 0;
}
```

**Questions:**
1. What happens when you run this?
2. Why does it deadlock?
3. Fix it using std::recursive_mutex.
4. When is recursive_mutex appropriate?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Thread-Safe Counter

Implement a thread-safe counter with proper locking:

```cpp
#include <mutex>
#include <iostream>

class ThreadSafeCounter {
public:
    ThreadSafeCounter() : value_(0) {}
    
    void increment() {
        // Your implementation
    }
    
    void decrement() {
        // Your implementation
    }
    
    int get() const {
        // Your implementation (note: const method)
    }
    
    void add(int n) {
        // Your implementation
    }
    
    // Compound operation (atomic increment and return)
    int incrementAndGet() {
        // Your implementation
    }
    
private:
    int value_;
    mutable std::mutex mutex_;  // mutable for const methods
};

// Test
void testCounter() {
    ThreadSafeCounter counter;
    std::vector<std::thread> threads;
    
    // 10 threads, each increments 1000 times
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter] {
            for (int j = 0; j < 1000; ++j) {
                counter.increment();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final count: " << counter.get() << "\n";  // Should be 10000
}
```

**Requirements:**
- Thread-safe operations
- Efficient locking (minimize critical sections)
- Support const methods with mutable mutex

---

### Exercise 3.2: Thread-Safe Bank Account

```cpp
#include <mutex>
#include <iostream>
#include <string>

class BankAccount {
public:
    BankAccount(std::string owner, double initialBalance)
        : owner_(owner), balance_(initialBalance) {}
    
    void deposit(double amount) {
        // Your implementation
        // Validate amount > 0
    }
    
    bool withdraw(double amount) {
        // Your implementation
        // Check sufficient funds
        // Return true if successful
    }
    
    double getBalance() const {
        // Your implementation
    }
    
    // Transfer between accounts (advanced: needs multiple locks)
    static bool transfer(BankAccount& from, BankAccount& to, double amount) {
        // Your implementation
        // Lock both accounts to prevent inconsistency
        // Be careful of deadlock!
    }
    
private:
    std::string owner_;
    double balance_;
    mutable std::mutex mutex_;
};

// Test
void testBankAccount() {
    BankAccount alice("Alice", 1000.0);
    BankAccount bob("Bob", 500.0);
    
    std::thread t1([&] {
        for (int i = 0; i < 100; ++i) {
            alice.deposit(10);
        }
    });
    
    std::thread t2([&] {
        for (int i = 0; i < 100; ++i) {
            alice.withdraw(5);
        }
    });
    
    t1.join();
    t2.join();
    
    std::cout << "Alice's balance: " << alice.getBalance() << "\n";
    // Should be 1000 + 100*10 - 100*5 = 1500
}
```

**Hint for transfer():**
Use `std::scoped_lock` or lock accounts in consistent order (by address).

---

### Exercise 3.3: Reader-Writer Lock (Simplified)

Implement a simple read-write lock using shared_mutex (C++17):

```cpp
#include <shared_mutex>
#include <mutex>
#include <map>
#include <string>

class ThreadSafeCache {
public:
    void insert(const std::string& key, int value) {
        // Exclusive lock (write)
        std::unique_lock<std::shared_mutex> lock(mutex_);
        cache_[key] = value;
    }
    
    bool find(const std::string& key, int& value) const {
        // Shared lock (read)
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }
    
    void erase(const std::string& key) {
        // Your implementation (exclusive lock)
    }
    
    size_t size() const {
        // Your implementation (shared lock)
    }
    
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, int> cache_;
};

// Test with many readers, few writers
void testCache() {
    ThreadSafeCache cache;
    
    // Writer thread
    std::thread writer([&] {
        for (int i = 0; i < 100; ++i) {
            cache.insert("key" + std::to_string(i), i);
        }
    });
    
    // Multiple reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 10; ++i) {
        readers.emplace_back([&] {
            for (int j = 0; j < 100; ++j) {
                int value;
                cache.find("key50", value);
            }
        });
    }
    
    writer.join();
    for (auto& r : readers) {
        r.join();
    }
}
```

---

### Exercise 3.4: Conditional Lock Wrapper

Implement a lock that can be conditionally enabled/disabled:

```cpp
#include <mutex>

template<typename Mutex>
class ConditionalLock {
public:
    ConditionalLock(Mutex& mtx, bool shouldLock)
        : mutex_(mtx), locked_(shouldLock) {
        if (locked_) {
            mutex_.lock();
        }
    }
    
    ~ConditionalLock() {
        if (locked_) {
            mutex_.unlock();
        }
    }
    
    // Non-copyable, non-movable
    ConditionalLock(const ConditionalLock&) = delete;
    ConditionalLock& operator=(const ConditionalLock&) = delete;
    
private:
    Mutex& mutex_;
    bool locked_;
};

// Usage: lock only when needed
void process(bool needsSynchronization) {
    static std::mutex mtx;
    ConditionalLock<std::mutex> lock(mtx, needsSynchronization);
    
    // Critical section (protected only if needsSynchronization is true)
}
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Mysterious Deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void function1() {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Function 1\n";
    
    // Oops! Calling function2 while holding lock
    function2();
}

void function2() {
    std::lock_guard<std::mutex> lock(mtx);  // Deadlock!
    std::cout << "Function 2\n";
}

int main() {
    std::thread t(function1);
    t.join();
    return 0;
}
```

**Tasks:**
1. Identify why this deadlocks
2. Fix using recursive_mutex or redesign
3. Is recursive_mutex the best solution here?

---

### Exercise 4.2: Data Race Despite Locking

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
std::vector<int> data;

void addData(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    data.push_back(value);
}

int* getData(size_t index) {
    std::lock_guard<std::mutex> lock(mtx);
    if (index < data.size()) {
        return &data[index];  // Returning pointer to protected data!
    }
    return nullptr;
}

int main() {
    std::thread t1([] {
        for (int i = 0; i < 1000; ++i) {
            addData(i);
        }
    });
    
    std::thread t2([] {
        int* ptr = getData(0);
        if (ptr) {
            // Lock released! Another thread might modify data
            std::cout << *ptr << "\n";  // Potential race!
        }
    });
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Questions:**
1. What is the bug here?
2. How can race condition occur despite locking?
3. Fix by returning value instead of pointer

---

## Part 5: Performance Analysis

### Exercise 5.1: Lock Contention

Measure performance under high contention:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

std::mutex mtx;
int counter = 0;

void incrementWithContention(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        counter++;  // Very short critical section = high contention
    }
}

void benchmarkContention(int numThreads, int iterations) {
    counter = 0;
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(incrementWithContention, iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Threads: " << numThreads 
              << ", Time: " << duration.count() << "ms"
              << ", Result: " << counter << "\n";
}

int main() {
    for (int threads : {1, 2, 4, 8, 16}) {
        benchmarkContention(threads, 100000);
    }
    
    return 0;
}
```

**Analysis:**
1. Does performance improve with more threads?
2. Why or why not?
3. How would atomics compare?

---

### Exercise 5.2: Lock Granularity Impact

```cpp
// Coarse-grained: Lock entire operation
void coarseGrained() {
    std::vector<int> data;
    std::mutex mtx;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 10000; ++j) {
                std::lock_guard<std::mutex> lock(mtx);
                
                // Computation inside lock
                int result = j * j;
                data.push_back(result);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Coarse-grained: " << duration.count() << "ms\n";
}

// Fine-grained: Lock only data access
void fineGrained() {
    std::vector<int> data;
    std::mutex mtx;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 10000; ++j) {
                // Computation outside lock
                int result = j * j;
                
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    data.push_back(result);
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Fine-grained: " << duration.count() << "ms\n";
}

int main() {
    coarseGrained();
    fineGrained();
    return 0;
}
```

**Questions:**
1. Which is faster?
2. What is the speedup factor?
3. What is the general principle?

---

## Submission Guidelines

Submit in a single ZIP file:
1. **answers.md** - MCQ answers with explanations
2. **code_review/** - Fixed code for Part 2
3. **implementations/** - All Part 3 implementations
4. **debugging/** - Part 4 fixes with analysis
5. **performance/** - Part 5 benchmarks and analysis

---

## Evaluation Criteria

- **Correctness (35%):** Thread-safe implementations
- **Understanding (25%):** MCQs and race condition analysis
- **Design (20%):** Proper lock granularity and selection
- **Performance (20%):** Insightful analysis of contention

---

## Key Takeaways

✅ Use RAII locks (lock_guard, unique_lock) to prevent forgetting unlock  
✅ Minimize critical sections for better performance  
✅ scoped_lock prevents deadlock when locking multiple mutexes  
✅ shared_mutex for read-heavy workloads  
✅ Don't return pointers/references to protected data  

---

## Common Pitfalls

❌ Forgetting to lock shared data  
❌ Locking too much (large critical sections)  
❌ Returning references to protected data  
❌ Using wrong mutex type (e.g., mutex instead of recursive_mutex)  
❌ Manual lock/unlock instead of RAII  

---

## Next Steps

Proceed to **Assignment 05: Deadlock Avoidance** to learn about preventing and detecting deadlocks.

---

## Resources

- [cppreference - std::mutex](https://en.cppreference.com/w/cpp/thread/mutex)
- [cppreference - std::lock_guard](https://en.cppreference.com/w/cpp/thread/lock_guard)
- [cppreference - std::unique_lock](https://en.cppreference.com/w/cpp/thread/unique_lock)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 3

Good luck!