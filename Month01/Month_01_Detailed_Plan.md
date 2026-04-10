# Month 1: Modern C++ Foundations & Advanced Features
**Duration:** Week 1-4  
**Focus:** Project-Intensive Deep Dive into Modern C++ with Practical Implementation

---

## Overview
This month focuses on mastering Modern C++ fundamentals through intensive hands-on projects. You'll build real-world systems while learning advanced features, setting a strong foundation for upcoming concurrency work.

---

## Week 1: Move Semantics, Perfect Forwarding & Smart Pointers

### Learning Objectives
- Master rvalue references and move semantics internals
- Understand perfect forwarding and universal references
- Deep dive into smart pointer implementations
- Learn custom deleters and allocator integration

### Daily Breakdown

#### Day 1-2: Move Semantics Deep Dive
**Theory (2 hours):**
- Value categories (lvalue, prvalue, xvalue, glvalue, rvalue)
- Move constructor and move assignment operator rules
- The Rule of Five/Zero
- std::move and std::forward mechanics
- Return value optimization (RVO) and Named RVO (NRVO)

**Project Task:**
Build a **Custom String Class** with move semantics
- Implement full move semantics
- Add SSO (Small String Optimization)
- Benchmark against std::string
- Profile memory allocations

**Deliverables:**
- `custom_string.hpp` and `custom_string.cpp`
- Benchmark comparison report
- Unit tests with GoogleTest

#### Day 3-4: Perfect Forwarding & Universal References
**Theory (2 hours):**
- Template argument deduction rules
- Reference collapsing rules
- std::forward implementation details
- Perfect forwarding patterns
- Forwarding references vs rvalue references

**Project Task:**
Build a **Generic Factory System**
- Template factory with perfect forwarding
- Variadic template parameter packs
- Type-safe object creation
- Factory registration system

**Deliverables:**
- `factory.hpp` with complete implementation
- Example usage with multiple types
- Documentation on forwarding patterns used

#### Day 5-7: Smart Pointers Mastery
**Theory (2 hours):**
- unique_ptr internals and custom deleters
- shared_ptr reference counting implementation
- weak_ptr and circular reference breaking
- make_unique vs make_shared optimization
- Custom allocators with smart pointers

**Project Task:**
Build a **Memory Pool Manager** with smart pointers
- Custom memory pool allocator
- Pool-aware smart pointers
- Memory leak detection system
- Performance comparison with standard allocation

**Deliverables:**
- `memory_pool.hpp` - Pool allocator implementation
- `pool_ptr.hpp` - Custom smart pointer using pool
- Benchmarks: allocation speed comparison
- Memory usage analysis report

**Weekly Mini-Project:**
Create a **Resource Manager System** (like game engine resource loading)
- Load/cache textures, sounds, models (mock data)
- Use smart pointers for automatic cleanup
- Implement weak pointer observer pattern
- Add move semantics for resource transfer

---

## Week 2: Template Metaprogramming & Type Traits

### Learning Objectives
- Master template specialization and SFINAE
- Understand type traits and std::enable_if
- Learn compile-time computations
- Explore constexpr and consteval functions

### Daily Breakdown

#### Day 8-9: Template Fundamentals & Specialization
**Theory (2 hours):**
- Template instantiation process
- Full and partial specialization
- Template template parameters
- Variadic templates deep dive
- SFINAE (Substitution Failure Is Not An Error)

**Project Task:**
Build a **Compile-Time Expression Template Library**
- Matrix operations with expression templates
- Lazy evaluation system
- Zero-cost abstractions
- Compile-time dimension checking

**Deliverables:**
- `expr_template.hpp` - Expression template library
- Matrix operations: add, multiply, transpose
- Benchmark: expression templates vs naive loops
- Blog draft: "Expression Templates Explained"

#### Day 10-11: Type Traits & SFINAE
**Theory (2 hours):**
- Standard type traits (is_integral, is_pointer, etc.)
- Creating custom type traits
- std::enable_if, std::conditional
- Tag dispatching techniques
- SFINAE-friendly detection idioms

**Project Task:**
Build a **Generic Serialization Framework**
- Automatic serialization for POD types
- Custom serialization for complex types
- SFINAE-based type detection
- Binary and text formats

**Deliverables:**
- `serializer.hpp` - Generic serialization system
- Support for: primitives, containers, custom classes
- Deserialization with error handling
- Example: serialize/deserialize game state

#### Day 12-14: Constexpr Programming
**Theory (2 hours):**
- constexpr vs consteval vs const
- Compile-time computations
- constexpr algorithms and containers (C++20)
- Immediate functions
- Compile-time string manipulation

**Project Task:**
Build a **Compile-Time Parser & Validator**
- Compile-time JSON/config parser
- Constexpr string algorithms
- Compile-time format string validation
- Static configuration system

**Deliverables:**
- `constexpr_parser.hpp` - Compile-time parser
- Compile-time validation examples
- Performance comparison: runtime vs compile-time
- Blog draft: "Constexpr: Moving Logic to Compile Time"

**Weekly Mini-Project:**
Create a **Type-Safe Units Library** (like `std::chrono`)
- Compile-time unit conversions (meters, feet, kilograms)
- Dimensional analysis at compile time
- Zero runtime overhead
- Prevent unit mismatches (can't add meters to seconds)

---

## Week 3: Concurrency Fundamentals & Thread Management

### Learning Objectives
- Master std::thread and thread lifecycle
- Understand mutex types and locking strategies
- Learn RAII locking patterns
- Explore thread-local storage

### Daily Breakdown

#### Day 15-16: Thread Basics & std::thread
**Theory (2 hours):**
- Thread creation and lifecycle
- Thread joinability and detachment
- Passing arguments to threads
- Thread IDs and hardware_concurrency
- Exception safety in threads

**Project Task:**
Build a **Thread Pool Executor** (Basic Version)
- Fixed-size thread pool
- Task queue with thread-safe operations
- Worker threads with proper lifecycle
- Graceful shutdown mechanism

**Deliverables:**
- `thread_pool.hpp` - Basic thread pool implementation
- Task submission interface
- Unit tests for edge cases
- Performance benchmarks with different pool sizes

#### Day 17-18: Mutexes & Locking Strategies
**Theory (2 hours):**
- std::mutex, recursive_mutex, timed_mutex
- std::lock_guard, std::unique_lock, std::scoped_lock
- std::shared_mutex for reader-writer locks
- Lock ordering and deadlock prevention
- Lock-free vs lock-based approaches

**Project Task:**
Build a **Thread-Safe Logger System**
- Multiple log levels (DEBUG, INFO, ERROR)
- Asynchronous logging with background thread
- File and console outputs
- Configurable format strings
- Proper mutex usage for thread safety

**Deliverables:**
- `logger.hpp` and `logger.cpp`
- Async logging with buffering
- Benchmarks: async vs sync logging
- Example integration in multi-threaded app

#### Day 19-21: RAII Locks & Advanced Patterns
**Theory (2 hours):**
- RAII idioms for resource management
- std::lock for multiple mutexes
- std::call_once and std::once_flag
- Thread-local storage (thread_local keyword)
- Lock hierarchies and custom lock types

**Project Task:**
Build a **Thread-Safe Object Pool**
- Reusable object allocation
- Lock-free when possible, locks when necessary
- Per-thread caches to reduce contention
- Statistics tracking (allocations, reuses)

**Deliverables:**
- `object_pool.hpp` - Thread-safe object pool
- Benchmarks vs standard allocation
- Contention analysis with multiple threads
- Example: connection pool for network sockets

**Weekly Mini-Project:**
Create a **Concurrent Data Processing Pipeline**
- Multi-stage pipeline (read → process → write)
- Each stage runs in separate thread(s)
- Thread-safe queues between stages
- Backpressure handling
- Performance metrics (throughput, latency)

---

## Week 4: System Programming & Integration Project

### Learning Objectives
- Understand memory alignment and padding
- Learn cache-friendly data structure design
- Master low-level I/O operations
- Integrate all Month 1 concepts

### Daily Breakdown

#### Day 22-23: Memory Layout & Alignment
**Theory (2 hours):**
- Memory alignment requirements
- alignas and alignof
- Padding and structure packing
- Cache line awareness (false sharing)
- Memory layout tools and inspection

**Project Task:**
Build a **Cache-Optimized Data Structures Library**
- Cache-friendly vector (aligned, prefetch-aware)
- Flat map for better cache locality
- AoS vs SoA (Array of Structures vs Structure of Arrays)
- Benchmarks showing cache impact

**Deliverables:**
- `cache_friendly.hpp` - Optimized containers
- Benchmark: cache misses comparison
- Blog draft: "Cache-Friendly Data Structures in C++"
- Memory layout diagrams

#### Day 24-25: System Calls & Low-Level I/O
**Theory (2 hours):**
- File descriptors and POSIX I/O
- Memory-mapped files (mmap)
- Direct I/O and unbuffered operations
- I/O multiplexing basics (select/poll)
- Platform-specific optimizations

**Project Task:**
Build a **High-Performance File Reader**
- Memory-mapped file reading
- Sequential access optimization
- Parallel file processing
- Compare: mmap vs read() vs ifstream

**Deliverables:**
- `fast_file_reader.hpp`
- Benchmarks across different file sizes
- Example: log file analyzer using parallel processing
- Platform-specific optimizations (macOS/Linux)

#### Day 26-28: Integration Project - Build a Complete System

**Major Project: High-Performance Task Scheduler with Persistent Storage**

Combine all Month 1 concepts into one cohesive system:

**Requirements:**
1. **Task Scheduling System**
   - Accept tasks with priorities and dependencies
   - Thread pool for parallel execution
   - Custom allocator for task objects
   - Move semantics for task transfer

2. **Template-Based Task System**
   - Generic task wrapper (any callable)
   - Perfect forwarding for task arguments
   - Type-safe task results with futures
   - SFINAE-based task trait detection

3. **Persistent Storage**
   - Serialize tasks to disk
   - Memory-mapped file for task queue
   - Crash recovery capability
   - Custom binary format

4. **Thread-Safe Operations**
   - Multiple producer, multiple consumer
   - Lock-free where possible
   - Proper RAII lock management
   - Statistics and monitoring

5. **Performance & System Integration**
   - Cache-friendly task storage
   - Zero-copy task passing where possible
   - Low-level I/O optimization
   - Comprehensive benchmarking

**Deliverables:**
- Complete source code with CMake build system
- Comprehensive documentation
- Unit tests (GoogleTest) with >80% coverage
- Performance benchmarks and analysis
- Example applications using the scheduler
- README with architecture diagrams

**Testing Scenarios:**
- 10,000+ concurrent tasks
- Task dependencies (DAG execution)
- Crash and recovery
- Performance under load
- Memory leak detection (Valgrind/ASan)

---

## Open Source Exploration & Contribution

### Week 1-2: Study Phase
**Projects to Study:**
- **LLVM/Clang**: Build system structure (CMake)
- Study template usage in LLVM ADT (ArrayRef, StringRef)
- Understand LLVM's memory management patterns

**Tasks:**
- Clone and build LLVM locally
- Read documentation on contributing
- Identify "good first issue" tags

### Week 3-4: First Contribution
**Target:** Documentation or small bug fix

**Activities:**
- Set up development environment
- Join community channels (Discord/Mailing list)
- Submit first PR (documentation or trivial fix)
- Code review participation

**Alternative Projects:**
- **fmt library**: Modern formatting library
- **spdlog**: Fast logging library
- Study their template techniques and move semantics usage

---

## Blog Posts to Write

### Blog Post 1: "Move Semantics: Beyond the Basics"
**Outline:**
- Introduction: Why move semantics matter
- Deep dive: Value categories explained
- Common pitfalls and misconceptions
- std::move vs std::forward
- Performance analysis with benchmarks
- Real-world examples from your string class
- Best practices and guidelines

**Target:** 2000-3000 words, publish on Medium/dev.to

### Blog Post 2: "Template Metaprogramming Patterns in Modern C++"
**Outline:**
- Evolution of template programming
- Expression templates walkthrough
- Type traits and SFINAE techniques
- Constexpr programming
- Real examples from your serialization framework
- Performance implications
- When (not) to use templates

**Target:** 2500-3500 words, include code examples

---

## Learning Resources

### Books (Reference)
- "C++ Move Semantics - The Complete Guide" by Nicolai Josuttis
- "C++ Templates: The Complete Guide" by Vandevoorde & Josuttis
- "C++ Concurrency in Action" by Anthony Williams (Ch 1-4)

### Online Resources
- CppReference.com for standard library details
- Compiler Explorer (godbolt.org) for assembly analysis
- Quick-bench.com for micro-benchmarking
- GitHub for open source code reading

### Tools to Master
- **Build:** CMake, Ninja
- **Testing:** GoogleTest, Catch2
- **Profiling:** perf, Instruments (macOS), Valgrind
- **Sanitizers:** AddressSanitizer, ThreadSanitizer
- **Debugging:** lldb, gdb

---

## Week-by-Week Checklist

### Week 1 Deliverables
- [ ] Custom string class with move semantics
- [ ] Generic factory system
- [ ] Memory pool manager with custom smart pointers
- [ ] Resource manager mini-project
- [ ] Unit tests for all components

### Week 2 Deliverables
- [ ] Expression template library
- [ ] Generic serialization framework
- [ ] Compile-time parser
- [ ] Type-safe units library
- [ ] Blog draft 1: Move Semantics

### Week 3 Deliverables
- [ ] Thread pool executor
- [ ] Thread-safe logger system
- [ ] Thread-safe object pool
- [ ] Concurrent data pipeline
- [ ] LLVM build and setup complete

### Week 4 Deliverables
- [ ] Cache-optimized data structures
- [ ] High-performance file reader
- [ ] **Major Integration Project** (Task Scheduler)
- [ ] Blog draft 2: Template Metaprogramming
- [ ] First open source contribution/PR

---

## Success Metrics

### Technical Skills
- ✅ Can implement move semantics correctly in any class
- ✅ Comfortable with template metaprogramming
- ✅ Understand thread lifecycle and basic synchronization
- ✅ Can design cache-friendly data structures
- ✅ Proficient with modern C++ build tools

### Projects Completed
- ✅ 5+ mini-projects
- ✅ 1 major integration project
- ✅ All projects with tests and benchmarks

### Community Engagement
- ✅ 1 open source PR submitted
- ✅ 2 blog posts drafted/published
- ✅ Active participation in C++ communities

---

## Daily Schedule Template

**Morning (2 hours):**
- 30 min: Read theory/documentation
- 90 min: Hands-on coding project tasks

**Afternoon (1.5 hours):**
- 60 min: Continue project implementation
- 30 min: Write tests and documentation

**Evening (1 hour):**
- 30 min: Open source exploration
- 30 min: Blog writing or community engagement

**Total:** 4-5 hours daily (adjustable based on your schedule)

---

## Tips for Success

1. **Focus on Projects**: Theory is important, but building solidifies knowledge
2. **Benchmark Everything**: Learn to measure performance, not guess
3. **Test Thoroughly**: Practice writing tests as you code
4. **Document as You Go**: Write README files and comments
5. **Share Your Work**: Post projects on GitHub, share progress
6. **Engage Community**: Ask questions, review code, help others
7. **Iterate**: If something doesn't work, refactor and improve

---

## End of Month 1 Goals

By the end of Month 1, you should have:
- ✅ Deep understanding of modern C++ core features
- ✅ Portfolio of 6+ working projects on GitHub
- ✅ Hands-on experience with concurrency basics
- ✅ First open source contribution
- ✅ 2 technical blog posts
- ✅ Strong foundation for Month 2's advanced concurrency

**Next Month Preview:** Lock-free programming, atomics, memory ordering, and building production-grade concurrent systems.
