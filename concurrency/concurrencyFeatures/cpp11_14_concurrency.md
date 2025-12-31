# C++11/14 Concurrency Features

C++11 introduced a standardized threading model and comprehensive concurrency support for the first time. C++14 added minor enhancements.

## Core Threading (C++11)

### `std::thread`
- **Purpose**: Represents a single thread of execution
- **Usage**: Create and manage threads directly
- **Key Methods**: `join()`, `detach()`, `joinable()`, `get_id()`
- **Example**: `std::thread t([]{ std::cout << "Hello from thread\n"; });`

### Thread Management
- `std::this_thread` namespace with utilities:
  - `get_id()`: Get current thread ID
  - `sleep_for()`, `sleep_until()`: Thread sleeping
  - `yield()`: Hint to reschedule thread

## Mutual Exclusion (C++11)

### `std::mutex`
- **Purpose**: Basic mutual exclusion primitive
- **Key Methods**: `lock()`, `unlock()`, `try_lock()`
- **Variants**: `std::recursive_mutex`, `std::timed_mutex`, `std::recursive_timed_mutex`

### `std::lock_guard<T>`
- **Purpose**: RAII-style mutex wrapper
- **Benefit**: Automatic unlock on scope exit
- **Usage**: `std::lock_guard<std::mutex> lock(mtx);`

### `std::unique_lock<T>`
- **Purpose**: More flexible lock wrapper
- **Features**: Deferred locking, time-constrained locking, transfer of ownership
- **Additional Methods**: `lock()`, `unlock()`, `try_lock()`, `owns_lock()`

### Lock Functions
- `std::lock()`: Lock multiple mutexes without deadlock
- `std::try_lock()`: Try to lock multiple mutexes

## Synchronization (C++11)

### `std::condition_variable`
- **Purpose**: Thread synchronization primitive for waiting/notification
- **Key Methods**: `wait()`, `wait_for()`, `wait_until()`, `notify_one()`, `notify_all()`
- **Use Case**: Producer-consumer patterns, thread coordination

### `std::condition_variable_any`
- **Purpose**: Works with any lock type (not just `std::unique_lock`)

## Atomic Operations (C++11)

### `std::atomic<T>`
- **Purpose**: Lock-free atomic operations on data types
- **Operations**: `load()`, `store()`, `exchange()`, `compare_exchange_weak/strong()`
- **Specialized Types**: `std::atomic_bool`, `std::atomic_int`, etc.

### Memory Ordering
- **Orderings**: 
  - `memory_order_relaxed`: No synchronization
  - `memory_order_acquire/release`: Acquire-release semantics
  - `memory_order_acq_rel`: Both acquire and release
  - `memory_order_seq_cst`: Sequentially consistent (default)
  - `memory_order_consume`: Data dependency ordering

### Atomic Flags
- `std::atomic_flag`: Guaranteed lock-free boolean atomic
- **Methods**: `test_and_set()`, `clear()`

## Asynchronous Programming (C++11)

### `std::future<T>`
- **Purpose**: Represents a value that will be available in the future
- **Key Methods**: `get()`, `wait()`, `wait_for()`, `wait_until()`, `valid()`
- **Use Case**: Retrieve result from asynchronous operation

### `std::promise<T>`
- **Purpose**: Sets a value that can be retrieved by a future
- **Key Methods**: `set_value()`, `set_exception()`, `get_future()`
- **Use Case**: Communication channel to pass value to future

### `std::async`
- **Purpose**: Run function asynchronously and return a future
- **Launch Policies**: 
  - `std::launch::async`: Guarantee asynchronous execution
  - `std::launch::deferred`: Lazy evaluation
  - Default: Implementation chooses
- **Example**: `auto fut = std::async(std::launch::async, []{ return 42; });`

### `std::packaged_task<T>`
- **Purpose**: Wrapper for callable object that stores result for future
- **Use Case**: Decouple task execution from result retrieval

### `std::shared_future<T>`
- **Purpose**: Multiple threads can wait for the same shared state
- **Difference from future**: Copyable, multiple threads can access

## Thread-Local Storage (C++11)

### `thread_local` Keyword
- **Purpose**: Declare thread-specific data
- **Lifetime**: Exists for the duration of the thread
- **Example**: `thread_local int counter = 0;`

## C++14 Additions

### `std::shared_timed_mutex`
- **Purpose**: Shared mutex with timed locking capabilities
- **Use Case**: Multiple readers, single writer scenarios
- **Methods**: `lock_shared()`, `unlock_shared()`, `try_lock_shared_for()`

### `std::shared_lock<T>`
- **Purpose**: RAII shared lock wrapper for shared mutexes
- **Use Case**: Reader locks in read-write scenarios
- **Example**: `std::shared_lock<std::shared_timed_mutex> lock(mtx);`

## Memory Model (C++11)

### Memory Model Guarantees
- **Sequential Consistency**: Default for atomics
- **Happens-Before Relationship**: Defines order of operations
- **Synchronizes-With**: Establishes ordering between threads
- **Data Races**: Defined as undefined behavior when non-atomic concurrent access occurs

### Memory Fences
- `std::atomic_thread_fence()`: Establish synchronization without atomic operations
- `std::atomic_signal_fence()`: Fence for signal handlers

## Summary

C++11/14 established the foundation for portable multithreaded C++ programming with:
- Thread creation and management (`std::thread`)
- Mutual exclusion primitives (`std::mutex`, RAII wrappers)
- Condition variables for synchronization
- Atomic operations with memory ordering control
- High-level async programming (`std::async`, `std::future`)
- Formal memory model with clear concurrency guarantees
- Shared locking support (C++14)

These features enabled writing concurrent code without relying on platform-specific APIs.
