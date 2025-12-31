# C++ Memory Model Exercises

This document contains a series of exercises to help you understand and master the C++ memory model. Each exercise includes the problem, hints, and solutions.

---

## Exercise 1: Basic Atomic Counter (Beginner)

### Problem
Implement a thread-safe counter that can be incremented by multiple threads simultaneously. Create 10 threads, each incrementing the counter 10,000 times. The final count should be exactly 100,000.

**Requirements:**
- Use `std::atomic<int>`
- Choose appropriate memory ordering
- Verify the final count is correct

### Hints
- Which memory order is sufficient for simple increments?
- Do you need synchronization between threads?

### Your Solution
```cpp
// Write your solution here
```

<details>
<summary>Click to see solution</summary>

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>

std::atomic<int> counter{0};

void increment() {
    for (int i = 0; i < 10000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
        // Relaxed is sufficient since we only need atomicity
        // We don't need to synchronize with other operations
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(increment);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final count: " << counter.load() << std::endl;
    // Should print: Final count: 100000
    return 0;
}
```
</details>

---

## Exercise 2: Find the Race Condition (Beginner)

### Problem
The following code has a data race. Identify the problem and fix it.

```cpp
#include <thread>
#include <iostream>

int shared_value = 0;
bool ready = false;

void writer() {
    shared_value = 42;
    ready = true;
}

void reader() {
    while (!ready) {
        // spin
    }
    std::cout << shared_value << std::endl;
}

int main() {
    std::thread t1(writer);
    std::thread t2(reader);
    t1.join();
    t2.join();
    return 0;
}
```

### Questions
1. What is the data race in this code?
2. Could the reader print a value other than 42?
3. How would you fix this using atomics and proper memory ordering?

### Your Solution
```cpp
// Write your fixed version here
```

<details>
<summary>Click to see solution</summary>

**Problems:**
1. `ready` is accessed by multiple threads without synchronization (data race)
2. Even if `ready` were atomic, `shared_value` would still have a race condition
3. The compiler/CPU could reorder operations, so reader might see `ready == true` but old `shared_value`

**Fixed version:**
```cpp
#include <atomic>
#include <thread>
#include <iostream>

int shared_value = 0;
std::atomic<bool> ready{false};

void writer() {
    shared_value = 42;
    ready.store(true, std::memory_order_release);  // Release: all prior writes visible
}

void reader() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire: synchronizes with release
        // spin
    }
    std::cout << shared_value << std::endl;  // Guaranteed to see 42
}

int main() {
    std::thread t1(writer);
    std::thread t2(reader);
    t1.join();
    t2.join();
    return 0;
}
```
</details>

---

## Exercise 3: Message Passing (Intermediate)

### Problem
Implement a simple message passing system where one thread sends a message (string) and another thread receives it. Use atomic flags with proper memory ordering.

**Requirements:**
- Producer sets a message and signals it's ready
- Consumer waits for the signal and reads the message
- No data races
- Use acquire/release semantics

### Your Solution
```cpp
// Write your solution here
```

<details>
<summary>Click to see solution</summary>

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <string>

std::string message;
std::atomic<bool> message_ready{false};

void producer() {
    message = "Hello from producer!";
    message_ready.store(true, std::memory_order_release);
}

void consumer() {
    while (!message_ready.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    std::cout << "Received: " << message << std::endl;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    
    return 0;
}
```
</details>

---

## Exercise 4: Dekker's Algorithm Bug (Intermediate)

### Problem
This is an attempt at implementing a simple mutual exclusion algorithm. However, it uses the wrong memory ordering. Fix it.

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<bool> flag0{false};
std::atomic<bool> flag1{false};
int shared_resource = 0;

void thread0() {
    for (int i = 0; i < 10000; ++i) {
        flag0.store(true, std::memory_order_relaxed);
        while (flag1.load(std::memory_order_relaxed)) {
            // wait
        }
        // Critical section
        shared_resource++;
        flag0.store(false, std::memory_order_relaxed);
    }
}

void thread1() {
    for (int i = 0; i < 10000; ++i) {
        flag1.store(true, std::memory_order_relaxed);
        while (flag0.load(std::memory_order_relaxed)) {
            // wait
        }
        // Critical section
        shared_resource++;
        flag1.store(false, std::memory_order_relaxed);
    }
}

int main() {
    std::thread t0(thread0);
    std::thread t1(thread1);
    t0.join();
    t1.join();
    std::cout << "Final value: " << shared_resource << std::endl;
    return 0;
}
```

### Questions
1. Why doesn't `memory_order_relaxed` work here?
2. What memory ordering should be used?

### Your Solution
```cpp
// Write your fixed version here
```

<details>
<summary>Click to see solution</summary>

**Problem:** With `memory_order_relaxed`, there's no synchronization between threads. Both threads could see the other's flag as false and enter the critical section simultaneously.

**Solution:**
```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<bool> flag0{false};
std::atomic<bool> flag1{false};
int shared_resource = 0;

void thread0() {
    for (int i = 0; i < 10000; ++i) {
        flag0.store(true, std::memory_order_release);
        while (flag1.load(std::memory_order_acquire)) {
            // wait
        }
        // Critical section
        shared_resource++;
        flag0.store(false, std::memory_order_release);
    }
}

void thread1() {
    for (int i = 0; i < 10000; ++i) {
        flag1.store(true, std::memory_order_release);
        while (flag0.load(std::memory_order_acquire)) {
            // wait
        }
        // Critical section
        shared_resource++;
        flag1.store(false, std::memory_order_release);
    }
}

int main() {
    std::thread t0(thread0);
    std::thread t1(thread1);
    t0.join();
    t1.join();
    std::cout << "Final value: " << shared_resource << std::endl;
    // Should be 20000
    return 0;
}
```

Note: This is still not a perfect mutex implementation (it can have issues), but it demonstrates proper memory ordering.
</details>

---

## Exercise 5: Lazy Initialization (Intermediate)

### Problem
Implement a lazy initialization pattern for an expensive resource that should only be created once, even when accessed by multiple threads simultaneously.

**Requirements:**
- The resource should be created only once
- Use double-checked locking pattern
- Use appropriate memory ordering (no mutex!)
- Thread-safe

### Your Solution
```cpp
// Write your solution here
```

<details>
<summary>Click to see solution</summary>

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

class ExpensiveResource {
public:
    ExpensiveResource() {
        std::cout << "Resource created!\n";
        // Simulate expensive initialization
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    void use() {
        std::cout << "Using resource\n";
    }
};

class LazyInitializer {
    std::atomic<ExpensiveResource*> resource{nullptr};
    
public:
    ~LazyInitializer() {
        ExpensiveResource* r = resource.load();
        delete r;
    }
    
    ExpensiveResource* get() {
        ExpensiveResource* r = resource.load(std::memory_order_acquire);
        
        if (r == nullptr) {
            // First check failed, try to create
            r = new ExpensiveResource();
            
            ExpensiveResource* expected = nullptr;
            if (!resource.compare_exchange_strong(expected, r, 
                                                  std::memory_order_release,
                                                  std::memory_order_acquire)) {
                // Someone else created it first
                delete r;
                r = expected;
            }
        }
        
        return r;
    }
};

int main() {
    LazyInitializer initializer;
    std::vector<std::thread> threads;
    
    // Multiple threads try to get the resource
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&initializer]() {
            ExpensiveResource* r = initializer.get();
            r->use();
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```
</details>

---

## Exercise 6: Sequential Consistency Puzzle (Advanced)

### Problem
Consider this code with relaxed memory ordering:

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> x{0}, y{0};
int r1, r2;

void thread1() {
    x.store(1, std::memory_order_relaxed);
    r1 = y.load(std::memory_order_relaxed);
}

void thread2() {
    y.store(1, std::memory_order_relaxed);
    r2 = x.load(std::memory_order_relaxed);
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    t1.join();
    t2.join();
    std::cout << "r1=" << r1 << ", r2=" << r2 << std::endl;
    return 0;
}
```

### Questions
1. What are the possible values of `r1` and `r2`?
2. Is it possible for both `r1 == 0` and `r2 == 0`?
3. Change the code to use `memory_order_seq_cst`. Can both be 0 now?
4. Why does sequential consistency prevent this?

### Your Analysis
```
// Write your analysis here
```

<details>
<summary>Click to see solution</summary>

**With `memory_order_relaxed`:**

Possible outcomes:
- `r1=0, r2=1` (thread2 runs first)
- `r1=1, r2=0` (thread1 runs first)
- `r1=1, r2=1` (both stores visible before loads)
- `r1=0, r2=0` ⚠️ **POSSIBLE with relaxed!** (reordering allowed)

**With `memory_order_seq_cst`:**
```cpp
void thread1() {
    x.store(1, std::memory_order_seq_cst);
    r1 = y.load(std::memory_order_seq_cst);
}

void thread2() {
    y.store(1, std::memory_order_seq_cst);
    r2 = x.load(std::memory_order_seq_cst);
}
```

Now `r1=0, r2=0` is **IMPOSSIBLE** because sequential consistency guarantees a total global order of all operations. If `r1=0`, then `y.store(1)` must come after `x.store(1)` in the global order, which means `r2` must see `x=1`.

**Why?** Sequential consistency ensures all threads agree on a single interleaving of operations.
</details>

---

## Exercise 7: Lock-Free Stack (Advanced)

### Problem
Implement a simple lock-free stack using atomics and compare-and-swap operations.

**Requirements:**
- `push()` operation
- `pop()` operation (returns nullptr if empty)
- Thread-safe without locks
- Use `compare_exchange_weak` or `compare_exchange_strong`

### Your Solution
```cpp
// Write your solution here
```

<details>
<summary>Click to see solution</summary>

```cpp
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>
#include <vector>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(T const& data_) : data(data_), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};
    
public:
    ~LockFreeStack() {
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }
    
    void push(T const& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load(std::memory_order_relaxed);
        
        // Keep trying until we successfully update head
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
            // If CAS fails, new_node->next is updated to current head
            // Loop will retry
        }
    }
    
    std::shared_ptr<T> pop() {
        Node* old_head = head.load(std::memory_order_relaxed);
        
        while (old_head && 
               !head.compare_exchange_weak(old_head, old_head->next,
                                          std::memory_order_acquire,
                                          std::memory_order_relaxed)) {
            // If CAS fails, old_head is updated to current head
            // Loop will retry
        }
        
        if (old_head) {
            std::shared_ptr<T> result = std::make_shared<T>(old_head->data);
            delete old_head;
            return result;
        }
        
        return nullptr;
    }
};

int main() {
    LockFreeStack<int> stack;
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    
    std::vector<std::thread> threads;
    
    // Pushers
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&stack, &push_count, i]() {
            for (int j = 0; j < 1000; ++j) {
                stack.push(i * 1000 + j);
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    // Poppers
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&stack, &pop_count]() {
            for (int j = 0; j < 1000; ++j) {
                if (stack.pop()) {
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Pushed: " << push_count.load() << std::endl;
    std::cout << "Popped: " << pop_count.load() << std::endl;
    
    return 0;
}
```

**Note:** This implementation has the ABA problem! For production code, you'd need hazard pointers or other techniques to handle it safely.
</details>

---

## Exercise 8: Memory Order Performance Test (Advanced)

### Problem
Create a benchmark to measure the performance difference between different memory orderings.

**Requirements:**
- Test `memory_order_relaxed`, `memory_order_acquire/release`, and `memory_order_seq_cst`
- Measure time for 10 million operations
- Compare results
- Explain the performance differences

### Your Solution
```cpp
// Write your solution here
```

<details>
<summary>Click to see solution</summary>

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

void benchmark_relaxed() {
    std::atomic<int> counter{0};
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 2500000; ++j) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Relaxed: " << duration.count() << "ms, final: " << counter.load() << std::endl;
}

void benchmark_acquire_release() {
    std::atomic<int> counter{0};
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 2500000; ++j) {
                counter.fetch_add(1, std::memory_order_acq_rel);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Acq/Rel: " << duration.count() << "ms, final: " << counter.load() << std::endl;
}

void benchmark_seq_cst() {
    std::atomic<int> counter{0};
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 2500000; ++j) {
                counter.fetch_add(1, std::memory_order_seq_cst);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Seq_cst: " << duration.count() << "ms, final: " << counter.load() << std::endl;
}

int main() {
    std::cout << "Running benchmarks (10 million operations)...\n" << std::endl;
    
    benchmark_relaxed();
    benchmark_acquire_release();
    benchmark_seq_cst();
    
    return 0;
}
```

**Typical Results:**
- Relaxed: ~50-100ms (fastest)
- Acquire/Release: ~100-200ms (moderate)
- Sequential Consistency: ~150-300ms (slowest)

**Explanation:** 
- Relaxed has no synchronization overhead
- Acquire/Release adds memory barriers
- Seq_cst enforces global ordering, requiring more expensive synchronization
</details>

---

## Exercise 9: Producer-Consumer Queue (Advanced)

### Problem
Implement a bounded single-producer single-consumer queue using atomics.

**Requirements:**
- Fixed size circular buffer
- `push()` returns false if full
- `pop()` returns false if empty
- Use appropriate memory ordering
- No locks

### Your Solution
```cpp
// Write your solution here
```

<details>
<summary>Click to see solution</summary>

```cpp
#include <atomic>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>

template<typename T, size_t Size>
class SPSCQueue {
private:
    std::vector<T> buffer;
    std::atomic<size_t> write_pos{0};
    std::atomic<size_t> read_pos{0};
    
public:
    SPSCQueue() : buffer(Size) {}
    
    bool push(T const& item) {
        size_t current_write = write_pos.load(std::memory_order_relaxed);
        size_t next_write = (current_write + 1) % Size;
        
        if (next_write == read_pos.load(std::memory_order_acquire)) {
            return false;  // Queue is full
        }
        
        buffer[current_write] = item;
        write_pos.store(next_write, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        size_t current_read = read_pos.load(std::memory_order_relaxed);
        
        if (current_read == write_pos.load(std::memory_order_acquire)) {
            return false;  // Queue is empty
        }
        
        item = buffer[current_read];
        read_pos.store((current_read + 1) % Size, std::memory_order_release);
        return true;
    }
};

int main() {
    SPSCQueue<int, 100> queue;
    std::atomic<bool> done{false};
    std::atomic<int> items_produced{0};
    std::atomic<int> items_consumed{0};
    
    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 10000; ++i) {
            while (!queue.push(i)) {
                std::this_thread::yield();
            }
            items_produced.fetch_add(1, std::memory_order_relaxed);
        }
        done.store(true, std::memory_order_release);
    });
    
    // Consumer thread
    std::thread consumer([&]() {
        int value;
        while (!done.load(std::memory_order_acquire) || items_consumed.load() < 10000) {
            if (queue.pop(value)) {
                items_consumed.fetch_add(1, std::memory_order_relaxed);
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "Produced: " << items_produced.load() << std::endl;
    std::cout << "Consumed: " << items_consumed.load() << std::endl;
    
    return 0;
}
```
</details>

---

## Exercise 10: Debugging Challenge (Expert)

### Problem
This code sometimes fails (prints incorrect values or crashes). Find ALL the bugs and fix them.

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

struct Data {
    int value1;
    int value2;
};

std::atomic<Data*> shared_data{nullptr};

void writer() {
    for (int i = 0; i < 1000; ++i) {
        Data* d = new Data{i, i * 2};
        Data* old = shared_data.exchange(d, std::memory_order_relaxed);
        delete old;
    }
}

void reader() {
    for (int i = 0; i < 1000; ++i) {
        Data* d = shared_data.load(std::memory_order_relaxed);
        if (d != nullptr) {
            std::cout << d->value1 << ", " << d->value2 << std::endl;
        }
    }
}

int main() {
    std::vector<std::thread> threads;
    threads.emplace_back(writer);
    threads.emplace_back(reader);
    
    for (auto& t : threads) {
        t.join();
    }
    
    delete shared_data.load();
    return 0;
}
```

### Questions
1. What are all the bugs in this code?
2. How would you fix each one?
3. What memory ordering should be used?

### Your Solution
```cpp
// Write your fixed version here
```

<details>
<summary>Click to see solution</summary>

**Bugs:**
1. **Use-after-free:** Reader might access `d` after writer deletes it
2. **Memory ordering:** Relaxed doesn't synchronize the Data construction with the read
3. **Race on delete:** Writer might delete data that reader is still using
4. **Incomplete initialization visibility:** Even if pointer is seen, Data members might not be visible

**Fixed version (using reference counting):**
```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <memory>

struct Data {
    int value1;
    int value2;
};

std::atomic<std::shared_ptr<Data>> shared_data;

void writer() {
    for (int i = 0; i < 1000; ++i) {
        auto d = std::make_shared<Data>(Data{i, i * 2});
        shared_data.store(d, std::memory_order_release);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void reader() {
    for (int i = 0; i < 1000; ++i) {
        auto d = shared_data.load(std::memory_order_acquire);
        if (d != nullptr) {
            std::cout << d->value1 << ", " << d->value2 << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

int main() {
    std::vector<std::thread> threads;
    threads.emplace_back(writer);
    threads.emplace_back(reader);
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

**Key fixes:**
- Use `shared_ptr` for automatic memory management
- Use `memory_order_acquire/release` for proper synchronization
- Reference counting prevents use-after-free
</details>

---

## Bonus Challenge: Create Your Own Exercise!

Now that you've completed these exercises, try creating your own scenario that demonstrates a specific aspect of the C++ memory model. Share it with others to test their understanding!

---

## Additional Resources

- Compile with: `g++ -std=c++17 -pthread -O2 -o program program.cpp`
- Use Thread Sanitizer: `g++ -std=c++17 -pthread -fsanitize=thread -g -o program program.cpp`
- Test with: `valgrind --tool=helgrind ./program`

## Tips for Learning

1. Start with exercises 1-3, make sure you understand them completely
2. Compile and run each exercise to see the behavior
3. Use thread sanitizer to detect race conditions
4. Try breaking the code (use wrong memory order) to see what happens
5. Experiment with different thread counts and iteration counts
6. Read the solutions only after attempting the problem yourself

Good luck! 🚀
