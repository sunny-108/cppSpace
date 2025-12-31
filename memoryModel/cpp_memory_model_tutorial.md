# C++ Memory Model Tutorial

## What is the C++ Memory Model?

The C++ memory model, introduced in C++11, defines how threads interact through memory and what behaviors are allowed when multiple threads access the same memory location. It provides a foundation for writing portable multithreaded code with well-defined behavior.

## Key Concepts

### 1. **Memory Locations**
- Every variable occupies one or more memory locations
- Different threads can safely access different memory locations simultaneously
- Access to the same memory location from multiple threads requires synchronization

### 2. **Race Conditions**
A data race occurs when:
- Two or more threads access the same memory location
- At least one access is a write
- The accesses are not synchronized

Data races lead to **undefined behavior**.

## Memory Orders

C++ provides six memory ordering options that control how memory operations are ordered:

### 1. `memory_order_relaxed`
- No synchronization or ordering constraints
- Only atomicity is guaranteed
- Fastest but least restrictive

```cpp
#include <atomic>
#include <thread>

std::atomic<int> counter{0};

void increment() {
    for (int i = 0; i < 1000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}
```

### 2. `memory_order_acquire`
- Used for **load** operations
- Prevents reordering of reads/writes after this load
- Synchronizes with `memory_order_release`

### 3. `memory_order_release`
- Used for **store** operations
- Prevents reordering of reads/writes before this store
- Synchronizes with `memory_order_acquire`

```cpp
#include <atomic>
#include <thread>
#include <cassert>

std::atomic<bool> ready{false};
int data = 0;

void producer() {
    data = 42;  // Non-atomic write
    ready.store(true, std::memory_order_release);  // Release
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire
        // Wait
    }
    assert(data == 42);  // Guaranteed to see the write
}
```

### 4. `memory_order_acq_rel`
- Combines acquire and release semantics
- Used for read-modify-write operations

```cpp
std::atomic<int> value{0};

void update() {
    value.fetch_add(1, std::memory_order_acq_rel);
}
```

### 5. `memory_order_seq_cst` (Default)
- **Sequential consistency** - strongest guarantee
- All operations appear in a single global order
- Most intuitive but can be slower

```cpp
std::atomic<int> x{0}, y{0};

void thread1() {
    x.store(1, std::memory_order_seq_cst);
    int r1 = y.load(std::memory_order_seq_cst);
}

void thread2() {
    y.store(1, std::memory_order_seq_cst);
    int r2 = x.load(std::memory_order_seq_cst);
}
// It's impossible for both r1 and r2 to be 0
```

### 6. `memory_order_consume`
- Similar to acquire but weaker
- Only dependent operations are ordered
- Rarely used due to complexity

## Practical Examples

### Example 1: Thread-Safe Counter

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>

class Counter {
    std::atomic<int> count{0};
public:
    void increment() {
        count.fetch_add(1, std::memory_order_relaxed);
    }
    
    int get() const {
        return count.load(std::memory_order_relaxed);
    }
};

int main() {
    Counter counter;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 1000; ++j) {
                counter.increment();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final count: " << counter.get() << std::endl;
    return 0;
}
```

### Example 2: Producer-Consumer Pattern

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

class SpinLockQueue {
    std::vector<int> data;
    std::atomic<bool> ready{false};
    
public:
    void produce(int value) {
        data.push_back(value);
        ready.store(true, std::memory_order_release);
    }
    
    bool consume(int& value) {
        if (ready.load(std::memory_order_acquire)) {
            if (!data.empty()) {
                value = data.back();
                return true;
            }
        }
        return false;
    }
};
```

### Example 3: Double-Checked Locking Pattern

```cpp
#include <atomic>
#include <mutex>
#include <memory>

class Singleton {
    static std::atomic<Singleton*> instance;
    static std::mutex mtx;
    
    Singleton() = default;
    
public:
    static Singleton* getInstance() {
        Singleton* tmp = instance.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mtx);
            tmp = instance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new Singleton();
                instance.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }
};

std::atomic<Singleton*> Singleton::instance{nullptr};
std::mutex Singleton::mtx;
```

## Common Pitfalls

### 1. **Assuming Sequential Consistency**
```cpp
// DON'T: This can have surprising behavior
int data = 0;
std::atomic<bool> flag{false};

// Thread 1
data = 42;
flag.store(true, std::memory_order_relaxed);  // ❌ Relaxed!

// Thread 2
if (flag.load(std::memory_order_relaxed)) {
    // data might not be 42!
}
```

### 2. **Mixing Atomic and Non-Atomic Operations**
```cpp
std::atomic<int> x{0};

// Thread 1
x.store(1);

// Thread 2
int temp = x;  // ❌ Should use x.load()
```

## When to Use Which Memory Order?

| Memory Order | Use Case |
|--------------|----------|
| `relaxed` | Counters, statistics (when you only need atomicity) |
| `acquire/release` | Most synchronization scenarios (recommended default) |
| `seq_cst` | When you need global ordering (default, but slower) |
| `acq_rel` | Read-modify-write operations |
| `consume` | Rarely needed (complex dependency tracking) |

## Performance Considerations

1. **Relaxed** is fastest but requires careful reasoning
2. **Acquire/Release** provides good balance of performance and safety
3. **Sequential Consistency** is slowest but easiest to reason about

## Best Practices

1. ✅ Start with `seq_cst` (default), optimize later if needed
2. ✅ Use `acquire/release` for most producer-consumer patterns
3. ✅ Use `relaxed` only when you understand the implications
4. ✅ Always use atomic operations for shared variables
5. ✅ Document your memory ordering choices
6. ✅ Test multithreaded code thoroughly

## Further Reading

- C++ Standard (ISO/IEC 14882)
- "C++ Concurrency in Action" by Anthony Williams
- [cppreference.com - Memory Order](https://en.cppreference.com/w/cpp/atomic/memory_order)
- "The C++11 Memory Model and GCC" by Hans Boehm

## Conclusion

The C++ memory model provides fine-grained control over how threads interact through memory. While it's complex, understanding it is essential for writing correct and efficient multithreaded code. Start with the stronger guarantees (`seq_cst`) and relax them only when you have a clear understanding of the implications and performance requirements.
