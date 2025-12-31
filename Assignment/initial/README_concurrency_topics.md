# C++ Concurrency Curriculum - Foundational Series

## Overview

This folder contains a comprehensive **10-assignment foundational series** on C++ concurrency, designed for **intermediate developers** (3-5 years experience). These assignments serve as prerequisites for the advanced concurrency series located in `/concurrency/`.

**Total Estimated Time:** 50-60 hours  
**Target Audience:** Intermediate C++ developers  
**Level:** Foundational → Intermediate  
**Format:** Hands-on assignments with MCQs, code review, implementation, debugging, and performance analysis

---

## Curriculum Structure

Each assignment follows a consistent 5-part format:
1. **Multiple Choice Questions (8-12 MCQs)** - Test conceptual understanding
2. **Code Review Exercises (3-4 exercises)** - Identify and fix bugs
3. **Implementation from Scratch (3-4 exercises)** - Build thread-safe components
4. **Debugging Exercises (2 exercises)** - Real-world problem solving
5. **Performance Analysis (2 exercises)** - Benchmark and optimize

---

## Assignment Series

### **Assignment 01: Thread Basics** (4-5 hours)
**File:** `01_thread_basics.md`  
**Topics:**
- Creating and launching threads
- Thread function signatures (functions, lambdas, functors)
- Passing arguments to threads
- Joining vs detaching threads
- Hardware concurrency

**Key Skills:**
- Basic thread creation and management
- Understanding joinable state
- Exception safety basics

---

### **Assignment 02: std::thread Deep Dive** (4-5 hours)
**File:** `02_std_thread.md`  
**Topics:**
- std::thread API (get_id, joinable, native_handle)
- Move semantics with threads
- Argument passing (by value, by reference with std::ref)
- Exception handling in threads
- Thread manager RAII patterns

**Key Skills:**
- Advanced thread argument handling
- Thread ownership and transfer
- Simple thread pool implementation

---

### **Assignment 03: Thread Lifecycle Management** (5-6 hours)
**File:** `03_thread_lifecycle_management.md`  
**Topics:**
- Thread states and lifecycle
- Join vs detach trade-offs
- RAII wrappers (ScopedThread, JoiningThread)
- Exception-safe thread management
- Thread pool lifecycle

**Key Skills:**
- RAII for automatic resource management
- Exception-safe threading patterns
- Understanding detach dangers

---

### **Assignment 04: Mutexes and Locks** (5-6 hours)
**File:** `04_mutexes_locks.md`  
**Topics:**
- Mutual exclusion and critical sections
- std::mutex, recursive_mutex, timed_mutex
- lock_guard, unique_lock, scoped_lock
- Lock granularity (coarse vs fine-grained)
- Reader-writer locks (shared_mutex)

**Key Skills:**
- Proper lock selection
- Minimizing critical sections
- Thread-safe class design
- Lock contention analysis

---

### **Assignment 05: Deadlock Avoidance** (5-6 hours)
**File:** `05_deadlock_avoidance.md`  
**Topics:**
- Four deadlock conditions
- Lock ordering strategies
- std::lock and std::scoped_lock
- Timeout-based locking
- Dining philosophers problem

**Key Skills:**
- Deadlock prevention and detection
- Safe multi-mutex locking
- Resource acquisition strategies

---

### **Assignment 06: Condition Variables** (5-6 hours)
**File:** `06_condition_variables.md`  
**Topics:**
- condition_variable for thread coordination
- Spurious wakeups handling
- Wait predicates
- Producer-consumer patterns
- notify_one vs notify_all

**Key Skills:**
- Blocking synchronization
- Proper wait condition checking
- Barrier and latch implementations
- Thread pool with task queue

---

### **Assignment 07: Atomic Operations** (5-6 hours)
**File:** `07_atomic_operations.md`  
**Topics:**
- std::atomic types
- Atomic operations (load, store, exchange, fetch_add)
- compare_exchange for lock-free algorithms
- atomic_flag for spin locks
- Introduction to memory ordering

**Key Skills:**
- Lock-free programming basics
- Atomic vs mutex performance
- Simple lock-free structures

---

### **Assignment 08: Memory Model & Ordering** (6 hours)
**File:** `08_memory_model_ordering.md`  
**Topics:**
- C++ memory model fundamentals
- memory_order: relaxed, acquire, release, seq_cst
- Happens-before relationships
- Acquire-release synchronization
- Platform differences (x86 vs ARM)

**Key Skills:**
- Understanding visibility and ordering
- Choosing appropriate memory ordering
- Avoiding unnecessary synchronization overhead

---

### **Assignment 09: Futures and Promises** (5-6 hours)
**File:** `09_futures_promises.md`  
**Topics:**
- std::future and std::promise
- std::async and launch policies
- std::packaged_task
- Exception propagation across threads
- Task-based parallelism

**Key Skills:**
- Asynchronous programming
- Async vs manual threads
- Future-based thread pools
- Timeout handling

---

### **Assignment 10: Thread-Safe Data Structures** (6-7 hours)
**File:** `10_thread_safe_data_structures.md`  
**Topics:**
- Thread-safe queue (bounded and unbounded)
- Thread-safe stack
- Thread-safe LRU cache
- Object pool
- Lock-free vs lock-based designs

**Key Skills:**
- Designing thread-safe containers
- Handling blocking operations
- Fine-grained locking strategies
- Read-write optimization

---

## Learning Path

### Recommended Progression:
```
01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 → 10
```

### Skill Development:
- **Assignments 01-03:** Thread fundamentals and lifecycle
- **Assignments 04-06:** Synchronization primitives
- **Assignments 07-08:** Lock-free programming and memory model
- **Assignments 09-10:** Async programming and advanced patterns

---

## Prerequisites

- **C++ Knowledge:** Solid understanding of C++11/14/17
- **Experience:** 3-5 years C++ development
- **Topics:** Move semantics, RAII, templates, lambdas
- **Tools:** GCC/Clang compiler with C++17 support, ThreadSanitizer (optional)

---

## Tools & Environment

### Required:
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+ or equivalent build system

### Recommended:
- **ThreadSanitizer:** Detect data races (`-fsanitize=thread`)
- **Valgrind/Helgrind:** Thread debugging
- **perf/VTune:** Performance profiling
- **Address Sanitizer:** Memory error detection

---

## Evaluation Criteria

Each assignment is evaluated on:
- **Correctness (30-40%):** Code works as specified, thread-safe
- **Understanding (20-30%):** MCQ answers, explanations, analysis
- **Code Quality (15-20%):** Clean, readable, maintainable code
- **Performance (15-20%):** Insightful benchmarks and optimizations

---

## Next Steps

After completing this foundational series:

1. **Advanced Series:** Proceed to `/concurrency/` folder for 12 advanced assignments
   - Thread pools and work stealing
   - Lock-free data structures (ABA problem, hazard pointers)
   - Advanced memory ordering patterns
   - Concurrent algorithms and patterns
   - Production-ready systems

2. **Real Projects:** Apply skills to:
   - Web servers
   - Parallel algorithms
   - Game engines
   - High-performance computing

---

## Key Concepts Summary

### Foundational Concepts:
✅ Thread creation, joining, detaching  
✅ RAII for thread management  
✅ Mutexes and locks (lock_guard, unique_lock, scoped_lock)  
✅ Deadlock prevention  
✅ Condition variables and blocking operations  
✅ Atomic operations and memory ordering basics  
✅ Futures and promises  
✅ Thread-safe container design  

### Skills Gained:
- Write thread-safe code
- Choose appropriate synchronization primitives
- Debug concurrency issues
- Optimize concurrent performance
- Design scalable multi-threaded systems

---

## Common Pitfalls to Avoid

❌ Forgetting to join/detach threads  
❌ Data races on shared variables  
❌ Deadlock from inconsistent lock ordering  
❌ Not handling spurious wakeups  
❌ Using relaxed memory ordering inappropriately  
❌ Calling future.get() multiple times  
❌ Returning references to protected data  
❌ Poor lock granularity  

---

## Resources

### Books:
- **C++ Concurrency in Action (2nd Edition)** by Anthony Williams
- **The Art of Multiprocessor Programming** by Maurice Herlihy
- **Effective Modern C++** by Scott Meyers (Items 37-40)

### Online:
- [cppreference - Thread Support Library](https://en.cppreference.com/w/cpp/thread)
- [Preshing on Programming](https://preshing.com/) - Memory ordering articles
- [Herb Sutter's Blog](https://herbsutter.com/) - Concurrency patterns

### Videos:
- CppCon talks on concurrency (YouTube)
- Fedor Pikus - Concurrency in C++ talks
- Anthony Williams - C++ Concurrency tutorials

---

## Contributing

Found an issue or have suggestions? Please:
1. Test your solution thoroughly
2. Run ThreadSanitizer to verify thread safety
3. Document any assumptions
4. Include performance measurements

---

## License & Usage

These assignments are designed for educational purposes. Feel free to:
- Use for personal learning
- Share with study groups
- Adapt for teaching (with attribution)

---

## FAQ

**Q: Do I need to complete all assignments?**  
A: Yes, they build on each other progressively.

**Q: Can I skip the advanced series?**  
A: This foundational series prepares you for production code. The advanced series covers expert topics.

**Q: How long does the full curriculum take?**  
A: Foundational (50-60 hours) + Advanced (75-95 hours) = 125-155 hours total.

**Q: What if I get stuck?**  
A: Review the resources, use ThreadSanitizer for race detection, and study the code review examples carefully.

---

## Completion Certificate

Upon completing all 10 assignments with passing grades:
- ✅ Solid foundation in C++ concurrency
- ✅ Ready for advanced concurrency topics
- ✅ Prepared for production multi-threaded development
- ✅ Equipped to debug complex race conditions
- ✅ Understanding of performance implications

---

**Good luck with your concurrency journey!**

For questions or support, refer to the resources section or consult C++ community forums.
