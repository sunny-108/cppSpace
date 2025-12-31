# C++23 Concurrency Features

C++23 continued refining concurrency support with improvements to existing features and some new additions, though it was a smaller update compared to C++20.

## Thread Management Enhancements (C++23)

### `std::thread` Constructor with Attributes

- **Purpose**: Platform-specific thread attributes (stack size, priority, etc.)
- **Portable Interface**: Standardized way to set thread attributes
- **Use Case**: Fine-tuned thread configuration

### `std::jthread::get_stop_source()`

- [ ] **Purpose**: Access the stop_source from jthread
- [ ] **Benefit**: Share stop_source with other operations
- [ ] **Use Case**: Coordinate cancellation across multiple components

## Atomic Operations Improvements (C++23)

### Freestanding Atomics

- **Purpose**: Atomics available in freestanding implementations
- **Benefit**: Use atomics in embedded/kernel environments
- **Coverage**: Core atomic types and operations

### `std::atomic<T>::wait()` / `notify_*()` Improvements

- **Enhanced Specification**: Better defined behavior and guarantees
- **Performance**: More efficient implementations encouraged
- **Portability**: More consistent across platforms

## Synchronization Primitives (C++23)

### `std::barrier::arrive()` Return Value

- **Change**: Returns arrival token for later waiting
- **Benefit**: Separate arrival from waiting
- **Use Case**: Asynchronous barrier patterns
- **Example**:
  ```cpp
  auto token = barrier.arrive();
  // Do other work
  barrier.wait(std::move(token)); // Wait later
  ```

## Library Support Additions (C++23)

### `std::move_only_function`

- **Purpose**: Function wrapper that cannot be copied (only moved)
- **Concurrency Relevance**: More efficient for thread tasks
- **Benefit**: Avoid unnecessary copies of large captures
- **Use Case**: Pass expensive callables to threads
- **Example**:
  ```cpp
  std::move_only_function<void()> task = [unique_data = std::move(data)]() {
      // Use unique_data
  };
  std::jthread t(std::move(task));
  ```

### `std::expected<T, E>`

- **Purpose**: Type for functions that may fail (like Result in Rust)
- **Concurrency Relevance**: Better error handling in async operations
- **Alternative to Exceptions**: Explicit error handling without exceptions
- **Use Case**: Return values or errors from thread functions
- **Example**:
  ```cpp
  std::expected<int, std::string> compute() {
      if (error_condition)
          return std::unexpected("Error occurred");
      return 42;
  }

  auto result = std::async(compute);
  if (result.get().has_value()) {
      // Success
  } else {
      // Handle error: result.get().error()
  }
  ```

## Stack Trace Support (C++23)

### `std::stacktrace`

- **Purpose**: Capture and examine stack traces
- **Concurrency Use**: Better debugging of multithreaded programs
- **Methods**: Capture current stack, iterate frames
- **Use Case**: Log stack traces during deadlocks or errors
- **Example**:
  ```cpp
  void log_with_trace() {
      auto trace = std::stacktrace::current();
      std::cout << "Thread " << std::this_thread::get_id() 
                << " trace:\n" << trace;
  }
  ```

## Execution and Async Enhancements (C++23)

### `std::execution` Refinements

- **Clarifications**: Better specification of parallel algorithm execution
- **Consistency**: More predictable behavior across implementations
- **Note**: Full "Senders/Receivers" execution model deferred to C++26

### Async Operation Support

- **Improved Integration**: Better compatibility between async features
- **Error Handling**: Better propagation of exceptions in async chains

## Memory Model Refinements (C++23)

### Mixed-Size Atomics

- **Clarification**: Behavior when atomics of different sizes share memory
- **Padding Bits**: Better specification of padding in atomic operations
- **Alignment**: More precise alignment requirements

### Memory Ordering Documentation

- **Improved Examples**: More examples in standard specification
- **Clearer Guidance**: Better explanation of when to use each ordering

## Utility Improvements for Concurrency (C++23)

### `std::byteswap`

- **Purpose**: Efficient byte order swapping
- **Relevance**: Useful in network programming and concurrent I/O
- **Benefit**: Portable endianness handling

### `std::unreachable()`

- **Purpose**: Mark unreachable code for optimization
- **Concurrency Use**: Optimize state machines in concurrent code
- **Benefit**: Better code generation in switch statements

### Multidimensional Subscript Operator

- **Syntax**: `operator[](x, y, z)`
- **Relevance**: More efficient concurrent data structure access
- **Use Case**: Multidimensional thread-safe containers

## Standard Library Concurrency Patterns (C++23)

### `std::out_ptr` and `std::inout_ptr`

- **Purpose**: Safer interop with C APIs
- **Concurrency Relevance**: Thread-safe wrapping of C API calls
- **Benefit**: RAII for output parameters

## Formatting Thread IDs (C++23)

### `std::format` Support for Thread IDs

- **Purpose**: Format thread IDs directly with `std::format`
- **Benefit**: Easier logging in multithreaded applications
- **Example**:
  ```cpp
  auto tid = std::this_thread::get_id();
  std::string msg = std::format("Thread {} starting", tid);
  ```

## Ranges and Views for Concurrent Data (C++23)

### `std::ranges::to`

- **Purpose**: Convert ranges to containers
- **Concurrency Use**: Collect parallel algorithm results
- **Example**:
  ```cpp
  auto results = views::transform(data, compute)
                 | std::ranges::to<std::vector>();
  ```

### Zip Views

- **Purpose**: Iterate multiple ranges in parallel
- **Relevance**: Simplify parallel data processing code
- **Example**: `for (auto [a, b] : std::views::zip(vec1, vec2))`

## What Didn't Make It

### Executors (Deferred)

- **Status**: Full sender/receiver model postponed to C++26
- **Reason**: Design complexity, need for more implementation experience
- **Impact**: Would have provided unified async execution framework

### Networking

- **Status**: Still not standardized (targeted for future)
- **Networking TS**: Exists but not yet in standard
- **Impact**: Would greatly benefit async I/O patterns

## Compiler and Library Support (C++23)

### Implementation Status

- **Variable Support**: C++23 features being gradually implemented
- **Compilers**: GCC 13+, Clang 17+, MSVC 19.37+ have partial support
- **Libraries**: libstdc++, libc++, MSVC STL implementing features

## Summary

C++23's concurrency improvements were more incremental:

1. **`std::move_only_function`**: Efficient non-copyable callables
2. **`std::expected`**: Better error handling for async operations
3. **`std::stacktrace`**: Debugging multithreaded programs
4. **Atomic Refinements**: Better specification and freestanding support
5. **Barrier Improvements**: Separate arrive/wait operations
6. **Memory Model Clarifications**: Mixed-size atomics, clearer specifications
7. **Utility Additions**: Better formatting, debugging, and interop

While C++23 didn't introduce revolutionary concurrency features like C++20, it focused on refinement, usability improvements, and setting the stage for future developments (particularly the executor framework planned for C++26).

## Looking Forward

### C++26 and Beyond

- **Executors**: Expected to bring unified async execution model
- **Networking**: Possible standardization of networking library
- **Coroutine Libraries**: Higher-level standard coroutine types
- **Concurrent Data Structures**: More lock-free containers
- **Transaction Memory**: Possible future addition

The evolution of C++ concurrency continues toward higher-level abstractions while maintaining zero-overhead principles.
