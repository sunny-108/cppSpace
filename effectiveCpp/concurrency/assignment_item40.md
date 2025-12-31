# Programming Assignment - Item 40: Use std::atomic for Concurrency, volatile for Special Memory

## Learning Objectives
- Distinguish std::atomic from volatile correctly
- Use std::atomic for thread-safe operations
- Understand when to use volatile (special memory)
- Recognize common mistakes and anti-patterns

---

## Exercise 1: Race Condition Demonstration (Easy)

### Problem
Demonstrate the race condition with volatile and fix it with std::atomic.

### Requirements
```cpp
class RaceConditionDemo {
public:
    // Broken: Uses volatile (NOT thread-safe)
    static int brokenCounter(int numThreads, int iterations);
    
    // Fixed: Uses std::atomic (thread-safe)
    static int fixedCounter(int numThreads, int iterations);
    
    // Compare and show lost updates
    static void demonstrateRace();
};
```

### Example
```cpp
RaceConditionDemo::demonstrateRace();
// Output:
// volatile version: Expected 10000, got 8743 (lost 1257 updates!)
// atomic version: Expected 10000, got 10000 ✓
```

### Hints
- Use volatile int for broken version
- Use std::atomic<int> for fixed version
- Run same increment in multiple threads

---

## Exercise 2: Atomic Operations (Easy)

### Problem
Implement thread-safe operations using std::atomic.

### Requirements
```cpp
class AtomicOperations {
public:
    AtomicOperations();
    
    void increment();
    void decrement();
    int get() const;
    
    int fetchAndAdd(int value);
    bool compareAndSwap(int expected, int desired);
    
    void reset();
    
private:
    std::atomic<int> value;
};
```

### Example
```cpp
AtomicOperations ops;

std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&ops]() {
        for (int j = 0; j < 1000; ++j) {
            ops.increment();
        }
    });
}

for (auto& t : threads) t.join();
assert(ops.get() == 10000);  // Always correct
```

### Hints
- Use ++ and -- operators
- Use fetch_add() for add-and-return
- Use compare_exchange_strong()

---

## Exercise 3: volatile for Hardware (Medium)

### Problem
Simulate hardware register access using volatile correctly.

### Requirements
```cpp
class HardwareRegister {
public:
    explicit HardwareRegister(uint32_t* address);
    
    uint32_t read() const;
    void write(uint32_t value);
    
    void setBit(int bitPosition);
    void clearBit(int bitPosition);
    bool testBit(int bitPosition) const;
    
private:
    volatile uint32_t* const reg;
};
```

### Example
```cpp
// Simulated hardware register at fixed address
uint32_t hwMemory = 0;
HardwareRegister reg(&hwMemory);

reg.write(0xFF);
uint32_t value = reg.read();  // Always reads from memory
reg.setBit(5);  // Actually writes to memory
```

### Hints
- Use volatile uint32_t*
- Don't cache register reads
- Each access goes to memory

---

## Exercise 4: Atomic vs Volatile Benchmark (Medium)

### Problem
Compare performance of atomic vs volatile vs regular variables.

### Requirements
```cpp
class PerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string type;
        double singleThreadTime;
        double multiThreadTime;
        int lostUpdates;
    };
    
    BenchmarkResult benchmarkRegular(int ops);
    BenchmarkResult benchmarkVolatile(int ops);
    BenchmarkResult benchmarkAtomic(int ops);
    
    void printComparison();
};
```

### Example Output
```
Single Thread (1M ops):
  Regular:  5ms
  volatile: 12ms
  atomic:   15ms

Multi Thread (1M ops, 4 threads):
  Regular:  8ms (lost 250000 updates!)
  volatile: 18ms (lost 180000 updates!)
  atomic:   45ms (0 lost updates) ✓
```

### Hints
- Time operations with std::chrono
- Compare correctness and speed
- Show atomic is slower but correct

---

## Exercise 5: Lock-Free Stack (Hard)

### Problem
Implement a simple lock-free stack using std::atomic.

### Requirements
```cpp
template<typename T>
class LockFreeStack {
public:
    void push(const T& value);
    bool pop(T& value);
    
    bool empty() const;
    size_t size() const;  // Approximate
    
private:
    struct Node {
        T data;
        Node* next;
    };
    
    std::atomic<Node*> head{nullptr};
    std::atomic<size_t> count{0};
};
```

### Example
```cpp
LockFreeStack<int> stack;

// Multiple threads pushing
std::vector<std::thread> pushers;
for (int i = 0; i < 10; ++i) {
    pushers.emplace_back([&stack, i]() {
        stack.push(i);
    });
}

// Multiple threads popping
std::vector<std::thread> poppers;
for (int i = 0; i < 10; ++i) {
    poppers.emplace_back([&stack]() {
        int value;
        stack.pop(value);
    });
}
```

### Hints
- Use compare_exchange for push/pop
- Handle ABA problem (simplified version ok)
- Atomic operations on pointer

---

## Exercise 6: Memory Ordering (Advanced)

### Problem
Demonstrate different memory ordering guarantees.

### Requirements
```cpp
class MemoryOrderingDemo {
public:
    // Relaxed ordering
    static void demonstrateRelaxed();
    
    // Acquire-release ordering
    static void demonstrateAcquireRelease();
    
    // Sequential consistency
    static void demonstrateSeqCst();
    
    static void explainDifferences();
};
```

### Example
```cpp
// Acquire-Release pattern
std::atomic<bool> ready{false};
int data = 0;

// Writer thread
data = 42;
ready.store(true, std::memory_order_release);

// Reader thread
while (!ready.load(std::memory_order_acquire));
assert(data == 42);  // Guaranteed to see 42
```

### Hints
- Show happens-before relationships
- Demonstrate synchronization
- Explain when to use each ordering

---

## Exercise 7: Thread-Safe Counter Comparison (Medium)

### Problem
Implement same counter using different synchronization methods.

### Requirements
```cpp
class CounterComparison {
public:
    // Different implementations
    class AtomicCounter {
    public:
        void increment();
        int get() const;
    private:
        std::atomic<int> value{0};
    };
    
    class MutexCounter {
    public:
        void increment();
        int get() const;
    private:
        int value{0};
        mutable std::mutex mutex;
    };
    
    class VolatileCounter {  // BROKEN
    public:
        void increment();
        int get() const;
    private:
        volatile int value{0};
    };
    
    static void compareAll();
};
```

### Example Output
```
Atomic: 10000 (correct) - 25ms
Mutex:  10000 (correct) - 120ms
volatile: 8456 (WRONG) - 15ms
```

### Hints
- Show atomic is fast and correct
- Mutex is slow but correct
- volatile is broken for threading

---

## Exercise 8: Atomic Flags and Spinlock (Medium)

### Problem
Implement a spinlock using std::atomic_flag.

### Requirements
```cpp
class Spinlock {
public:
    void lock();
    void unlock();
    bool try_lock();
    
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

// RAII wrapper
class SpinlockGuard {
public:
    explicit SpinlockGuard(Spinlock& lock);
    ~SpinlockGuard();
    
private:
    Spinlock& spinlock;
};
```

### Example
```cpp
Spinlock lock;
int shared = 0;

auto worker = [&]() {
    for (int i = 0; i < 1000; ++i) {
        SpinlockGuard guard(lock);
        ++shared;
    }
};

std::thread t1(worker);
std::thread t2(worker);

t1.join();
t2.join();

assert(shared == 2000);
```

### Hints
- Use test_and_set() for lock
- Use clear() for unlock
- Spin while locked

---

## Exercise 9: Producer-Consumer with Atomics (Hard)

### Problem
Implement a lock-free single-producer single-consumer queue.

### Requirements
```cpp
template<typename T, size_t Size>
class SPSCQueue {
public:
    bool push(const T& value);
    bool pop(T& value);
    
    bool empty() const;
    bool full() const;
    
private:
    std::array<T, Size> buffer;
    std::atomic<size_t> writePos{0};
    std::atomic<size_t> readPos{0};
};
```

### Example
```cpp
SPSCQueue<int, 100> queue;

// Producer
std::thread producer([&queue]() {
    for (int i = 0; i < 1000; ++i) {
        while (!queue.push(i));
    }
});

// Consumer
std::thread consumer([&queue]() {
    int value;
    for (int i = 0; i < 1000; ++i) {
        while (!queue.pop(value));
        assert(value == i);
    }
});
```

### Hints
- Use atomic positions
- Single producer/consumer simplifies
- Memory ordering important

---

## Exercise 10: Misconception Checker (Medium)

### Problem
Create a tool that detects common misuses of volatile for threading.

### Requirements
```cpp
class AtomicVolatileMisuse {
public:
    // Common mistakes
    static void mistake1_VolatileForFlag();
    static void mistake2_VolatileForCounter();
    static void mistake3_VolatileForPointer();
    
    // Correct versions
    static void correct1_AtomicForFlag();
    static void correct2_AtomicForCounter();
    static void correct3_AtomicForPointer();
    
    static void explainMistakes();
};
```

### Example
```cpp
// WRONG
volatile bool done = false;
std::thread t([&done]() {
    doWork();
    done = true;  // Race condition!
});

// CORRECT
std::atomic<bool> done{false};
std::thread t([&done]() {
    doWork();
    done.store(true);  // Thread-safe!
});
```

### Hints
- Show common volatile mistakes
- Explain why they fail
- Provide atomic corrections

---

## Bonus Challenge 1: Atomic Smart Pointer

### Problem
Implement reference counting using std::atomic.

### Requirements
```cpp
template<typename T>
class AtomicRefCount {
public:
    explicit AtomicRefCount(T* ptr);
    
    void addRef();
    void release();
    
    T* get() const;
    size_t useCount() const;
    
private:
    T* ptr;
    std::atomic<size_t> refCount;
};
```

---

## Bonus Challenge 2: Memory Mapped I/O Simulator

### Problem
Simulate memory-mapped I/O device using volatile.

### Requirements
```cpp
class MMIODevice {
public:
    MMIODevice();
    
    // Control registers (volatile!)
    volatile uint32_t& control();
    volatile uint32_t& status();
    volatile uint32_t& data();
    
    // High-level operations
    void sendCommand(uint32_t cmd);
    uint32_t receiveData();
    bool isReady() const;
    
private:
    struct Registers {
        volatile uint32_t control;
        volatile uint32_t status;
        volatile uint32_t data;
    };
    
    Registers regs;
};
```

---

## Testing Your Solutions

### Test Correctness
```cpp
void testAtomicCorrectness() {
    std::atomic<int> counter{0};
    
    constexpr int THREADS = 10;
    constexpr int OPS = 10000;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < OPS; ++j) {
                counter++;
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    assert(counter == THREADS * OPS);
}

void testVolatileFailure() {
    volatile int counter = 0;
    
    // Same test - will likely fail
    // Shows volatile doesn't provide atomicity
}
```

### Verification Checklist
- [ ] Atomic operations are thread-safe
- [ ] volatile used only for special memory
- [ ] No volatile for thread synchronization
- [ ] Understand memory ordering
- [ ] Lock-free structures work correctly
- [ ] Performance characteristics understood

---

## Common Anti-Patterns

### Anti-Pattern 1: volatile for Synchronization
```cpp
// ❌ WRONG
volatile bool flag = false;

std::thread t([&flag]() {
    flag = true;  // NOT ATOMIC!
});

while (!flag);  // NOT SAFE!

// ✅ CORRECT
std::atomic<bool> flag{false};

std::thread t([&flag]() {
    flag.store(true);
});

while (!flag.load());
```

### Anti-Pattern 2: Assuming volatile is Atomic
```cpp
// ❌ WRONG
volatile int counter = 0;
++counter;  // NOT ATOMIC! (read-modify-write)

// ✅ CORRECT
std::atomic<int> counter{0};
++counter;  // ATOMIC!
```

### Anti-Pattern 3: Using volatile for Performance
```cpp
// ❌ WRONG (trying to "optimize")
volatile int* ptr = data;
// Actually prevents optimizations!

// ✅ CORRECT
int* ptr = data;
// Compiler can optimize
```

---

## When to Use What

| Use Case | Use This | NOT This |
|----------|----------|----------|
| Thread synchronization | `std::atomic` | `volatile` |
| Shared counter | `std::atomic` | `volatile` |
| Flags between threads | `std::atomic` | `volatile` |
| Hardware registers | `volatile` | `std::atomic` |
| Memory-mapped I/O | `volatile` | `std::atomic` |
| Signal handlers | `volatile sig_atomic_t` | `std::atomic` |

---

## Grading Rubric

| Criteria | Points |
|----------|--------|
| Correct atomic usage | 30 |
| Understanding volatile purpose | 25 |
| Thread safety | 20 |
| Performance awareness | 15 |
| Documentation | 10 |
| **Total** | **100** |

---

## Key Takeaways

1. **std::atomic** = Thread synchronization, concurrent access
2. **volatile** = Special memory, prevent compiler optimization
3. They are **NOT** interchangeable
4. Never use volatile thinking it makes code thread-safe
5. Atomic operations have overhead but guarantee correctness
6. Understand memory ordering for advanced use
