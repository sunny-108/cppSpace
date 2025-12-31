# Assignment 05: Deadlock Avoidance

## Overview
Master deadlock prevention, detection, and avoidance strategies in concurrent C++ programs.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 5-6 hours  
**Prerequisites:** Assignments 01-04

---

## Learning Objectives
- Understand deadlock conditions and causes
- Implement deadlock prevention strategies
- Use std::lock and std::scoped_lock for atomic locking
- Apply lock ordering to prevent deadlock
- Detect and recover from deadlock scenarios

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. How many necessary conditions must hold for deadlock to occur?
A) 2  
B) 3  
C) 4  
D) 5  

**Answer:** C - Mutual exclusion, hold and wait, no preemption, circular wait

### Q2. Which is NOT a deadlock prevention strategy?
A) Lock ordering  
B) Using std::lock to lock multiple mutexes atomically  
C) Using more threads  
D) Timeout-based locking  

**Answer:** C

### Q3. std::lock (free function) guarantees:
A) Locks are acquired in order  
B) Locks are acquired atomically without deadlock  
C) Fastest locking  
D) Only works with two mutexes  

**Answer:** B

### Q4. std::scoped_lock (C++17) is equivalent to:
A) std::lock_guard  
B) std::lock + std::lock_guard for multiple mutexes  
C) std::unique_lock  
D) Manual locking  

**Answer:** B - It's the RAII version of std::lock for multiple mutexes

### Q5. A classic cause of deadlock is:
A) Too many threads  
B) Two threads acquiring locks in opposite order  
C) Using atomic operations  
D) Fast CPUs  

**Answer:** B

### Q6. Lock ordering means:
A) Always lock mutexes in the same order across all threads  
B) Lock the newest mutex first  
C) Random locking  
D) Lock by thread ID  

**Answer:** A

### Q7. Can std::try_lock prevent deadlock?
A) No, never  
B) Yes, by backing off and retrying if lock fails  
C) Only with timed_mutex  
D) Only in debug mode  

**Answer:** B

### Q8. The circular wait condition requires:
A) Exactly 2 threads  
B) A cycle in the resource allocation graph  
C) Infinite loops  
D) No mutexes  

**Answer:** B

### Q9. Which is more robust for multiple mutex locking?
A) std::scoped_lock  
B) Manual lock() calls  
C) Nested lock_guards  
D) try_lock in a loop  

**Answer:** A - It's exception-safe and deadlock-free

### Q10. To break the "hold and wait" condition:
A) Don't use mutexes  
B) Acquire all locks atomically or none  
C) Use more threads  
D) Lock for longer  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Classic Deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mutex1;
std::mutex mutex2;

void thread1Function() {
    std::lock_guard<std::mutex> lock1(mutex1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Increase deadlock chance
    
    std::lock_guard<std::mutex> lock2(mutex2);  // Waiting for mutex2
    std::cout << "Thread 1 acquired both locks\n";
}

void thread2Function() {
    std::lock_guard<std::mutex> lock2(mutex2);  // Opposite order!
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::lock_guard<std::mutex> lock1(mutex1);  // Waiting for mutex1
    std::cout << "Thread 2 acquired both locks\n";
}

int main() {
    std::thread t1(thread1Function);
    std::thread t2(thread2Function);
    
    t1.join();
    t2.join();
    
    std::cout << "Finished (if you see this, no deadlock occurred)\n";
    return 0;
}
```

**Questions:**
1. Why does this deadlock?
2. Draw the resource allocation graph
3. Fix using consistent lock ordering
4. Fix using std::scoped_lock

**Solution 1: Lock Ordering**
```cpp
void thread1Function() {
    std::lock_guard<std::mutex> lock1(mutex1);
    std::lock_guard<std::mutex> lock2(mutex2);
    std::cout << "Thread 1\n";
}

void thread2Function() {
    std::lock_guard<std::mutex> lock1(mutex1);  // Same order!
    std::lock_guard<std::mutex> lock2(mutex2);
    std::cout << "Thread 2\n";
}
```

**Solution 2: scoped_lock**
```cpp
void thread1Function() {
    std::scoped_lock lock(mutex1, mutex2);
    std::cout << "Thread 1\n";
}

void thread2Function() {
    std::scoped_lock lock(mutex2, mutex1);  // Order doesn't matter!
    std::cout << "Thread 2\n";
}
```

---

### Exercise 2.2: Bank Transfer Deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>

class BankAccount {
public:
    BankAccount(int id, double balance) : id_(id), balance_(balance) {}
    
    int id() const { return id_; }
    double balance() const { return balance_; }
    
    void withdraw(double amount) {
        balance_ -= amount;
    }
    
    void deposit(double amount) {
        balance_ += amount;
    }
    
    std::mutex& getMutex() { return mutex_; }
    
private:
    int id_;
    double balance_;
    std::mutex mutex_;
};

void transfer(BankAccount& from, BankAccount& to, double amount) {
    std::lock_guard<std::mutex> lock1(from.getMutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(to.getMutex());  // Potential deadlock!
    
    from.withdraw(amount);
    to.deposit(amount);
    
    std::cout << "Transferred " << amount 
              << " from " << from.id() << " to " << to.id() << "\n";
}

int main() {
    BankAccount account1(1, 1000.0);
    BankAccount account2(2, 1000.0);
    
    // Simultaneous transfers in opposite directions
    std::thread t1([&] { transfer(account1, account2, 100.0); });
    std::thread t2([&] { transfer(account2, account1, 50.0); });  // Deadlock!
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Tasks:**
1. Explain why deadlock can occur
2. Fix using lock ordering by account ID
3. Fix using std::lock
4. Which solution is better?

**Solution: Lock Ordering by ID**
```cpp
void transfer(BankAccount& from, BankAccount& to, double amount) {
    // Always lock lower ID first
    BankAccount* first = &from;
    BankAccount* second = &to;
    
    if (from.id() > to.id()) {
        std::swap(first, second);
    }
    
    std::lock_guard<std::mutex> lock1(first->getMutex());
    std::lock_guard<std::mutex> lock2(second->getMutex());
    
    from.withdraw(amount);
    to.deposit(amount);
}
```

**Solution: std::scoped_lock (Preferred)**
```cpp
void transfer(BankAccount& from, BankAccount& to, double amount) {
    std::scoped_lock lock(from.getMutex(), to.getMutex());
    
    from.withdraw(amount);
    to.deposit(amount);
}
```

---

### Exercise 2.3: Hierarchical Deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex databaseMutex;
std::mutex networkMutex;
std::mutex logMutex;

void operation1() {
    std::lock_guard<std::mutex> dbLock(databaseMutex);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::lock_guard<std::mutex> netLock(networkMutex);
    std::lock_guard<std::mutex> logLock(logMutex);
    
    std::cout << "Operation 1\n";
}

void operation2() {
    std::lock_guard<std::mutex> netLock(networkMutex);  // Different order!
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::lock_guard<std::mutex> dbLock(databaseMutex);
    
    std::cout << "Operation 2\n";
}

int main() {
    std::thread t1(operation1);
    std::thread t2(operation2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Questions:**
1. Identify the deadlock scenario
2. Establish a lock hierarchy
3. Fix by enforcing consistent ordering

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Deadlock-Free Dining Philosophers

Implement the classic dining philosophers problem without deadlock:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

class DiningPhilosophers {
public:
    DiningPhilosophers(int numPhilosophers)
        : forks_(numPhilosophers) {}
    
    void philosopher(int id) {
        int leftFork = id;
        int rightFork = (id + 1) % forks_.size();
        
        for (int i = 0; i < 5; ++i) {
            think(id);
            eat(id, leftFork, rightFork);
        }
    }
    
private:
    void think(int id) {
        std::cout << "Philosopher " << id << " is thinking\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void eat(int id, int leftFork, int rightFork) {
        // Your implementation: acquire both forks without deadlock!
        // Use std::scoped_lock or lock ordering
        
        std::cout << "Philosopher " << id << " is eating\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::vector<std::mutex> forks_;
};

int main() {
    const int numPhilosophers = 5;
    DiningPhilosophers dining(numPhilosophers);
    
    std::vector<std::thread> philosophers;
    for (int i = 0; i < numPhilosophers; ++i) {
        philosophers.emplace_back([&dining, i] {
            dining.philosopher(i);
        });
    }
    
    for (auto& t : philosophers) {
        t.join();
    }
    
    std::cout << "All philosophers finished!\n";
    return 0;
}
```

**Solutions to try:**
1. Lock ordering (always lock lower-numbered fork first)
2. std::scoped_lock for atomic acquisition
3. Resource hierarchy (asymmetric solution)

---

### Exercise 3.2: Timeout-Based Deadlock Avoidance

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

class Resource {
public:
    Resource(int id) : id_(id) {}
    
    bool tryAcquire(std::chrono::milliseconds timeout) {
        return mutex_.try_lock_for(timeout);
    }
    
    void release() {
        mutex_.unlock();
    }
    
    int id() const { return id_; }
    
private:
    int id_;
    std::timed_mutex mutex_;
};

bool acquireResources(Resource& r1, Resource& r2, int maxRetries = 5) {
    // Your implementation:
    // Try to acquire both resources with timeout
    // If one fails, release any held locks and retry
    // Return true if both acquired, false if max retries exceeded
    
    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        if (r1.tryAcquire(std::chrono::milliseconds(50))) {
            if (r2.tryAcquire(std::chrono::milliseconds(50))) {
                return true;  // Both acquired
            }
            r1.release();  // Failed to get r2, release r1
        }
        
        // Back off before retry
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return false;  // Failed after max retries
}

void testTimeoutApproach() {
    Resource resource1(1);
    Resource resource2(2);
    
    auto task = [&](int threadId) {
        for (int i = 0; i < 3; ++i) {
            if (acquireResources(resource1, resource2)) {
                std::cout << "Thread " << threadId << " working\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                resource1.release();
                resource2.release();
            } else {
                std::cout << "Thread " << threadId << " failed to acquire\n";
            }
        }
    };
    
    std::thread t1(task, 1);
    std::thread t2(task, 2);
    
    t1.join();
    t2.join();
}
```

---

### Exercise 3.3: Deadlock Detection

Implement simple deadlock detection by timeout:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

class DeadlockDetector {
public:
    DeadlockDetector(std::chrono::milliseconds timeout)
        : timeout_(timeout), deadlockDetected_(false) {}
    
    template<typename Func>
    bool execute(Func&& func) {
        std::atomic<bool> completed{false};
        
        std::thread worker([&] {
            func();
            completed = true;
        });
        
        // Wait with timeout
        auto start = std::chrono::steady_clock::now();
        while (!completed) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed > timeout_) {
                deadlockDetected_ = true;
                std::cout << "DEADLOCK DETECTED! Task exceeded timeout.\n";
                
                // In real system: log, alert, possibly abort thread
                worker.detach();  // Can't safely join a deadlocked thread
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        worker.join();
        return true;
    }
    
    bool deadlockDetected() const { return deadlockDetected_; }
    
private:
    std::chrono::milliseconds timeout_;
    std::atomic<bool> deadlockDetected_;
};

// Test
void testDeadlockDetection() {
    std::mutex m1, m2;
    
    DeadlockDetector detector(std::chrono::seconds(2));
    
    auto deadlockTask = [&] {
        std::lock_guard<std::mutex> lock1(m1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock2(m2);  // Will deadlock if m2 held
    };
    
    // This should complete normally
    detector.execute(deadlockTask);
    
    // Create actual deadlock scenario and test detection
    // (Implementation left as exercise)
}
```

---

### Exercise 3.4: Lock-Free Alternative (Advanced)

Demonstrate avoiding locks entirely with atomics:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

// Lock-free counter (no deadlock possible!)
class LockFreeCounter {
public:
    LockFreeCounter() : value_(0) {}
    
    void increment() {
        value_.fetch_add(1, std::memory_order_relaxed);
    }
    
    int get() const {
        return value_.load(std::memory_order_relaxed);
    }
    
private:
    std::atomic<int> value_;
};

// Compare with mutex-based counter
class LockedCounter {
public:
    LockedCounter() : value_(0) {}
    
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++value_;
    }
    
    int get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }
    
private:
    int value_;
    mutable std::mutex mutex_;
};

void benchmark() {
    // Test both approaches
    // Compare performance and deadlock risk
}
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Hidden Deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

class ThreadSafeVector {
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.push_back(value);
    }
    
    int sum() {
        std::lock_guard<std::mutex> lock(mutex_);
        int total = 0;
        for (int value : data_) {
            total += value;
        }
        return total;
    }
    
    void pushAndSum(int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.push_back(value);
        
        // Deadlock! sum() tries to lock already-held mutex
        int total = sum();  
        std::cout << "Total: " << total << "\n";
    }
    
private:
    std::vector<int> data_;
    std::mutex mutex_;
};

int main() {
    ThreadSafeVector vec;
    std::thread t([&] { vec.pushAndSum(42); });
    t.join();
    return 0;
}
```

**Tasks:**
1. Identify the deadlock
2. Fix by creating internal non-locking version
3. Or use recursive_mutex (discuss trade-offs)

---

### Exercise 4.2: ABBA Deadlock

```cpp
// Thread 1: locks A then B
// Thread 2: locks B then A
// Classic ABBA deadlock pattern
// Your task: detect and fix
```

---

## Part 5: Performance Analysis

### Exercise 5.1: Lock Ordering Overhead

Compare performance of ordered vs unordered locking:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

std::mutex m1, m2;

void unorderedLocking(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock1(m1);
        std::lock_guard<std::mutex> lock2(m2);
        // Work...
    }
}

void scopedLocking(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::scoped_lock lock(m1, m2);
        // Work...
    }
}

void benchmark() {
    const int iterations = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    std::thread t1(unorderedLocking, iterations);
    t1.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    std::thread t2(scopedLocking, iterations);
    t2.join();
    end = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Unordered: " << duration1.count() << "ms\n";
    std::cout << "Scoped: " << duration2.count() << "ms\n";
}
```

**Analysis:**
1. Is there measurable overhead?
2. Is it worth the safety?

---

## Submission Guidelines

Submit in a single ZIP file:
1. **answers.md** - MCQ answers with explanations
2. **code_review/** - Fixed deadlock scenarios
3. **implementations/** - All implementations
4. **debugging/** - Fixed bugs with analysis
5. **performance/** - Benchmarks and analysis

---

## Evaluation Criteria

- **Correctness (40%):** Deadlock-free implementations
- **Understanding (25%):** Clear explanation of deadlock causes
- **Strategy (20%):** Appropriate prevention techniques
- **Analysis (15%):** Performance trade-off insights

---

## Key Takeaways

✅ Always acquire multiple locks in consistent order  
✅ Use std::scoped_lock for multiple mutexes  
✅ Four conditions required for deadlock  
✅ Prevention is better than detection  
✅ Timeout-based approaches can help but aren't foolproof  

---

## Common Pitfalls

❌ Inconsistent lock ordering  
❌ Calling locking functions while holding locks  
❌ Not using std::scoped_lock for multiple mutexes  
❌ Ignoring lock hierarchy  

---

## Next Steps

Proceed to **Assignment 06: Condition Variables** for advanced synchronization.

---

## Resources

- [cppreference - std::lock](https://en.cppreference.com/w/cpp/thread/lock)
- [cppreference - std::scoped_lock](https://en.cppreference.com/w/cpp/thread/scoped_lock)
- [Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)

Good luck!