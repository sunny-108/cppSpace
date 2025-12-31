# Chapter 7 Concurrency Project - Effective Modern C++

## Project Overview

This project is designed to help you master Chapter 7 (Concurrency) from "Effective Modern C++" by Scott Meyers. You'll build a comprehensive understanding of modern C++ concurrency primitives through hands-on coding exercises.

## Learning Objectives

By completing this project, you will:
1. Understand when to use task-based vs. thread-based programming
2. Master std::async launch policies and their implications
3. Learn about future destructor behavior and shared state
4. Use void futures for one-shot event communication
5. Distinguish between std::atomic and volatile correctly

## Project Structure

```
effectiveCpp/concurrency/
├── chapter7_project.md          (This file)
├── item35-*.md                  (Explanations)
├── item36-*.md
├── item38-*.md
├── item39-*.md
├── item40-*.md
├── assignment_item35.md         (Programming assignments)
├── assignment_item36.md
├── assignment_item37.md
├── assignment_item38.md
├── assignment_item39.md
├── assignment_item40.md
├── mcq_item35.md               (Multiple choice questions)
├── mcq_item36.md
├── mcq_item37.md
├── mcq_item38.md
├── mcq_item39.md
├── mcq_item40.md
└── solutions/                  (Your implementations)
    ├── item35_solution.cpp
    ├── item36_solution.cpp
    ├── item37_solution.cpp
    ├── item38_solution.cpp
    ├── item39_solution.cpp
    └── item40_solution.cpp
```

## Items Covered

### Item 35: Prefer Task-Based Programming to Thread-Based
**Focus**: Using std::async instead of std::thread
- Return value handling
- Exception propagation
- Automatic resource management
- Thread pool optimization

### Item 36: Specify std::launch::async if Asynchronicity is Essential
**Focus**: Understanding launch policies
- std::launch::async vs. std::launch::deferred
- Default policy pitfalls
- Timeout behavior
- Thread-local storage issues

### Item 37: Make std::threads Unjoinable on All Paths
**Focus**: RAII for thread management
- Dangers of unjoinable threads
- Using join() and detach() correctly
- Creating RAII wrappers for threads
- Exception safety

### Item 38: Be Aware of Varying Thread Handle Destructor Behavior
**Focus**: Future destructor blocking behavior
- When futures block on destruction
- Shared state and reference counting
- packaged_task vs. async differences
- Implications for program flow

### Item 39: Consider void Futures for One-Shot Event Communication
**Focus**: Using futures for synchronization
- void futures vs. condition variables
- One-shot notification patterns
- shared_future for multiple waiters
- When to use each approach

### Item 40: Use std::atomic for Concurrency, volatile for Special Memory
**Focus**: Understanding atomic vs. volatile
- Thread safety with std::atomic
- Memory ordering guarantees
- volatile for hardware I/O
- Common misconceptions

## Project Phases

### Phase 1: Study (Week 1)
1. Read the explanation files (item35-40)
2. Understand the code examples
3. Run the provided examples
4. Take notes on key concepts

### Phase 2: Practice (Week 2)
1. Complete programming assignments for each item
2. Test your implementations thoroughly
3. Compare with provided solutions
4. Debug and refine your code

### Phase 3: Assessment (Week 3)
1. Take MCQ tests for each item
2. Review incorrect answers
3. Revisit explanation materials
4. Retake tests until mastery

### Phase 4: Real-World Application (Week 4)
1. Build the capstone project (below)
2. Apply all learned concepts
3. Document your design decisions
4. Optimize for performance

## Capstone Project: Concurrent Task Scheduler

Build a production-quality concurrent task scheduler that demonstrates all concepts from Chapter 7.

### Requirements:

```cpp
class TaskScheduler {
public:
    // Item 35: Task-based interface
    template<typename F, typename... Args>
    auto submitTask(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>>;
    
    // Item 36: Explicit launch policies
    template<typename F, typename... Args>
    auto submitAsyncTask(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;
    
    // Item 37: RAII thread management
    ~TaskScheduler(); // Must ensure all threads joined
    
    // Item 39: Shutdown notification
    void shutdown();
    bool isShutdown() const;
    
    // Item 40: Thread-safe statistics
    std::atomic<size_t> completedTasks{0};
    std::atomic<size_t> failedTasks{0};
    
private:
    // Implementation details
};
```

### Features to Implement:

1. **Thread Pool** (Item 35)
   - Fixed number of worker threads
   - Task queue
   - Automatic load balancing

2. **Launch Policy Support** (Item 36)
   - Force immediate execution
   - Deferred execution option
   - Default with smart selection

3. **RAII Thread Management** (Item 37)
   - ThreadGuard wrapper
   - Automatic joining on destruction
   - Exception safety

4. **Proper Future Handling** (Item 38)
   - Understand blocking behavior
   - Document shared state lifetime
   - Avoid accidental blocks

5. **Shutdown Signal** (Item 39)
   - void future for shutdown notification
   - Graceful worker termination
   - Wait for in-flight tasks

6. **Thread-Safe Counters** (Item 40)
   - std::atomic for statistics
   - Lock-free operations
   - No volatile misuse

### Example Usage:

```cpp
int main() {
    TaskScheduler scheduler;
    
    // Submit various tasks
    auto future1 = scheduler.submitTask([]() { 
        return compute(); 
    });
    
    auto future2 = scheduler.submitAsyncTask([](int x) { 
        return x * 2; 
    }, 42);
    
    // Get results
    auto result1 = future1.get();
    auto result2 = future2.get();
    
    // Check statistics
    std::cout << "Completed: " << scheduler.completedTasks << std::endl;
    
    // Graceful shutdown
    scheduler.shutdown();
    
    return 0;
} // Scheduler destructor ensures all threads joined
```

## Building and Running

### Compiler Requirements:
- C++14 or later
- Support for std::thread, std::async, std::atomic
- pthread support (Linux/macOS) or Windows threading

### Build Commands:

```bash
# Linux/macOS
g++ -std=c++17 -pthread -o solution item35_solution.cpp
./solution

# Or with CMake
mkdir build && cd build
cmake ..
make
./chapter7_tests
```

### Sample CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.10)
project(Chapter7Concurrency)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Individual exercises
add_executable(item35 solutions/item35_solution.cpp)
add_executable(item36 solutions/item36_solution.cpp)
add_executable(item37 solutions/item37_solution.cpp)
add_executable(item38 solutions/item38_solution.cpp)
add_executable(item39 solutions/item39_solution.cpp)
add_executable(item40 solutions/item40_solution.cpp)

# Link threads
target_link_libraries(item35 Threads::Threads)
target_link_libraries(item36 Threads::Threads)
target_link_libraries(item37 Threads::Threads)
target_link_libraries(item38 Threads::Threads)
target_link_libraries(item39 Threads::Threads)
target_link_libraries(item40 Threads::Threads)

# Capstone project
add_executable(task_scheduler capstone/task_scheduler.cpp)
target_link_libraries(task_scheduler Threads::Threads)
```

## Testing Strategy

### Unit Tests
- Test individual concepts in isolation
- Verify thread safety with concurrent access
- Check for memory leaks (valgrind, sanitizers)

### Integration Tests
- Combine multiple concepts
- Test edge cases and error conditions
- Stress test with many threads/tasks

### Performance Tests
- Measure overhead of abstractions
- Compare task-based vs. thread-based
- Profile with perf/Instruments

### Commands:

```bash
# Run with address sanitizer
g++ -std=c++17 -pthread -fsanitize=address -g solution.cpp
./a.out

# Run with thread sanitizer
g++ -std=c++17 -pthread -fsanitize=thread -g solution.cpp
./a.out

# Check for memory leaks
valgrind --leak-check=full ./solution
```

## Common Pitfalls to Avoid

1. **Item 35**: Using std::thread when std::async would be simpler
2. **Item 36**: Relying on default launch policy with timeouts
3. **Item 37**: Forgetting to join or detach threads
4. **Item 38**: Not understanding when futures block
5. **Item 39**: Using condition variables for simple one-shot events
6. **Item 40**: Using volatile for thread synchronization

## Resources

### Documentation:
- [cppreference - std::async](https://en.cppreference.com/w/cpp/thread/async)
- [cppreference - std::future](https://en.cppreference.com/w/cpp/thread/future)
- [cppreference - std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [cppreference - std::thread](https://en.cppreference.com/w/cpp/thread/thread)

### Books:
- "Effective Modern C++" by Scott Meyers (Chapter 7)
- "C++ Concurrency in Action" by Anthony Williams
- "The C++ Programming Language" by Bjarne Stroustrup

### Tools:
- Thread sanitizer (TSan)
- Address sanitizer (ASan)
- Valgrind (Helgrind for threading)
- gdb/lldb for debugging

## Success Criteria

You have mastered Chapter 7 when you can:
- [ ] Explain why task-based is preferred over thread-based
- [ ] Choose the correct launch policy for std::async
- [ ] Implement RAII wrappers for std::thread
- [ ] Predict when future destructors will block
- [ ] Use void futures for one-shot synchronization
- [ ] Distinguish std::atomic from volatile
- [ ] Complete all programming assignments
- [ ] Score 90%+ on all MCQ tests
- [ ] Build the capstone project successfully
- [ ] Explain your design decisions clearly

## Next Steps

After completing this project:
1. Explore C++20 concurrency features (jthread, latch, barrier, semaphore)
2. Study lock-free programming in depth
3. Learn about memory models and ordering
4. Practice with real-world concurrent applications
5. Read "C++ Concurrency in Action" for advanced topics

## Notes and Observations

Use this space to document your learning journey:

### Key Insights:
- 

### Challenges Faced:
- 

### Solutions Discovered:
- 

### Performance Lessons:
- 

### Future Study Topics:
- 

---

**Good luck with your concurrency journey! Remember: Concurrency is hard, but modern C++ makes it manageable.**
