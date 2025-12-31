# Senior C++ Developer Interview Topics (12+ Years Experience)

## Table of Contents
1. [Modern C++ Features (C++11/14/17/20/23)](#modern-cpp-features)
2. [Advanced Memory Management](#advanced-memory-management)
3. [Template Metaprogramming](#template-metaprogramming)
4. [Concurrency and Multithreading](#concurrency-and-multithreading)
5. [Design Patterns and Architecture](#design-patterns-and-architecture)
6. [Performance Optimization](#performance-optimization)
7. [STL and Algorithms](#stl-and-algorithms)
8. [Object-Oriented Design](#object-oriented-design)
9. [Build Systems and Tooling](#build-systems-and-tooling)
10. [System Programming](#system-programming)

---

## Modern C++ Features (C++11/14/17/20/23)

### C++11 Key Features
- **Move Semantics & Rvalue References**
  - Move constructors and move assignment operators
  - Perfect forwarding with `std::forward`
  - `std::move` semantics
- **Smart Pointers**
  - `std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`
  - Custom deleters
- **Lambda Expressions**
  - Capture lists (by value, by reference, generalized capture)
  - Mutable lambdas
  - Generic lambdas (C++14)
- **Variadic Templates**
- **nullptr** keyword
- **auto** and **decltype**
- **Range-based for loops**
- **Uniform initialization** (brace initialization)
- **constexpr** functions
- **std::thread** and basic concurrency primitives

### C++14 Enhancements
- **Generic lambdas** (auto parameters)
- **Return type deduction** for functions
- **Binary literals** and digit separators
- **std::make_unique**
- **Variable templates**

### C++17 Features
- **Structured Bindings**
- **if/switch with initializers**
- **std::optional**, **std::variant**, **std::any**
- **std::string_view**
- **Fold expressions**
- **inline variables**
- **Parallel STL algorithms**
- **Filesystem library** (std::filesystem)
- **std::byte**

### C++20 Major Features
- **Concepts** and constraints
- **Ranges library**
- **Coroutines** (co_await, co_yield, co_return)
- **Modules**
- **Three-way comparison operator** (spaceship operator <=>)
- **std::span**
- **consteval** and **constinit**
- **std::format**
- **Calendar and timezone library**

### C++23 Updates
- **std::expected**
- **std::flat_map** and **std::flat_set**
- **std::print**
- **Multidimensional subscript operator**
- **Deducing this**

---

## Advanced Memory Management

### Core Concepts
- **RAII** (Resource Acquisition Is Initialization)
- **Memory allocation strategies**
  - Stack vs Heap allocation
  - Custom allocators (std::allocator)
  - Pool allocators, arena allocators
- **Memory alignment** and padding
- **Cache-friendly data structures**
- **Memory pools** and object pools
- **Placement new**
- **Memory leaks detection** (Valgrind, AddressSanitizer)
- **Copy elision** and **RVO/NRVO**
- **Small String Optimization (SSO)**
- **std::pmr** (Polymorphic Memory Resources)

### Common Interview Questions
- Difference between `delete` and `delete[]`
- When to use smart pointers vs raw pointers
- Memory fragmentation issues
- Implementing custom memory allocators
- Memory ordering and cache coherency

---

## Template Metaprogramming

### Essential Topics
- **Template specialization** (full and partial)
- **SFINAE** (Substitution Failure Is Not An Error)
- **Type traits** (std::is_*, std::enable_if)
- **Template template parameters**
- **Variadic templates** and parameter packs
- **Fold expressions**
- **constexpr programming**
- **if constexpr**
- **Concepts** (C++20)
- **CRTP** (Curiously Recurring Template Pattern)
- **Tag dispatching**
- **Policy-based design**

### Advanced Techniques
- **Expression templates**
- **Metafunctions**
- **Template recursion**
- **Type erasure**
- **std::declval** usage
- **Perfect forwarding** patterns

---

## Concurrency and Multithreading

### Threading Fundamentals
- **std::thread**, **std::jthread** (C++20)
- **Thread lifecycle** management
- **Thread-local storage**
- **Race conditions** and **data races**
- **Deadlocks**, **livelocks**, and **priority inversion**

### Synchronization Primitives
- **std::mutex**, **std::recursive_mutex**, **std::timed_mutex**
- **std::shared_mutex** (reader-writer locks)
- **std::lock_guard**, **std::unique_lock**, **std::scoped_lock**
- **std::condition_variable**
- **std::atomic** operations
- **Memory ordering** (acquire, release, seq_cst, relaxed)
- **std::atomic_flag** and lock-free programming
- **Semaphores** (C++20)
- **Latches** and **barriers** (C++20)

### Async Programming
- **std::async** and **std::future**
- **std::promise**
- **std::packaged_task**
- **Thread pools** implementation
- **Producer-consumer patterns**
- **Work stealing algorithms**

### Advanced Concurrency
- **Lock-free data structures**
- **ABA problem**
- **Memory fences**
- **Hazard pointers**
- **Coroutines** for async I/O (C++20)

---

## Design Patterns and Architecture

### Creational Patterns
- **Singleton** (thread-safe variants)
- **Factory Method** and **Abstract Factory**
- **Builder Pattern**
- **Prototype Pattern**
- **Object Pool**

### Structural Patterns
- **Adapter** and **Facade**
- **Proxy** and **Decorator**
- **Bridge Pattern**
- **Composite Pattern**
- **Flyweight Pattern**
- **PIMPL** (Pointer to Implementation)

### Behavioral Patterns
- **Observer Pattern**
- **Strategy Pattern**
- **Command Pattern**
- **State Pattern**
- **Visitor Pattern**
- **Template Method Pattern**
- **Chain of Responsibility**

### Architectural Patterns
- **MVC**, **MVP**, **MVVM**
- **Dependency Injection**
- **Event-driven architecture**
- **Plugin architecture**
- **Microservices patterns**

### Modern C++ Idioms
- **RAII**
- **CRTP**
- **Type Erasure**
- **Expression Templates**
- **Policy-based Design**
- **Mixin classes**

---

## Performance Optimization

### Profiling and Analysis
- **CPU profiling** (gprof, perf, VTune)
- **Memory profiling** (Valgrind, Heaptrack)
- **Hotspot identification**
- **Cache analysis** (cachegrind)
- **Branch prediction** optimization

### Optimization Techniques
- **Inlining** strategies
- **Loop optimization** (unrolling, fusion, fission)
- **Branch prediction** and **likely/unlikely** hints
- **Data-oriented design**
- **SoA vs AoS** (Structure of Arrays vs Array of Structures)
- **SIMD** programming (SSE, AVX)
- **Cache optimization** (prefetching, false sharing)
- **Compiler optimization flags** (-O2, -O3, LTO)
- **PGO** (Profile-Guided Optimization)

### Algorithm Complexity
- **Time complexity** analysis
- **Space complexity** considerations
- **Amortized analysis**
- **Trade-offs** between time and space

### Modern Performance Features
- **constexpr** for compile-time computation
- **std::string_view** for string operations
- **Move semantics** to avoid copies
- **Perfect forwarding**
- **Small buffer optimization**

---

## STL and Algorithms

### Containers
- **Sequence containers**: vector, deque, list, array, forward_list
- **Associative containers**: set, map, multiset, multimap
- **Unordered containers**: unordered_set, unordered_map
- **Container adapters**: stack, queue, priority_queue
- **std::span** (C++20)
- **Container performance characteristics** (Big-O)
- **Iterator invalidation rules**

### Algorithms
- **Sorting**: sort, stable_sort, partial_sort, nth_element
- **Searching**: find, binary_search, lower_bound, upper_bound
- **Modifying**: transform, copy, move, swap
- **Numeric**: accumulate, inner_product, adjacent_difference
- **Parallel algorithms** (C++17 execution policies)
- **Ranges algorithms** (C++20)

### Iterators
- **Iterator categories** (input, output, forward, bidirectional, random access)
- **Iterator adapters** (reverse_iterator, move_iterator)
- **Custom iterator implementation**

### Advanced STL
- **Custom allocators**
- **Custom comparators** and hash functions
- **std::function** and **std::bind**
- **std::ref** and **std::cref**
- **Type traits** for SFINAE

---

## Object-Oriented Design

### Core OOP Principles
- **Encapsulation**, **Abstraction**, **Inheritance**, **Polymorphism**
- **SOLID principles**
  - Single Responsibility
  - Open/Closed
  - Liskov Substitution
  - Interface Segregation
  - Dependency Inversion

### Inheritance and Polymorphism
- **Virtual functions** and **vtables**
- **Pure virtual functions** (abstract classes)
- **Virtual destructors**
- **Multiple inheritance** (diamond problem)
- **Virtual inheritance**
- **Covariant return types**
- **Override** and **final** keywords

### Class Design
- **Rule of Three/Five/Zero**
- **Copy constructors** and **copy assignment**
- **Move constructors** and **move assignment**
- **Deleted functions** (= delete)
- **Defaulted functions** (= default)
- **Explicit constructors**
- **Friend functions** and classes
- **Nested classes**

### Interface Design
- **Designing for testability**
- **Dependency injection**
- **Contract-based programming**
- **Exception safety guarantees**

---

## Build Systems and Tooling

### Build Systems
- **CMake** (modern CMake practices)
- **Make** and **Ninja**
- **Bazel**
- **Meson**
- **Build configuration management**

### Compilation Process
- **Preprocessing**, **compilation**, **assembly**, **linking**
- **Static vs dynamic linking**
- **Symbol visibility**
- **Name mangling**
- **Link-Time Optimization (LTO)**
- **Precompiled headers** (PCH)
- **Unity builds**

### Package Management
- **Conan**
- **vcpkg**
- **Hunter**

### Testing Frameworks
- **Google Test** (gtest)
- **Catch2**
- **Boost.Test**
- **Unit testing** best practices
- **Mock frameworks** (gmock)
- **Code coverage** tools

### Static Analysis
- **Clang-Tidy**
- **Cppcheck**
- **Clang Static Analyzer**
- **PVS-Studio**
- **Static Application Security Testing (SAST)**

### Debugging Tools
- **GDB** and **LLDB**
- **Valgrind** (memcheck, helgrind, cachegrind)
- **AddressSanitizer**, **ThreadSanitizer**, **UndefinedBehaviorSanitizer**
- **Core dump analysis**

---

## System Programming

### Operating System Concepts
- **Process** vs **Thread**
- **Inter-Process Communication (IPC)**
  - Pipes, named pipes (FIFO)
  - Message queues
  - Shared memory
  - Sockets
  - Signals
- **System calls**
- **Virtual memory**
- **Memory-mapped files**

### Network Programming
- **Socket programming** (TCP/UDP)
- **Asynchronous I/O** (select, poll, epoll, kqueue)
- **Event-driven programming**
- **Boost.Asio**
- **Zero-copy techniques**

### File I/O
- **Standard I/O** (stdio.h)
- **C++ I/O streams** (iostream, fstream)
- **Binary I/O**
- **Memory-mapped I/O**
- **std::filesystem** (C++17)

### Low-Level Programming
- **Bit manipulation**
- **Endianness** handling
- **Alignment** requirements
- **Volatile** keyword
- **Inline assembly**
- **Compiler intrinsics**

---

## Additional Important Topics

### Exception Handling
- **RAII** and exception safety
- **noexcept** specification
- **Exception guarantees** (basic, strong, no-throw)
- **Stack unwinding**
- **Custom exception hierarchies**
- **std::exception_ptr**

### Standard Library Utilities
- **std::tuple** and structured bindings
- **std::pair**
- **std::optional**, **std::variant**, **std::any**
- **std::chrono** for time handling
- **std::regex** for regular expressions
- **std::random** for random number generation

### Undefined Behavior
- Common sources of UB
- Compiler assumptions
- Sanitizers for UB detection

### ABI and Compatibility
- **Application Binary Interface**
- **ABI stability**
- **Symbol versioning**
- **C++ name mangling**

### Cross-Platform Development
- **Platform-specific code** handling
- **Conditional compilation**
- **Portable types** (stdint.h)
- **Endianness** portability

---

## Interview Preparation Tips

### For 12+ Years Experience Level
1. **System Design**: Be prepared for architectural discussions
2. **Leadership**: Discuss code reviews, mentoring, and technical decisions
3. **Trade-offs**: Explain pros/cons of different approaches
4. **Real-world Experience**: Share specific project challenges and solutions
5. **Best Practices**: Demonstrate knowledge of industry standards
6. **Performance**: Deep understanding of optimization techniques
7. **Debugging**: Complex problem-solving scenarios
8. **Technical Depth**: Advanced C++ features and internals
9. **Code Quality**: Testing, documentation, maintainability
10. **Continuous Learning**: Knowledge of modern C++ standards

### Common Senior-Level Questions
- How would you design a high-performance, multi-threaded system?
- Explain a challenging technical problem you solved
- How do you ensure code quality in a large codebase?
- What's your approach to performance optimization?
- How do you handle technical debt?
- Describe your experience with large-scale refactoring
- How do you mentor junior developers?
- What's your philosophy on code reviews?

---

## Resources for Further Study

### Books
- "Effective Modern C++" by Scott Meyers
- "C++ Concurrency in Action" by Anthony Williams
- "The C++ Programming Language" by Bjarne Stroustrup
- "C++ Templates: The Complete Guide" by Vandevoorde, Josuttis, and Gregor
- "Design Patterns" by Gang of Four

### Online Resources
- CppReference.com
- ISO C++ Standard
- CppCon talks on YouTube
- C++ Weekly by Jason Turner

---

*Last Updated: October 2025*
