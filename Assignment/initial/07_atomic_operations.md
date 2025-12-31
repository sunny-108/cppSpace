# Assignment 07: Atomic Operations

## Overview
Master atomic operations for lock-free programming, understanding atomic types, fetch operations, and introduction to memory ordering.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 5-6 hours  
**Prerequisites:** Assignments 01-06

---

## Learning Objectives
- Understand std::atomic and atomic types
- Use atomic operations (load, store, exchange, compare_exchange)
- Implement lock-free counters and flags
- Introduction to memory ordering (relaxed, acquire, release)
- Compare atomics vs mutex performance

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. What makes an operation atomic?
A) It's very fast  
B) It completes as a single, indivisible operation  
C) It uses mutexes  
D) It runs in one thread  

**Answer:** B

### Q2. std::atomic<int> is:
A) Always lock-free  
B) Potentially lock-free, check is_lock_free()  
C) Never lock-free  
D) Same as int  

**Answer:** B - Implementation-dependent, usually lock-free on modern systems

### Q3. Which operation is NOT provided by std::atomic?
A) load()  
B) store()  
C) push_back()  
D) fetch_add()  

**Answer:** C - Atomics are for primitive types, not containers

### Q4. fetch_add(1) is equivalent to:
A) ++value  
B) value++  
C) Both, atomically  
D) Neither  

**Answer:** C - Returns old value like value++, but atomic

### Q5. compare_exchange_strong(expected, desired) succeeds when:
A) Always  
B) current value == expected  
C) current value == desired  
D) Never  

**Answer:** B - If equal, sets to desired; otherwise updates expected

### Q6. Memory order relaxed means:
A) No ordering guarantees, only atomicity  
B) Strongest ordering  
C) Same as sequential consistency  
D) Uses locks  

**Answer:** A

### Q7. Can you use std::atomic<std::string>?
A) Yes, always  
B) No, string is not trivially copyable  
C) Only in C++20  
D) Yes, but it's lock-free  

**Answer:** B - Requires trivially copyable types (or use atomic<shared_ptr<string>>)

### Q8. Which is faster: atomic increment or mutex-protected increment?
A) Always atomic  
B) Always mutex  
C) Usually atomic, especially on modern CPUs  
D) Always the same  

**Answer:** C

### Q9. std::atomic_flag is:
A) Guaranteed lock-free  
B) Same as std::atomic<bool>  
C) Not useful  
D) Requires mutex  

**Answer:** A - Only type guaranteed lock-free

### Q10. exchange(newValue) atomically:
A) Adds newValue  
B) Replaces value with newValue and returns old value  
C) Compares values  
D) Does nothing  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: Non-Atomic Counter

```cpp
#include <iostream>
#include <thread>
#include <vector>

int counter = 0;  // Not thread-safe!

void increment(int times) {
    for (int i = 0; i < times; ++i) {
        ++counter;  // Race condition!
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
1. Why is the result not always 100000?
2. Fix using std::atomic<int>
3. Compare performance with mutex version
4. Check if it's lock-free using is_lock_free()

**Solution:**
```cpp
#include <atomic>

std::atomic<int> counter{0};  // Thread-safe!

void increment(int times) {
    for (int i = 0; i < times; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
        // Or simply: ++counter;
    }
}
```

---

### Exercise 2.2: Incorrect Flag Usage

```cpp
#include <iostream>
#include <thread>

bool ready = false;  // Not atomic!
int data = 0;

void producer() {
    data = 42;
    ready = true;  // No synchronization!
}

void consumer() {
    while (!ready) {
        // Busy wait
    }
    std::cout << "Data: " << data << "\n";  // Might not see 42!
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
1. What are the issues here?
2. Why might consumer not see data=42?
3. Fix using std::atomic with proper memory ordering
4. Why is memory ordering important?

**Solution:**
```cpp
#include <atomic>

std::atomic<bool> ready{false};
int data = 0;

void producer() {
    data = 42;
    ready.store(true, std::memory_order_release);  // Release semantics
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire semantics
        // Busy wait
    }
    std::cout << "Data: " << data << "\n";  // Guaranteed to see 42
}
```

---

### Exercise 2.3: Compare-Exchange Misuse

```cpp
#include <atomic>

std::atomic<int> value{0};

void buggyIncrement() {
    int expected = value.load();
    int desired = expected + 1;
    
    // Bug: value might change between load and compare_exchange!
    value.compare_exchange_strong(expected, desired);
}
```

**Questions:**
1. What's wrong with this code?
2. Why might increment be lost?
3. Fix with proper CAS loop

**Solution:**
```cpp
void correctIncrement() {
    int expected = value.load();
    int desired;
    
    do {
        desired = expected + 1;
    } while (!value.compare_exchange_weak(expected, desired));
    // Expected updated on failure, loop retries
}
```

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Lock-Free Counter

```cpp
#include <atomic>
#include <iostream>

class AtomicCounter {
public:
    AtomicCounter() : value_(0) {}
    
    void increment() {
        value_.fetch_add(1, std::memory_order_relaxed);
    }
    
    void decrement() {
        // Your implementation
    }
    
    int get() const {
        // Your implementation
    }
    
    int incrementAndGet() {
        // Your implementation (return new value)
    }
    
    int getAndIncrement() {
        // Your implementation (return old value)
    }
    
    bool compareAndSet(int expected, int newValue) {
        // Your implementation using compare_exchange
    }
    
private:
    std::atomic<int> value_;
};

// Test
void testAtomicCounter() {
    AtomicCounter counter;
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
    
    std::cout << "Counter: " << counter.get() << "\n";  // Should be 10000
}
```

---

### Exercise 3.2: Spin Lock

```cpp
#include <atomic>

class SpinLock {
public:
    SpinLock() : flag_(ATOMIC_FLAG_INIT) {}
    
    void lock() {
        // Your implementation:
        // Spin until flag is acquired
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Busy wait (could add yield or pause)
        }
    }
    
    void unlock() {
        // Your implementation
        flag_.clear(std::memory_order_release);
    }
    
private:
    std::atomic_flag flag_;
};

// Test
void testSpinLock() {
    SpinLock lock;
    int counter = 0;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 1000; ++j) {
                lock.lock();
                ++counter;
                lock.unlock();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Counter: " << counter << "\n";  // Should be 10000
}
```

**Enhancements:**
- Add try_lock()
- Add exponential backoff to reduce contention
- Compare performance with std::mutex

---

### Exercise 3.3: Atomic Flag for Initialization

```cpp
#include <atomic>
#include <iostream>

class Singleton {
public:
    static Singleton& getInstance() {
        // Your implementation:
        // Use atomic flag for one-time initialization
        // Thread-safe without mutex
        
        static std::atomic_flag initialized = ATOMIC_FLAG_INIT;
        static Singleton* instance = nullptr;
        
        if (!initialized.test_and_set(std::memory_order_acquire)) {
            // First thread here
            instance = new Singleton();
        }
        
        // Wait for initialization if needed
        while (instance == nullptr) {
            // Spin
        }
        
        return *instance;
    }
    
private:
    Singleton() {
        std::cout << "Singleton created\n";
    }
};

// Better approach: use std::call_once or C++11 static initialization
```

**Question:** Is this better than mutex? What about C++11 static initialization?

---

### Exercise 3.4: Lock-Free Stack (Simple)

```cpp
#include <atomic>
#include <memory>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(T const& data_) : data(data_), next(nullptr) {}
    };
    
    std::atomic<Node*> head_;
    
public:
    LockFreeStack() : head_(nullptr) {}
    
    void push(T const& data) {
        Node* newNode = new Node(data);
        newNode->next = head_.load();
        
        // CAS loop: retry if head changed
        while (!head_.compare_exchange_weak(newNode->next, newNode));
    }
    
    std::shared_ptr<T> pop() {
        // Your implementation:
        // CAS loop to remove head
        // Handle empty case
        // Be careful with memory management!
        
        Node* oldHead = head_.load();
        while (oldHead && !head_.compare_exchange_weak(oldHead, oldHead->next));
        
        if (oldHead) {
            std::shared_ptr<T> result(std::make_shared<T>(oldHead->data));
            delete oldHead;  // Dangerous! (ABA problem, covered in advanced)
            return result;
        }
        
        return std::shared_ptr<T>();
    }
};

// Test
void testLockFreeStack() {
    LockFreeStack<int> stack;
    
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            stack.push(i);
        }
    });
    
    std::thread t2([&] {
        for (int i = 0; i < 500; ++i) {
            stack.pop();
        }
    });
    
    t1.join();
    t2.join();
}
```

**Note:** This is simplified; real lock-free structures require handling ABA problem.

---

## Part 4: Debugging Exercises

### Exercise 4.1: Atomic Bool vs Int

```cpp
#include <atomic>
#include <iostream>

std::atomic<bool> flag1{false};
std::atomic<int> flag2{0};

void testAtomicBool() {
    // Multiple threads
    if (flag1.load()) {
        // Do something
    }
    flag1.store(true);
}

void testAtomicInt() {
    // Same logic with int
    if (flag2.load()) {
        // Do something
    }
    flag2.store(1);
}
```

**Questions:**
1. Is there a practical difference?
2. Which is preferred for flags?
3. What about atomic_flag?

---

### Exercise 4.2: Missing Memory Order

```cpp
std::atomic<int> value{0};

void thread1() {
    value.store(42);  // Default: seq_cst
}

void thread2() {
    int v = value.load();  // Default: seq_cst
}
```

**Questions:**
1. What is the default memory order?
2. Can you use relaxed here safely?
3. When do you need stronger ordering?

---

## Part 5: Performance Analysis

### Exercise 5.1: Atomic vs Mutex Performance

```cpp
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <vector>

std::atomic<int> atomicCounter{0};
int mutexCounter = 0;
std::mutex mtx;

void benchmarkAtomic(int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([iterations] {
            for (int j = 0; j < iterations; ++j) {
                atomicCounter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Atomic: " << duration.count() << "ms\n";
}

void benchmarkMutex(int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([iterations] {
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<std::mutex> lock(mtx);
                ++mutexCounter;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Mutex: " << duration.count() << "ms\n";
}

int main() {
    benchmarkAtomic(1000000);
    benchmarkMutex(1000000);
    return 0;
}
```

**Analysis:**
1. Which is faster?
2. How much faster?
3. When should you use each?

---

### Exercise 5.2: Memory Order Impact

```cpp
void benchmarkSeqCst() {
    std::atomic<int> counter{0};
    // Use seq_cst
    counter.fetch_add(1, std::memory_order_seq_cst);
}

void benchmarkRelaxed() {
    std::atomic<int> counter{0};
    // Use relaxed
    counter.fetch_add(1, std::memory_order_relaxed);
}
```

**Questions:**
1. Is there measurable difference?
2. When can you safely use relaxed?

---

## Submission Guidelines

Submit:
1. **answers.md** - MCQ answers
2. **code_review/** - Fixed code
3. **implementations/** - All implementations
4. **debugging/** - Analysis
5. **performance/** - Benchmarks

---

## Evaluation Criteria

- **Correctness (35%):** Proper atomic usage
- **Understanding (25%):** Memory ordering concepts
- **Performance (20%):** Insightful comparisons
- **Code Quality (20%):** Clean implementations

---

## Key Takeaways

✅ Atomics provide lock-free thread-safe operations  
✅ fetch_add, exchange, compare_exchange are key operations  
✅ Memory ordering: relaxed < acquire/release < seq_cst  
✅ Atomics faster than mutexes for simple operations  
✅ Not all types can be atomic (must be trivially copyable)  

---

## Common Pitfalls

❌ Assuming atomics make entire operation atomic  
❌ Using relaxed when synchronization needed  
❌ Not using CAS loop correctly  
❌ Forgetting to check is_lock_free()  

---

## Next Steps

Proceed to **Assignment 08: Memory Model Ordering** for deeper dive into memory ordering.

---

## Resources

- [cppreference - std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapter 5

Good luck!