# C++20 Concurrency Features

C++20 brought major improvements to concurrency with higher-level synchronization primitives, better thread management, and the introduction of coroutines.

## Thread Management Improvements (C++20)

### `std::jthread` (Joining Thread)
- **Purpose**: RAII-based thread that auto-joins on destruction
- **Key Features**:
  - Automatically joins in destructor (no need for manual `join()`)
  - Supports cooperative cancellation via stop tokens
  - Safer than `std::thread` (no risk of `std::terminate` from unjoined thread)
- **Example**:
  ```cpp
  {
      std::jthread t([]{ /* work */ });
      // Automatically joins when going out of scope
  }
  ```

### Stop Tokens (Cooperative Cancellation)
- **`std::stop_token`**: Query object for cancellation requests
- **`std::stop_source`**: Creates stop tokens and requests stops
- **`std::stop_callback`**: Registers callback for stop requests
- **Use Case**: Gracefully cancel long-running operations
- **Example**:
  ```cpp
  std::jthread t([](std::stop_token stoken) {
      while (!stoken.stop_requested()) {
          // Do work
      }
  });
  t.request_stop(); // Signal thread to stop
  ```

## Synchronization Primitives (C++20)

### `std::counting_semaphore<N>`
- **Purpose**: Classic semaphore with counter
- **Template Parameter**: Maximum count value
- **Key Methods**:
  - `acquire()`: Decrement counter (block if zero)
  - `release(n)`: Increment counter by n
  - `try_acquire()`, `try_acquire_for()`, `try_acquire_until()`
- **Use Case**: Resource pooling, rate limiting
- **Example**:
  ```cpp
  std::counting_semaphore<10> pool(10); // 10 resources
  pool.acquire();  // Get resource
  // Use resource
  pool.release();  // Return resource
  ```

### `std::binary_semaphore`
- **Purpose**: Semaphore with maximum count of 1
- **Type Alias**: `std::counting_semaphore<1>`
- **Use Case**: Signaling between threads
- **Difference from Mutex**: Can be released by different thread than acquired

### `std::latch`
- **Purpose**: Single-use countdown synchronization point
- **Key Methods**:
  - `count_down(n)`: Decrement counter
  - `wait()`: Block until counter reaches zero
  - `arrive_and_wait(n)`: Decrement and wait
- **Use Case**: Wait for multiple threads to complete initialization
- **Single-Use**: Cannot be reset after countdown reaches zero
- **Example**:
  ```cpp
  std::latch done(3); // Wait for 3 threads
  
  // In worker threads:
  // Do work...
  done.count_down();
  
  // In main thread:
  done.wait(); // Wait for all 3 to finish
  ```

### `std::barrier`
- **Purpose**: Reusable synchronization point for multiple threads
- **Key Methods**:
  - `arrive()`: Indicate arrival at barrier
  - `wait()`: Block until all threads arrive
  - `arrive_and_wait()`: Arrive and wait in one call
  - `arrive_and_drop()`: Arrive and reduce participant count
- **Reusable**: Automatically resets after all threads arrive
- **Completion Function**: Optional callback when all threads arrive
- **Use Case**: Phased computations, iterative algorithms
- **Example**:
  ```cpp
  std::barrier sync_point(3, []() noexcept {
      std::cout << "All threads synchronized!\n";
  });
  
  // In each thread (repeated phases):
  for (int phase = 0; phase < 10; ++phase) {
      // Do work for this phase
      sync_point.arrive_and_wait(); // Synchronize between phases
  }
  ```

## Atomic Enhancements (C++20)

### `std::atomic_ref<T>`
- **Purpose**: Atomic operations on non-atomic objects
- **Use Case**: Add atomic operations to existing data without changing its type
- **Lifetime**: Must not outlive the referenced object
- **Example**:
  ```cpp
  int regular_int = 0;
  {
      std::atomic_ref<int> atomic_view(regular_int);
      atomic_view.fetch_add(1, std::memory_order_relaxed);
  }
  ```

### Atomic Smart Pointers
- **`std::atomic<std::shared_ptr<T>>`**: Atomic operations on shared pointers
- **`std::atomic<std::weak_ptr<T>>`**: Atomic operations on weak pointers
- **Purpose**: Thread-safe reference counting and pointer updates
- **Replaces**: Non-standard `std::atomic_*` free functions for shared_ptr
- **Methods**: `load()`, `store()`, `exchange()`, `compare_exchange_*`
- **Example**:
  ```cpp
  std::atomic<std::shared_ptr<Data>> ptr = std::make_shared<Data>();
  
  // Thread-safe update
  auto expected = ptr.load();
  auto desired = std::make_shared<Data>();
  ptr.compare_exchange_strong(expected, desired);
  ```

### Atomic Floating-Point
- **Specializations**: `std::atomic<float>`, `std::atomic<double>`
- **Operations**: `fetch_add()`, `fetch_sub()` for floating-point types
- **Use Case**: Atomic updates to floating-point counters/accumulators

### `std::atomic<T>::wait()` / `notify_one()` / `notify_all()`
- **Purpose**: Efficient waiting on atomic values (similar to condition variables)
- **Methods**:
  - `wait(old_value)`: Block until value changes from old_value
  - `notify_one()`: Wake one waiting thread
  - `notify_all()`: Wake all waiting threads
- **Benefit**: More efficient than polling
- **Example**:
  ```cpp
  std::atomic<int> value = 0;
  
  // Thread 1:
  value.wait(0); // Wait until value != 0
  
  // Thread 2:
  value.store(42);
  value.notify_one();
  ```

## Coroutines (C++20)

### Core Keywords
- **`co_await`**: Suspend execution until operation completes
- **`co_yield`**: Yield value and suspend (generators)
- **`co_return`**: Return from coroutine

### Coroutine Concepts
- **Coroutine Handle**: `std::coroutine_handle<Promise>`
- **Promise Type**: Defines coroutine behavior
- **Awaitable**: Types that can be co_awaited

### Benefits for Concurrency
- **Asynchronous I/O**: Write async code like synchronous
- **Generators**: Lazy evaluation and data streaming
- **Task Abstraction**: Higher-level async programming
- **No Callback Hell**: Linear code flow for async operations

### Example (Conceptual):
```cpp
Task<int> async_compute() {
    int result = co_await async_operation();
    co_return result * 2;
}

Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}
```

### Note on Coroutines
- **Low-Level**: C++20 provides machinery, not ready-to-use types
- **Libraries Needed**: Requires custom or third-party Promise/Awaitable types
- **Learning Curve**: Complex to implement, but powerful when mastered

## Memory Model Updates (C++20)

### `std::atomic_flag` Enhancements
- **`test()`**: Read flag value without modifying
- **Constructor**: Can initialize to set state
- **Improved Usability**: More than just test-and-set

### Memory Ordering Clarifications
- **Refined Semantics**: Clearer specification of ordering guarantees
- **Consistency**: Better alignment with hardware memory models

## Source Location for Debugging (C++20)

### `std::source_location`
- **Purpose**: Capture source information (file, line, function)
- **Concurrency Use**: Better logging/debugging in multithreaded code
- **Example**:
  ```cpp
  void log(std::string_view msg, 
           std::source_location loc = std::source_location::current()) {
      std::cout << loc.file_name() << ":" << loc.line() 
                << " [" << std::this_thread::get_id() << "] " 
                << msg << '\n';
  }
  ```

## Summary

C++20 represented a major leap in concurrency support:

1. **`std::jthread`**: RAII thread with automatic joining and cancellation
2. **Stop Tokens**: Cooperative cancellation mechanism
3. **Semaphores**: `counting_semaphore` and `binary_semaphore`
4. **Latches & Barriers**: High-level synchronization for multi-phase algorithms
5. **`std::atomic_ref`**: Atomic operations on existing objects
6. **Atomic Smart Pointers**: Thread-safe shared_ptr operations
7. **Atomic Wait/Notify**: Efficient waiting without polling
8. **Coroutines**: Foundation for async programming (though low-level)

These additions moved C++ toward higher-level concurrency abstractions while maintaining the performance and control C++ is known for. The synchronization primitives (latch, barrier, semaphore) address common patterns that previously required manual implementation.
