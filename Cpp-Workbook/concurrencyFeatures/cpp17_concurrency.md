# C++17 Concurrency Features

C++17 built upon C++11/14's foundation with significant additions, particularly parallel algorithms and improved synchronization primitives.

## Parallel Algorithms (C++17)

### Execution Policies
- **Purpose**: Specify how STL algorithms should execute (sequential, parallel, vectorized)
- **Policies**:
  - `std::execution::seq`: Sequential execution (default)
  - `std::execution::par`: Parallel execution
  - `std::execution::par_unseq`: Parallel and vectorized execution
  - `std::execution::unseq`: Vectorized execution (C++20)

### Parallel Algorithm Support
- **Supported Algorithms**: Most STL algorithms (`for_each`, `transform`, `reduce`, `sort`, etc.)
- **Example**: 
  ```cpp
  std::sort(std::execution::par, vec.begin(), vec.end());
  std::transform(std::execution::par_unseq, 
                 v1.begin(), v1.end(), v2.begin(), result.begin(), 
                 std::plus<>());
  ```

### New Parallel Algorithms
- `std::reduce()`: Parallel version of accumulate (order-independent)
- `std::transform_reduce()`: Parallel map-reduce operation
- `std::exclusive_scan()` / `std::inclusive_scan()`: Parallel prefix sum
- `std::transform_exclusive_scan()` / `std::transform_inclusive_scan()`

### Benefits
- **Easy Parallelization**: Add execution policy to existing code
- **Performance**: Automatic multi-core utilization
- **Safety**: Library handles synchronization

## Shared Mutex (C++17)

### `std::shared_mutex`
- **Purpose**: Non-timed version of `std::shared_timed_mutex` (C++14)
- **Performance**: More efficient than timed version when timeout not needed
- **Use Case**: Reader-writer locks (multiple readers, exclusive writer)
- **Methods**: 
  - Exclusive: `lock()`, `unlock()`, `try_lock()`
  - Shared: `lock_shared()`, `unlock_shared()`, `try_lock_shared()`
- **Example**:
  ```cpp
  std::shared_mutex mtx;
  
  // Multiple readers
  std::shared_lock<std::shared_mutex> read_lock(mtx);
  
  // Single writer
  std::unique_lock<std::shared_mutex> write_lock(mtx);
  ```

## Improved Lock Management (C++17)

### `std::scoped_lock`
- **Purpose**: RAII lock for multiple mutexes simultaneously
- **Benefit**: Deadlock-free locking of multiple mutexes
- **Replaces**: Manual use of `std::lock()` with multiple `std::lock_guard`
- **Variadic Template**: Can lock any number of mutexes
- **Example**:
  ```cpp
  std::scoped_lock lock(mutex1, mutex2, mutex3);
  // All mutexes locked, automatic deadlock avoidance
  // All automatically unlocked on scope exit
  ```

### Advantages Over Previous Approaches
- **Cleaner Syntax**: Single line vs multiple lock_guards
- **Exception Safe**: Automatic unlock even with exceptions
- **Deadlock Prevention**: Uses `std::lock()` internally

## Hardware Concurrency Information (C++17)

### Hardware Interference Sizes
- `std::hardware_destructive_interference_size`
  - **Purpose**: Minimum offset between two objects to avoid false sharing
  - **Use Case**: Cache line optimization for concurrent data structures
  - **Example**: Align data to prevent false sharing
  ```cpp
  struct alignas(std::hardware_destructive_interference_size) AlignedData {
      std::atomic<int> value;
  };
  ```

- `std::hardware_constructive_interference_size`
  - **Purpose**: Maximum size of contiguous memory for true sharing
  - **Use Case**: Pack related data in same cache line
  - **Example**: Optimize related atomics for cache locality

### Benefits
- **Performance**: Explicit control over cache line effects
- **Portability**: Platform-independent cache optimization

## Atomic Improvements (C++17)

### `std::atomic<T>::is_always_lock_free`
- **Purpose**: Compile-time constant indicating if type is always lock-free
- **Difference from `is_lock_free()`**: Compile-time vs runtime check
- **Use Case**: Template specialization, static assertions
- **Example**:
  ```cpp
  static_assert(std::atomic<int>::is_always_lock_free);
  ```

## Memory Model Refinements (C++17)

### Clearer Specifications
- **Improved Wording**: More precise definition of data races and synchronization
- **Clarifications**: Better specification of memory ordering guarantees
- **Consistent Semantics**: Resolved ambiguities from C++11/14

## Summary

C++17's concurrency additions focused on:

1. **Parallel Algorithms**: Easy parallelization of STL algorithms with execution policies
2. **`std::shared_mutex`**: Efficient reader-writer synchronization
3. **`std::scoped_lock`**: Simplified multi-mutex locking without deadlocks
4. **Hardware Interference Sizes**: Cache-aware data structure design
5. **Atomic Improvements**: Better compile-time information

These features made it easier to write high-performance concurrent code while reducing common pitfalls like deadlocks and false sharing. The parallel algorithms particularly democratized parallel programming by making it as simple as adding an execution policy parameter.
