# C++ Concurrency Assignments - Complete Series

## Overview
A comprehensive series of 12 assignments designed to master C++ concurrency for experienced developers (7+ years). Each assignment combines theory (MCQs), code review, implementation, debugging, and performance optimization.

---

## Assignment List

### Foundational Level (Assignments 1-3)
1. **Advanced Thread Management** - Thread pools, thread-local storage, affinity
2. **Mutex & Lock Strategies** - Lock hierarchies, reader-writer locks, fine-grained locking
3. **Deadlock Prevention & Detection** - Real-world scenarios, prevention strategies

### Intermediate Level (Assignments 4-7)
4. **Condition Variables & Semaphores** - Advanced synchronization, spurious wakeups
5. **Atomic Operations & Memory Ordering** - Memory model, acquire-release semantics
6. **Lock-Free Data Structures** - ABA problem, hazard pointers, Michael-Scott queue
7. **Futures, Promises & Async** - Task-based parallelism, continuations

### Advanced Level (Assignments 8-12)
8. **Thread-Safe Data Structures** - Concurrent skip lists, B-trees, hash tables
9. **Performance & Optimization** - False sharing, NUMA, cache effects
10. **Advanced Patterns** - Work stealing, fork-join, actor model
11. **Debugging & Testing** - ThreadSanitizer, stress testing, deadlock detection
12. **Real-World Systems** - Web server, task scheduler, cache, game server

---

## Structure of Each Assignment

### 1. Multiple Choice Questions (10-15)
- Test theoretical understanding
- Cover edge cases and common pitfalls
- Include answers and explanations

### 2. Code Review Exercises (2-3)
- Identify bugs in concurrent code
- Race conditions, deadlocks, memory ordering issues
- Propose and implement fixes

### 3. Implementation from Scratch (3-4)
- Build production-ready concurrent components
- Focus on correctness and performance
- Include comprehensive test cases

### 4. Debugging Concurrent Code (2-3)
- Use tools: ThreadSanitizer, Helgrind, Valgrind
- Find and fix subtle race conditions
- Reproduce and resolve intermittent bugs

### 5. Performance Optimization (2-3)
- Benchmark different approaches
- Profile with perf/VTune
- Analyze scalability and bottlenecks

---

## Time Commitment

| Assignment | Estimated Time | Difficulty |
|-----------|----------------|-----------|
| 01 | 6-8 hours | ⭐⭐⭐ |
| 02 | 6-8 hours | ⭐⭐⭐⭐ |
| 03 | 6-8 hours | ⭐⭐⭐⭐ |
| 04 | 6-8 hours | ⭐⭐⭐⭐ |
| 05 | 8-10 hours | ⭐⭐⭐⭐⭐ |
| 06 | 8-10 hours | ⭐⭐⭐⭐⭐ |
| 07 | 6-8 hours | ⭐⭐⭐⭐ |
| 08 | 8-10 hours | ⭐⭐⭐⭐⭐ |
| 09 | 6-8 hours | ⭐⭐⭐⭐ |
| 10 | 6-8 hours | ⭐⭐⭐⭐ |
| 11 | 6-8 hours | ⭐⭐⭐⭐ |
| 12 | 10-12 hours | ⭐⭐⭐⭐⭐ |
| **Total** | **75-95 hours** | |

---

## Prerequisites

### Knowledge
- Proficient in C++17/20
- Understanding of basic threading concepts
- Experience with STL and templates
- Familiarity with build systems (CMake/Make)

### Tools Required
- **Compiler**: GCC 10+, Clang 11+, or MSVC 2019+
- **Sanitizers**: ThreadSanitizer, AddressSanitizer
- **Debuggers**: GDB, LLDB
- **Profilers**: perf (Linux), VTune, or Instruments (macOS)
- **Build**: CMake 3.15+

### Installation (Linux/macOS)
```bash
# Install compiler with sanitizers
sudo apt install g++-11 clang-12  # Ubuntu/Debian
brew install gcc llvm              # macOS

# Install tools
sudo apt install valgrind linux-tools-generic  # Linux
brew install valgrind                          # macOS

# Verify ThreadSanitizer
g++ -fsanitize=thread test.cpp -pthread -o test
```

---

## Compilation Guidelines

### Basic Compilation
```bash
# C++17 with threading support
g++ -std=c++17 -pthread -O2 -o program program.cpp

# C++20
g++ -std=c++20 -pthread -O2 -o program program.cpp
```

### With Sanitizers
```bash
# ThreadSanitizer (race detection)
g++ -std=c++17 -fsanitize=thread -g -o program program.cpp -pthread

# AddressSanitizer (memory errors)
g++ -std=c++17 -fsanitize=address -g -o program program.cpp -pthread

# UndefinedBehaviorSanitizer
g++ -std=c++17 -fsanitize=undefined -g -o program program.cpp -pthread
```

### Optimization Levels
```bash
# Debug (no optimization)
g++ -std=c++17 -g -O0 -pthread -o program program.cpp

# Release (full optimization)
g++ -std=c++17 -O3 -DNDEBUG -pthread -o program program.cpp

# With debug symbols for profiling
g++ -std=c++17 -g -O2 -pthread -o program program.cpp
```

### CMake Template
```cmake
cmake_minimum_required(VERSION 3.15)
project(ConcurrencyAssignment)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threads package
find_package(Threads REQUIRED)

# Your executable
add_executable(assignment main.cpp)
target_link_libraries(assignment PRIVATE Threads::Threads)

# Sanitizer options
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
if(ENABLE_TSAN)
    target_compile_options(assignment PRIVATE -fsanitize=thread -g)
    target_link_options(assignment PRIVATE -fsanitize=thread)
endif()

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(assignment PRIVATE -fsanitize=address -g)
    target_link_options(assignment PRIVATE -fsanitize=address)
endif()
```

---

## Submission Format

### Directory Structure
```
assignment_XX/
├── README.md                 # Overview and build instructions
├── CMakeLists.txt            # Build configuration
├── src/                      # Source files
│   ├── implementation.cpp
│   ├── implementation.h
│   └── tests.cpp
├── docs/                     # Documentation
│   ├── mcq_answers.md
│   ├── code_review.md
│   ├── design_decisions.md
│   └── performance_analysis.md
├── benchmarks/               # Performance tests
│   ├── benchmark.cpp
│   └── results.csv
└── graphs/                   # Performance graphs
    ├── throughput.png
    └── scalability.png
```

### Documentation Requirements
1. **MCQ Answers** (mcq_answers.md)
   - All answers with explanations
   - References to concepts

2. **Code Review** (code_review.md)
   - List all bugs found
   - Explain root causes
   - Provide fixes with explanations

3. **Design Decisions** (design_decisions.md)
   - Architecture choices
   - Trade-off analysis
   - Alternative approaches considered

4. **Performance Analysis** (performance_analysis.md)
   - Benchmark methodology
   - Results with graphs
   - Bottleneck analysis
   - Recommendations

---

## Evaluation Rubric

### Correctness (30-40%)
- ✅ No data races (verified with ThreadSanitizer)
- ✅ No deadlocks
- ✅ Proper synchronization
- ✅ Correct memory ordering
- ✅ All tests pass

### Performance (20-25%)
- ✅ Efficient algorithms
- ✅ Minimal lock contention
- ✅ Good scalability
- ✅ Cache-friendly design
- ✅ Meets performance targets

### Code Quality (15-20%)
- ✅ Clean, readable code
- ✅ Proper error handling
- ✅ Well-documented
- ✅ Follows C++ best practices
- ✅ RAII and modern C++

### Analysis & Understanding (15-20%)
- ✅ Thorough explanations
- ✅ Deep understanding demonstrated
- ✅ Comprehensive benchmarking
- ✅ Insightful analysis

### Testing (10-15%)
- ✅ Comprehensive test coverage
- ✅ Stress tests included
- ✅ Edge cases covered
- ✅ Reproducible tests

---

## Common Pitfalls to Avoid

### 1. Data Races
```cpp
// ❌ Bad: Data race
int counter = 0;
void increment() { counter++; }

// ✅ Good: Atomic or mutex-protected
std::atomic<int> counter{0};
void increment() { counter++; }
```

### 2. Deadlocks
```cpp
// ❌ Bad: Potential deadlock
void transfer(Account& from, Account& to) {
    std::lock_guard lock1(from.mutex);
    std::lock_guard lock2(to.mutex);
    // ...
}

// ✅ Good: Consistent lock ordering
void transfer(Account& from, Account& to) {
    std::scoped_lock lock(from.mutex, to.mutex);
    // ...
}
```

### 3. Spurious Wakeups
```cpp
// ❌ Bad: if instead of while
cv.wait(lock);
if (!ready) return;

// ✅ Good: while loop or predicate
cv.wait(lock, [] { return ready; });
```

### 4. Memory Ordering
```cpp
// ❌ Bad: Relaxed where synchronization needed
flag.store(true, std::memory_order_relaxed);

// ✅ Good: Release for synchronization
flag.store(true, std::memory_order_release);
```

---

## Recommended Learning Path

### Week 1-2: Fundamentals
- Assignment 01: Thread Management
- Assignment 02: Mutex & Locks
- Assignment 03: Deadlock Prevention

### Week 3-4: Synchronization
- Assignment 04: Condition Variables
- Assignment 05: Atomics & Memory Model

### Week 5-6: Lock-Free Programming
- Assignment 06: Lock-Free Structures
- Assignment 07: Futures & Async

### Week 7-8: Advanced Topics
- Assignment 08: Thread-Safe Structures
- Assignment 09: Performance Optimization

### Week 9-10: Integration
- Assignment 10: Concurrency Patterns
- Assignment 11: Debugging & Testing

### Week 11-12: Real-World Project
- Assignment 12: Complete System

---

## Resources

### Books
1. **C++ Concurrency in Action, 2nd Edition** by Anthony Williams
2. **The Art of Multiprocessor Programming** by Herlihy & Shavit
3. **Programming with POSIX Threads** by David Butenhof

### Online Resources
- [cppreference.com - Thread Support](https://en.cppreference.com/w/cpp/thread)
- [Jeff Preshing's Blog](https://preshing.com/)
- [Herb Sutter's Blog](https://herbsutter.com/)
- [CppCon Talks on Concurrency](https://www.youtube.com/user/CppCon)

### Papers
- [Hazard Pointers](https://www.research.ibm.com/people/m/michael/ieeetpds-2004.pdf)
- [Michael-Scott Queue](https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf)
- [C++ Memory Model](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2633.html)

### Tools Documentation
- [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)
- [Valgrind Helgrind](https://valgrind.org/docs/manual/hg-manual.html)
- [Intel VTune](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/vtune-profiler.html)

---

## FAQ

### Q: Can I use C++11/14 instead of C++17/20?
A: Most assignments work with C++11, but some features (like `std::scoped_lock`, `std::shared_mutex`) require C++17. Use the latest standard available.

### Q: How do I know if my solution is correct?
A: Run with ThreadSanitizer repeatedly. If it passes 1000+ runs without errors, you're likely correct. Also, verify with stress tests.

### Q: What if I can't reproduce a bug?
A: Add random delays, increase thread count, and run longer. Use tools like `stress` or custom stress testers.

### Q: Is lock-free always faster?
A: No! Lock-free has overhead. It's beneficial under high contention but can be slower in low-contention scenarios. Always benchmark.

### Q: Should I implement all exercises?
A: Implement at least the required ones in each section. Additional exercises are for deeper learning.

---

## Support & Questions

For questions or clarifications:
1. Review the assignment carefully
2. Check the resources section
3. Use ThreadSanitizer to verify correctness
4. Profile to understand performance

---

## License

These assignments are for educational purposes. Feel free to use and modify for learning.

---

**Good luck on your concurrency mastery journey!** 🚀
