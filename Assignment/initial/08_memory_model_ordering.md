# Assignment 08: Memory Model & Ordering

## Overview
Understand C++ memory model basics, memory ordering semantics (relaxed, acquire, release, sequential consistency), and synchronization patterns.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 6 hours  
**Prerequisites:** Assignments 01-07

---

## Learning Objectives
- Understand C++ memory model fundamentals
- Master memory_order: relaxed, acquire, release, seq_cst
- Implement acquire-release synchronization
- Understand happens-before relationships
- Recognize when stronger ordering is needed

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. The C++ memory model defines:
A) CPU cache sizes  
B) How memory operations are visible across threads  
C) RAM speed  
D) Variable sizes  

**Answer:** B

### Q2. memory_order_relaxed provides:
A) No ordering guarantees, only atomicity  
B) Strongest ordering  
C) Mutex-like behavior  
D) Sequential consistency  

**Answer:** A

### Q3. memory_order_acquire is used for:
A) Storing values  
B) Loading values and acquiring dependencies  
C) Both load and store  
D) Non-atomic operations  

**Answer:** B

### Q4. memory_order_release is used for:
A) Loading values  
B) Storing values and releasing dependencies  
C) Deleting memory  
D) Locking mutexes  

**Answer:** B

### Q5. memory_order_seq_cst provides:
A) No ordering  
B) Sequential consistency (strongest ordering)  
C) Same as relaxed  
D) Only works on x86  

**Answer:** B - Default for atomic operations

### Q6. Acquire-release semantics ensure:
A) All operations before release are visible after acquire  
B) Random ordering  
C) No synchronization  
D) Fastest performance  

**Answer:** A

### Q7. On x86/x64, acquire-release is:
A) Very expensive  
B) Often "free" (strong hardware memory model)  
C) Impossible  
D) Requires OS calls  

**Answer:** B - x86 has strong memory ordering

### Q8. memory_order_relaxed is safe for:
A) All synchronization  
B) Simple counters where order doesn't matter  
C) Never safe  
D) Only on ARM  

**Answer:** B

### Q9. What does "happens-before" mean?
A) Time ordering  
B) One operation is guaranteed visible before another  
C) CPU instruction order  
D) Thread creation order  

**Answer:** B

### Q10. memory_order_consume (deprecated) was for:
A) Data dependencies  
B) Eating memory  
C) Same as acquire  
D) Releasing locks  

**Answer:** A - Deprecated due to complexity

---

## Part 2: Code Review Exercises

### Exercise 2.1: Incorrect Relaxed Usage

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<bool> ready{false};
int data = 0;

void producer() {
    data = 42;
    ready.store(true, std::memory_order_relaxed);  // Wrong!
}

void consumer() {
    while (!ready.load(std::memory_order_relaxed)) {}
    std::cout << data << "\n";  // Might not see 42!
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}
```

**Questions:**
1. What's the bug?
2. Why might consumer not see data=42?
3. Fix using acquire-release

**Solution:**
```cpp
void producer() {
    data = 42;
    ready.store(true, std::memory_order_release);  // Release
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {}  // Acquire
    std::cout << data << "\n";  // Guaranteed to see 42
}
```

---

### Exercise 2.2: Proper Release-Acquire

```cpp
#include <atomic>
#include <vector>
#include <thread>

std::vector<int> sharedData;
std::atomic<bool> dataReady{false};

void producer() {
    sharedData.push_back(1);
    sharedData.push_back(2);
    sharedData.push_back(3);
    
    dataReady.store(true, std::memory_order_release);
}

void consumer() {
    while (!dataReady.load(std::memory_order_acquire)) {}
    
    // All writes to sharedData are visible here
    for (int val : sharedData) {
        std::cout << val << " ";
    }
}
```

**Questions:**
1. Why is acquire-release needed?
2. What does release synchronize with?
3. Could you use relaxed? Why not?

---

### Exercise 2.3: Seq_cst Overhead

```cpp
std::atomic<int> x{0}, y{0};
int r1, r2;

void thread1() {
    x.store(1, std::memory_order_seq_cst);
    r1 = y.load(std::memory_order_seq_cst);
}

void thread2() {
    y.store(1, std::memory_order_seq_cst);
    r2 = x.load(std::memory_order_seq_cst);
}

// With seq_cst: impossible to have r1==0 && r2==0
// With relaxed: possible!
```

**Questions:**
1. Why does seq_cst prevent r1==0 && r2==0?
2. Can you use acquire-release instead?
3. When is seq_cst necessary?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Acquire-Release Flag

```cpp
#include <atomic>
#include <vector>
#include <thread>
#include <iostream>

class AcquireReleaseExample {
public:
    void producer() {
        // Produce data
        for (int i = 0; i < 100; ++i) {
            data_.push_back(i);
        }
        
        // Release: all above writes visible to consumer
        ready_.store(true, std::memory_order_release);
    }
    
    void consumer() {
        // Acquire: see all writes before release
        while (!ready_.load(std::memory_order_acquire)) {
            // Spin
        }
        
        // Process data
        std::cout << "Data size: " << data_.size() << "\n";
    }
    
private:
    std::vector<int> data_;
    std::atomic<bool> ready_{false};
};

void test() {
    AcquireReleaseExample example;
    
    std::thread t1([&] { example.producer(); });
    std::thread t2([&] { example.consumer(); });
    
    t1.join();
    t2.join();
}
```

---

### Exercise 3.2: Relaxed Counter

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>

class RelaxedCounter {
public:
    void increment() {
        // Relaxed: only atomicity, no ordering
        counter_.fetch_add(1, std::memory_order_relaxed);
    }
    
    int get() const {
        // Relaxed: we only care about final value
        return counter_.load(std::memory_order_relaxed);
    }
    
private:
    std::atomic<int> counter_{0};
};

void testRelaxedCounter() {
    RelaxedCounter counter;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 1000; ++j) {
                counter.increment();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Count: " << counter.get() << "\n";  // 10000
}
```

**Question:** Why is relaxed safe here?  
**Answer:** We only care about the final count, not the order of increments.

---

### Exercise 3.3: Double-Checked Locking (Correct)

```cpp
#include <atomic>
#include <mutex>

class Singleton {
public:
    static Singleton* getInstance() {
        // First check (relaxed, no lock)
        Singleton* tmp = instance_.load(std::memory_order_acquire);
        
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            // Second check (under lock)
            tmp = instance_.load(std::memory_order_relaxed);
            
            if (tmp == nullptr) {
                tmp = new Singleton();
                instance_.store(tmp, std::memory_order_release);
            }
        }
        
        return tmp;
    }
    
private:
    static std::atomic<Singleton*> instance_;
    static std::mutex mutex_;
    
    Singleton() = default;
};

std::atomic<Singleton*> Singleton::instance_{nullptr};
std::mutex Singleton::mutex_;

// Note: C++11 static initialization is simpler and correct!
```

**Questions:**
1. Why acquire-release here?
2. What if you used relaxed?
3. Is this better than C++11 static initialization?

---

### Exercise 3.4: Message Queue with Ordering

```cpp
#include <atomic>
#include <vector>
#include <thread>

template<typename T>
class LockFreeQueue {
public:
    void push(T value) {
        size_t writePos = writeIndex_.load(std::memory_order_relaxed);
        buffer_[writePos] = value;
        
        // Release: data write visible before index update
        writeIndex_.store(writePos + 1, std::memory_order_release);
    }
    
    bool pop(T& value) {
        size_t readPos = readIndex_.load(std::memory_order_relaxed);
        size_t writePos = writeIndex_.load(std::memory_order_acquire);
        
        if (readPos >= writePos) {
            return false;  // Empty
        }
        
        value = buffer_[readPos];
        readIndex_.store(readPos + 1, std::memory_order_release);
        return true;
    }
    
private:
    std::vector<T> buffer_{1000};
    std::atomic<size_t> writeIndex_{0};
    std::atomic<size_t> readIndex_{0};
};

// Note: Simplified, real queue needs wraparound handling
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Find the Ordering Bug

```cpp
std::atomic<int> x{0}, y{0};

void thread1() {
    x.store(1, std::memory_order_relaxed);
    y.store(1, std::memory_order_relaxed);
}

void thread2() {
    while (y.load(std::memory_order_relaxed) == 0) {}
    assert(x.load(std::memory_order_relaxed) == 1);  // Can fail!
}
```

**Tasks:**
1. Why can assertion fail?
2. Fix using acquire-release
3. Explain the happens-before relationship

---

### Exercise 4.2: Seq_cst When Needed

```cpp
std::atomic<bool> flag1{false}, flag2{false};
int data1 = 0, data2 = 0;

void thread1() {
    data1 = 42;
    flag1.store(true, std::memory_order_release);
    
    while (!flag2.load(std::memory_order_acquire)) {}
    std::cout << data2 << "\n";
}

void thread2() {
    data2 = 100;
    flag2.store(true, std::memory_order_release);
    
    while (!flag1.load(std::memory_order_acquire)) {}
    std::cout << data1 << "\n";
}
```

**Question:** Is acquire-release sufficient or do you need seq_cst?  
**Answer:** Acquire-release is sufficient here for synchronization between pairs.

---

## Part 5: Performance Analysis

### Exercise 5.1: Memory Order Benchmark

```cpp
#include <atomic>
#include <chrono>
#include <thread>

void benchmarkRelaxed() {
    std::atomic<int> counter{0};
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Relaxed: " << duration.count() << "ms\n";
}

void benchmarkSeqCst() {
    std::atomic<int> counter{0};
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000000; ++i) {
        counter.fetch_add(1, std::memory_order_seq_cst);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Seq_cst: " << duration.count() << "ms\n";
}

int main() {
    benchmarkRelaxed();
    benchmarkSeqCst();
    return 0;
}
```

**Analysis:**
1. Is there measurable difference?
2. Does it vary by platform (x86 vs ARM)?

---

### Exercise 5.2: Acquire-Release vs Seq_cst

Compare performance in multi-threaded scenario.

---

## Submission Guidelines

Submit:
1. **answers.md** - MCQ answers
2. **code_review/** - Fixed code
3. **implementations/** - All implementations
4. **debugging/** - Bug fixes
5. **performance/** - Benchmarks and analysis

---

## Evaluation Criteria

- **Understanding (40%):** Memory model concepts
- **Correctness (30%):** Proper ordering usage
- **Analysis (30%):** Performance insights

---

## Key Takeaways

✅ Acquire-release for most synchronization  
✅ Relaxed for simple counters  
✅ Seq_cst when total order needed (rare)  
✅ x86 has strong hardware ordering  
✅ ARM requires explicit barriers  

---

## Common Pitfalls

❌ Using relaxed for synchronization  
❌ Overusing seq_cst (performance cost)  
❌ Not understanding happens-before  

---

## Next Steps

Proceed to **Assignment 09: Futures and Promises**.

---

## Resources

- [cppreference - memory_order](https://en.cppreference.com/w/cpp/atomic/memory_order)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 5
- [Preshing on Programming - Memory Ordering](https://preshing.com/20120913/acquire-and-release-semantics/)

Good luck!