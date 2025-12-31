# Assignment 05: Atomic Operations & Memory Ordering

## Overview
Deep dive into C++ memory model, atomic operations, and memory ordering semantics. Learn to write correct lock-free code and understand hardware-level synchronization.

**Estimated Time:** 8-10 hours

---

## Learning Objectives
- Master C++ memory model and happens-before relationships
- Understand memory ordering (relaxed, acquire, release, seq_cst)
- Write correct lock-free algorithms
- Use atomics effectively for synchronization
- Avoid data races and optimize memory access patterns

---

## Part 1: Multiple Choice Questions (MCQs)

### Q1. What does `std::memory_order_seq_cst` guarantee?
A) Only atomicity  
B) Sequential consistency - total order across all atomic operations  
C) Just compiler reordering prevention  
D) Only same-thread ordering  

**Answer:** B

---

### Q2. `std::memory_order_relaxed` provides:
A) No ordering guarantees, only atomicity  
B) Acquire-release semantics  
C) Sequential consistency  
D) Synchronization between threads  

**Answer:** A

---

### Q3. An acquire load synchronized with a release store establishes:
A) Nothing special  
B) A happens-before relationship  
C) A total ordering  
D) Mutual exclusion  

**Answer:** B

---

### Q4. Which memory order is typically used for reference counting?
A) `memory_order_relaxed` for increment, `memory_order_acquire` for decrement  
B) `memory_order_seq_cst` for all operations  
C) `memory_order_relaxed` for increment, `memory_order_acq_rel` for decrement  
D) `memory_order_relaxed` for all operations  

**Answer:** C

---

### Q5. `compare_exchange_weak` vs `compare_exchange_strong`:
A) No difference  
B) Weak can spuriously fail even when comparison succeeds  
C) Strong is always faster  
D) Weak doesn't support memory ordering  

**Answer:** B

---

### Q6. A data race occurs when:
A) Two threads access the same variable  
B) Two threads access shared data without synchronization, at least one writes  
C) Threads finish in unexpected order  
D) Atomic operations are used  

**Answer:** B

---

### Q7. `std::atomic<T>` is lock-free if:
A) Always  
B) Only for primitive types  
C) `is_lock_free()` returns true (hardware dependent)  
D) Never, it always uses locks  

**Answer:** C

---

### Q8. Memory fences (`std::atomic_thread_fence`) are useful for:
A) Preventing compiler optimizations only  
B) Establishing ordering between non-atomic and atomic operations  
C) Creating locks  
D) Allocating memory  

**Answer:** B

---

### Q9. Which is the weakest memory ordering that provides synchronization?
A) `memory_order_relaxed`  
B) `memory_order_consume`  
C) `memory_order_acquire` / `memory_order_release`  
D) `memory_order_seq_cst`  

**Answer:** C

---

### Q10. False sharing occurs when:
A) Two variables share the same cache line, causing unnecessary synchronization  
B) Atomic operations fail  
C) Memory ordering is incorrect  
D) Threads share pointers  

**Answer:** A

---

### Q11. `std::atomic<std::shared_ptr<T>>` was added in C++20 because:
A) Regular atomics couldn't handle it  
B) Shared pointer operations need special atomic handling for control block  
C) It's faster than mutexes  
D) It prevents memory leaks  

**Answer:** B

---

### Q12. A spinlock implemented with `std::atomic_flag`:
A) Is always the best choice  
B) Burns CPU while waiting but avoids syscalls  
C) Prevents deadlocks  
D) Is lock-free  

**Answer:** B

---

### Q13. The `memory_order_consume` (discouraged) was intended for:
A) Sequential consistency  
B) Dependency-ordered synchronization  
C) Resource destruction  
D) Thread creation  

**Answer:** B

---

### Q14. Which atomic operation is read-modify-write (RMW)?
A) `load()`  
B) `store()`  
C) `fetch_add()`  
D) All of the above  

**Answer:** C

---

### Q15. In lock-free programming, the ABA problem occurs when:
A) Value changes from A to B and back to A, misleading CAS  
B) Two threads access variables A and B  
C) Memory ordering is wrong  
D) Atomic operations fail  

**Answer:** A

---

## Part 2: Code Review Exercises

### Exercise 2.1: Memory Ordering Bugs

```cpp
class SpinLock {
    std::atomic<bool> flag{false};
    
public:
    void lock() {
        while (flag.exchange(true, std::memory_order_relaxed)) {
            while (flag.load(std::memory_order_relaxed)) {
                // Spin
            }
        }
    }
    
    void unlock() {
        flag.store(false, std::memory_order_relaxed);
    }
};

int sharedData = 0;
SpinLock lock;

void thread1() {
    lock.lock();
    sharedData = 42;
    lock.unlock();
}

void thread2() {
    lock.lock();
    int value = sharedData;
    lock.unlock();
    assert(value == 42 || value == 0);  // Can this fail?
}
```

**Questions:**
1. What's wrong with the memory ordering in `lock()`?
2. What's wrong with `unlock()`'s ordering?
3. Can `sharedData` operations be reordered?
4. What guarantees are missing?
5. Fix with correct memory orders and explain each

---

### Exercise 2.2: Reference Counting Bug

```cpp
template<typename T>
class RefCounted {
    T* ptr;
    std::atomic<int> refCount;
    
public:
    RefCounted(T* p) : ptr(p), refCount(1) {}
    
    void addRef() {
        refCount.fetch_add(1, std::memory_order_relaxed);
    }
    
    void release() {
        if (refCount.fetch_sub(1, std::memory_order_relaxed) == 1) {
            delete ptr;
        }
    }
    
    T* get() { return ptr; }
};

void test() {
    auto* obj = new RefCounted<int>(new int(42));
    
    std::thread t1([obj] {
        int value = *obj->get();
        obj->release();
    });
    
    std::thread t2([obj] {
        obj->release();
    });
    
    t1.join();
    t2.join();
}
```

**Questions:**
1. What's wrong with the memory ordering in `release()`?
2. Why is relaxed insufficient for the final decrement?
3. Can thread1 access deleted memory?
4. What memory order should be used for decrement?
5. Is `addRef()` correct with relaxed?

---

### Exercise 2.3: Incorrect Acquire-Release

```cpp
class WorkQueue {
    std::atomic<Node*> head{nullptr};
    
    struct Node {
        int data;
        Node* next;
    };
    
public:
    void push(int value) {
        Node* newNode = new Node{value, nullptr};
        Node* oldHead = head.load(std::memory_order_relaxed);
        
        do {
            newNode->next = oldHead;
        } while (!head.compare_exchange_weak(oldHead, newNode,
                                            std::memory_order_relaxed,
                                            std::memory_order_relaxed));
    }
    
    bool pop(int& value) {
        Node* oldHead = head.load(std::memory_order_relaxed);
        
        while (oldHead != nullptr) {
            if (head.compare_exchange_weak(oldHead, oldHead->next,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed)) {
                value = oldHead->data;
                delete oldHead;
                return true;
            }
        }
        
        return false;
    }
};
```

**Questions:**
1. What memory ordering is needed in `push()`?
2. What memory ordering is needed in `pop()`?
3. Can other threads see uninitialized `newNode->next`?
4. Can deleted memory be accessed?
5. Fix all memory ordering issues

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Correct Spinlock with Various Memory Orders

Implement spinlocks with different memory ordering strategies:

```cpp
// Version 1: Sequential consistency (easiest, slowest)
class SpinLockSeqCst {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
public:
    void lock();
    void unlock();
};

// Version 2: Acquire-Release (standard, efficient)
class SpinLockAcqRel {
    std::atomic<bool> flag{false};
    
public:
    void lock();
    void unlock();
    bool try_lock();
};

// Version 3: Optimized with backoff
class SpinLockBackoff {
    std::atomic<bool> flag{false};
    
public:
    void lock() {
        int backoff = 1;
        while (flag.exchange(true, std::memory_order_acquire)) {
            while (flag.load(std::memory_order_relaxed)) {
                for (int i = 0; i < backoff; ++i) {
                    // Pause instruction (x86: _mm_pause())
                }
                backoff = std::min(backoff * 2, 1024);
            }
            backoff = 1;
        }
    }
    
    void unlock() {
        flag.store(false, std::memory_order_release);
    }
};

// Version 4: Ticket lock for fairness
class TicketLock {
    std::atomic<size_t> nowServing{0};
    std::atomic<size_t> nextTicket{0};
    
public:
    void lock();
    void unlock();
};

// Benchmark all versions
class SpinLockBenchmark {
public:
    struct Results {
        std::string lockType;
        double throughput;
        double fairnessVariance;
        double cpuUsage;
    };
    
    Results benchmark(size_t numThreads, size_t iterations);
    void compareAll();
};
```

**Requirements:**
- Correct memory ordering for each version
- Explain why each memory order is chosen
- Benchmark contention levels (low, medium, high)
- Measure fairness (variance in acquisitions per thread)
- Compare with `std::mutex`

---

### Exercise 3.2: Lock-Free Stack

```cpp
template<typename T>
class LockFreeStack {
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};
    
public:
    void push(const T& value) {
        // Implement with correct memory ordering
    }
    
    bool pop(T& value) {
        // Implement with correct memory ordering
        // Handle ABA problem (discuss, don't necessarily solve)
    }
    
    bool empty() const {
        return head.load(std::memory_order_acquire) == nullptr;
    }
    
    ~LockFreeStack() {
        T dummy;
        while (pop(dummy));
    }
};

// Stress test
void testLockFreeStack() {
    LockFreeStack<int> stack;
    std::atomic<int> pushCount{0};
    std::atomic<int> popCount{0};
    
    const int numThreads = 8;
    const int opsPerThread = 100000;
    
    auto worker = [&](int id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        
        for (int i = 0; i < opsPerThread; ++i) {
            if (dis(gen) == 0) {
                stack.push(id * opsPerThread + i);
                pushCount++;
            } else {
                int value;
                if (stack.pop(value)) {
                    popCount++;
                }
            }
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Pop remaining items
    int value;
    while (stack.pop(value)) {
        popCount++;
    }
    
    std::cout << "Pushed: " << pushCount << ", Popped: " << popCount << "\n";
    assert(pushCount == popCount);
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Duration: " << duration.count() << "ms\n";
}
```

**Requirements:**
- Correct memory ordering (explain each choice)
- Handle CAS failures properly
- Discuss ABA problem and potential solutions
- Verify with ThreadSanitizer
- Compare performance with locked version

---

### Exercise 3.3: Sequential Lock (SeqLock)

Implement a sequence lock for read-optimized scenarios:

```cpp
template<typename T>
class SeqLock {
public:
    SeqLock() : sequence(0) {}
    
    // Writers lock exclusively
    void write(const T& value) {
        // Implement: increment sequence (odd), write, increment sequence (even)
        // Use proper memory ordering
    }
    
    // Readers don't lock, but verify consistency
    T read() const {
        // Implement: read sequence, read data, verify sequence unchanged and even
        // Retry if inconsistent
        // Use proper memory ordering
    }
    
private:
    alignas(64) std::atomic<uint64_t> sequence;
    alignas(64) T data;
    alignas(64) std::atomic<bool> writerLock{false};
};

// Test with high read contention
struct Point3D {
    double x, y, z;
};

void testSeqLock() {
    SeqLock<Point3D> position;
    std::atomic<bool> done{false};
    std::atomic<uint64_t> reads{0};
    std::atomic<uint64_t> writes{0};
    
    // One writer
    std::thread writer([&] {
        for (int i = 0; i < 100000; ++i) {
            Point3D p{i * 1.0, i * 2.0, i * 3.0};
            position.write(p);
            writes++;
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        done = true;
    });
    
    // Multiple readers
    auto reader = [&](int id) {
        while (!done) {
            Point3D p = position.read();
            reads++;
            
            // Verify consistency
            assert(p.x * 2.0 == p.y);
            assert(p.x * 3.0 == p.z);
        }
    };
    
    std::vector<std::thread> readers;
    for (int i = 0; i < 8; ++i) {
        readers.emplace_back(reader, i);
    }
    
    writer.join();
    for (auto& r : readers) {
        r.join();
    }
    
    std::cout << "Writes: " << writes << ", Reads: " << reads << "\n";
}
```

**Requirements:**
- Correct use of odd/even sequence numbers
- Proper memory ordering (critical!)
- Handle reader retries
- Alignment to avoid false sharing
- Compare with RW lock

---

## Part 4: Debugging Concurrent Code

### Exercise 4.1: Data Race Detection

```cpp
class Counter {
    std::atomic<int> count{0};
    int lastValue = 0;  // Not atomic!
    
public:
    void increment() {
        int newValue = count.fetch_add(1, std::memory_order_relaxed) + 1;
        lastValue = newValue;  // Data race!
    }
    
    int getCount() const {
        return count.load(std::memory_order_relaxed);
    }
    
    int getLastValue() const {
        return lastValue;  // Data race!
    }
};

void test() {
    Counter counter;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 1000; ++j) {
                counter.increment();
                int last = counter.getLastValue();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final count: " << counter.getCount() << "\n";
}
```

**Tasks:**
1. Compile with ThreadSanitizer: `-fsanitize=thread`
2. Document all data races found
3. Explain why mixing atomic and non-atomic is dangerous
4. Fix all issues
5. Verify fix with sanitizer

---

### Exercise 4.2: Memory Ordering Bug Hunt

```cpp
// Dekker's algorithm (mutual exclusion)
class DekkerLock {
    std::atomic<bool> flag0{false};
    std::atomic<bool> flag1{false};
    std::atomic<int> turn{0};
    
public:
    void lock0() {
        flag0.store(true, std::memory_order_relaxed);
        while (flag1.load(std::memory_order_relaxed)) {
            if (turn.load(std::memory_order_relaxed) != 0) {
                flag0.store(false, std::memory_order_relaxed);
                while (turn.load(std::memory_order_relaxed) != 0);
                flag0.store(true, std::memory_order_relaxed);
            }
        }
    }
    
    void unlock0() {
        turn.store(1, std::memory_order_relaxed);
        flag0.store(false, std::memory_order_relaxed);
    }
    
    void lock1() {
        flag1.store(true, std::memory_order_relaxed);
        while (flag0.load(std::memory_order_relaxed)) {
            if (turn.load(std::memory_order_relaxed) != 1) {
                flag1.store(false, std::memory_order_relaxed);
                while (turn.load(std::memory_order_relaxed) != 1);
                flag1.store(true, std::memory_order_relaxed);
            }
        }
    }
    
    void unlock1() {
        turn.store(0, std::memory_order_relaxed);
        flag1.store(false, std::memory_order_relaxed);
    }
};

int sharedCounter = 0;
DekkerLock lock;

void testDekker() {
    std::thread t0([&] {
        for (int i = 0; i < 1000000; ++i) {
            lock.lock0();
            sharedCounter++;
            lock.unlock0();
        }
    });
    
    std::thread t1([&] {
        for (int i = 0; i < 1000000; ++i) {
            lock.lock1();
            sharedCounter++;
            lock.unlock1();
        }
    });
    
    t0.join();
    t1.join();
    
    std::cout << "Counter: " << sharedCounter << " (expected: 2000000)\n";
}
```

**Tasks:**
1. Run test - does it always produce correct result?
2. Identify necessary memory orders for correctness
3. Explain happens-before relationships needed
4. Fix memory orders
5. Why is this inferior to modern primitives?

---

## Part 5: Performance Optimization

### Exercise 5.1: False Sharing Elimination

```cpp
// Version 1: False sharing
struct CounterV1 {
    std::atomic<uint64_t> count{0};
};

class ParallelCounterV1 {
    std::vector<CounterV1> counters;
    
public:
    ParallelCounterV1(size_t numThreads) : counters(numThreads) {}
    
    void increment(size_t threadId) {
        counters[threadId].count.fetch_add(1, std::memory_order_relaxed);
    }
    
    uint64_t total() const {
        uint64_t sum = 0;
        for (const auto& c : counters) {
            sum += c.count.load(std::memory_order_relaxed);
        }
        return sum;
    }
};

// Version 2: Padded to avoid false sharing
struct alignas(64) CounterV2 {
    std::atomic<uint64_t> count{0};
};

class ParallelCounterV2 {
    std::vector<CounterV2> counters;
    
public:
    ParallelCounterV2(size_t numThreads) : counters(numThreads) {}
    
    void increment(size_t threadId) {
        counters[threadId].count.fetch_add(1, std::memory_order_relaxed);
    }
    
    uint64_t total() const {
        uint64_t sum = 0;
        for (const auto& c : counters) {
            sum += c.count.load(std::memory_order_relaxed);
        }
        return sum;
    }
};

// Benchmark both
void benchmarkFalseSharing() {
    const int numThreads = 8;
    const int increments = 10000000;
    
    // Test V1
    {
        ParallelCounterV1 counter(numThreads);
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([&, i] {
                for (int j = 0; j < increments; ++j) {
                    counter.increment(i);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "V1 (false sharing): " << duration.count() << "ms\n";
        std::cout << "Total: " << counter.total() << "\n";
    }
    
    // Test V2
    {
        ParallelCounterV2 counter(numThreads);
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([&, i] {
                for (int j = 0; j < increments; ++j) {
                    counter.increment(i);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "V2 (padded): " << duration.count() << "ms\n";
        std::cout << "Total: " << counter.total() << "\n";
    }
}
```

**Tasks:**
1. Benchmark both versions
2. Use `perf` to measure cache misses
3. Vary number of threads (1-32)
4. Graph performance difference
5. Explain cache line effects

---

### Exercise 5.2: Memory Order Optimization

Benchmark different memory orders for a counter:

```cpp
class MemoryOrderBenchmark {
    template<std::memory_order Order>
    static void benchmarkIncrement(size_t numThreads, size_t iterations) {
        std::atomic<uint64_t> counter{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back([&] {
                for (size_t j = 0; j < iterations; ++j) {
                    counter.fetch_add(1, Order);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Order: " << Order << ", Threads: " << numThreads
                  << ", Time: " << duration.count() << "us\n";
    }
    
public:
    static void run() {
        for (size_t threads : {1, 2, 4, 8, 16}) {
            std::cout << "\n=== " << threads << " threads ===\n";
            
            benchmarkIncrement<std::memory_order_relaxed>(threads, 1000000);
            benchmarkIncrement<std::memory_order_release>(threads, 1000000);
            benchmarkIncrement<std::memory_order_acq_rel>(threads, 1000000);
            benchmarkIncrement<std::memory_order_seq_cst>(threads, 1000000);
        }
    }
};
```

**Analysis:**
1. Run benchmarks
2. Graph results
3. Explain performance differences
4. When is each order appropriate?
5. Measure with hardware performance counters

---

## Submission Guidelines

Same format as previous assignments.

---

## Evaluation Criteria

- **Correctness (40%):** Proper memory ordering, no data races
- **Understanding (25%):** Clear explanations of memory model concepts
- **Performance (20%):** Efficient use of memory orders
- **Analysis (15%):** Thorough benchmarking and profiling

---

## Additional Resources

- [C++ Concurrency in Action, 2nd Ed. - Chapter 5](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)
- [cppreference - Memory Order](https://en.cppreference.com/w/cpp/atomic/memory_order)
- [Jeff Preshing - Memory Ordering Articles](https://preshing.com/archives/)
- [Herb Sutter - atomic Weapons](https://herbsutter.com/2013/02/11/atomic-weapons-the-c-memory-model-and-modern-hardware/)

---

Good luck with the memory model!
