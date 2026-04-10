# Modern C++ Learning Project

## Overview
A structured project-based approach to mastering modern C++ features (C++11/14/17/20/23).

## Project Structure
```
project/
├── 01_smart_pointers/
├── 02_move_semantics/
├── 03_lambdas/
├── 04_templates_concepts/
├── 05_ranges_views/
├── 06_coroutines/
├── 07_modules/
├── 08_concurrency/
├── 09_final_project/
└── common/
    ├── CMakeLists.txt
    └── utils.hpp
```

## Learning Path

### Phase 1: Foundation (C++11/14)

#### Project 1: Smart Memory Management System
**Location:** `01_smart_pointers/`

**Features to implement:**
- Resource manager using `std::unique_ptr`
- Shared cache system with `std::shared_ptr` and `std::weak_ptr`
- Custom deleters for different resource types
- RAII wrappers for file handles, network connections

**Key Concepts:**
- `std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`
- Custom deleters
- `std::make_unique`, `std::make_shared`
- Ownership semantics

**Deliverables:**
- [ ] `resource_manager.hpp/cpp` - RAII resource wrapper
- [ ] `cache_system.hpp/cpp` - Shared cache with weak references
- [ ] `tests.cpp` - Unit tests
- [ ] `README.md` - Documentation

---

#### Project 2: High-Performance Data Container
**Location:** `02_move_semantics/`

**Features to implement:**
- Custom vector-like container with move semantics
- Perfect forwarding in factory functions
- RVO/NRVO optimization examples
- Value categories demonstration (lvalue, rvalue, xvalue)

**Key Concepts:**
- Move constructors and move assignment
- `std::move` and `std::forward`
- Universal references (forwarding references)
- Perfect forwarding
- Copy elision and RVO

**Deliverables:**
- [ ] `dynamic_array.hpp` - Custom container
- [ ] `factory.hpp` - Factory with perfect forwarding
- [ ] `benchmark.cpp` - Performance comparison
- [ ] `value_categories.cpp` - Examples

---

#### Project 3: Functional Programming Toolkit
**Location:** `03_lambdas/`

**Features to implement:**
- Algorithm library using lambdas
- Event system with callbacks
- Function composition utilities
- Stateful lambda examples

**Key Concepts:**
- Lambda expressions
- Capture modes (by value, by reference, init captures)
- `std::function` and `std::bind`
- Generic lambdas (C++14)
- Mutable lambdas

**Deliverables:**
- [ ] `algorithms.hpp` - Custom algorithms with predicates
- [ ] `event_system.hpp` - Observer pattern with lambdas
- [ ] `composition.hpp` - Function composition
- [ ] `examples.cpp` - Various lambda patterns

---

### Phase 2: Advanced Features (C++17/20)

#### Project 4: Generic Type-Safe Framework
**Location:** `04_templates_concepts/`

**Features to implement:**
- SFINAE-based type traits
- Constexpr algorithms and data structures
- Concepts-based generic code (C++20)
- Variadic templates utilities

**Key Concepts:**
- Template metaprogramming
- SFINAE and `std::enable_if`
- Concepts and constraints (C++20)
- Fold expressions
- `if constexpr`
- Variadic templates

**Deliverables:**
- [ ] `type_traits.hpp` - Custom type traits
- [ ] `concepts.hpp` - Custom concepts
- [ ] `tuple_utils.hpp` - Tuple manipulation utilities
- [ ] `constexpr_data.hpp` - Compile-time data structures

---

#### Project 5: Functional Data Processing Pipeline
**Location:** `05_ranges_views/`

**Features to implement:**
- Data processing pipeline using ranges
- Custom view adaptors
- Lazy evaluation examples
- Integration with algorithms

**Key Concepts:**
- Ranges library (C++20)
- Views and view adaptors
- Range algorithms
- Lazy evaluation
- Projections

**Deliverables:**
- [ ] `pipeline.hpp` - Pipeline implementation
- [ ] `custom_views.hpp` - Custom view adaptors
- [ ] `data_processor.cpp` - Real-world example
- [ ] `performance.cpp` - Comparison with traditional loops

---

#### Project 6: Async Task Scheduler
**Location:** `06_coroutines/`

**Features to implement:**
- Task scheduler with coroutines
- Generator implementation
- Async file I/O with coroutines
- Lazy evaluation sequences

**Key Concepts:**
- Coroutines (C++20)
- `co_await`, `co_yield`, `co_return`
- Promise and awaitable types
- Generator pattern
- Async/await pattern

**Deliverables:**
- [ ] `task.hpp` - Task coroutine type
- [ ] `generator.hpp` - Generator implementation
- [ ] `scheduler.hpp` - Task scheduler
- [ ] `examples.cpp` - Various coroutine patterns

---

#### Project 7: Modular Library System
**Location:** `07_modules/`

**Features to implement:**
- Convert existing code to modules (C++20)
- Module interface units
- Module partitions
- Build system integration

**Key Concepts:**
- Modules (C++20)
- `import` and `export`
- Module partitions
- Module interface vs implementation

**Deliverables:**
- [ ] `math.ixx` - Math module
- [ ] `utils.ixx` - Utilities module
- [ ] `CMakeLists.txt` - Build configuration
- [ ] Migration guide

---

#### Project 8: Concurrent Data Structures
**Location:** `08_concurrency/`

**Features to implement:**
- Lock-free queue
- Thread-safe cache with `std::shared_mutex`
- Task pool with `std::jthread`
- Atomic operations and memory ordering
- Parallel algorithms

**Key Concepts:**
- `std::thread`, `std::jthread` (C++20)
- Mutexes and locks
- `std::atomic` and memory orders
- `std::condition_variable`
- Parallel algorithms (C++17)
- Latches and barriers (C++20)

**Deliverables:**
- [ ] `lock_free_queue.hpp` - Lock-free data structure
- [ ] `thread_pool.hpp` - Thread pool implementation
- [ ] `parallel_algorithms.cpp` - Parallel algorithm examples
- [ ] `memory_ordering.cpp` - Memory ordering examples

---

### Phase 3: Final Integration Project

#### Project 9: Modern C++ Web Server
**Location:** `09_final_project/`

**Comprehensive project combining all learned features:**

**Features:**
- HTTP server with async I/O (coroutines)
- Thread pool for request handling
- Smart pointer-based resource management
- Middleware system using lambdas
- Plugin system using concepts
- Configuration using ranges
- Modular architecture

**Architecture:**
```
server/
├── core/          # Core server logic with coroutines
├── http/          # HTTP protocol handling
├── middleware/    # Middleware system (lambdas)
├── plugins/       # Plugin interface (concepts)
├── utils/         # Utility functions (ranges)
└── main.cpp       # Entry point
```

**Key Integration Points:**
- Smart pointers for connection management
- Move semantics for efficient data transfer
- Lambdas for request handlers
- Templates for generic middleware
- Ranges for header parsing
- Coroutines for async operations
- Concurrency for multiple connections

---

## Build System Setup

### CMakeLists.txt (Root)
```cmake
cmake_minimum_required(VERSION 3.20)
project(ModernCppLearning CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Enable sanitizers for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT MSVC)
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# Add subdirectories
add_subdirectory(01_smart_pointers)
add_subdirectory(02_move_semantics)
add_subdirectory(03_lambdas)
# ... add others
```

---

## Learning Methodology

### Daily Practice
1. **Read** - Study the feature in documentation
2. **Implement** - Write code for the project
3. **Test** - Write unit tests
4. **Review** - Compare with best practices
5. **Document** - Write README explaining your approach

### Weekly Goals
- Complete one project per week
- Write at least 500 lines of tested code
- Document all design decisions
- Review code with static analyzers

### Tools to Use
- **Compiler:** Clang 15+ or GCC 11+ or MSVC 2022+
- **Build System:** CMake 3.20+
- **Testing:** Google Test or Catch2
- **Static Analysis:** clang-tidy, cppcheck
- **Sanitizers:** AddressSanitizer, ThreadSanitizer, UBSanitizer
- **Formatting:** clang-format
- **Documentation:** Doxygen

---

## Resources

### Books
- "Effective Modern C++" by Scott Meyers
- "C++ Concurrency in Action" by Anthony Williams
- "C++17 STL Cookbook" by Jacek Galowicz
- "C++20 - The Complete Guide" by Nicolai Josuttis

### Online Resources
- [cppreference.com](https://en.cppreference.com/)
- [CppCon YouTube Channel](https://www.youtube.com/user/CppCon)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Compiler Explorer (godbolt.org)](https://godbolt.org/)

### Practice Platforms
- LeetCode (C++ specific problems)
- HackerRank C++ track
- Exercism C++ track

---

## Progress Tracking

### Completed Projects
- [ ] Smart Memory Management System
- [ ] High-Performance Data Container
- [ ] Functional Programming Toolkit
- [ ] Generic Type-Safe Framework
- [ ] Functional Data Processing Pipeline
- [ ] Async Task Scheduler
- [ ] Modular Library System
- [ ] Concurrent Data Structures
- [ ] Modern C++ Web Server

### Skills Acquired
- [ ] Smart pointers mastery
- [ ] Move semantics and perfect forwarding
- [ ] Lambda expressions and functional programming
- [ ] Template metaprogramming
- [ ] Concepts and constraints
- [ ] Ranges and views
- [ ] Coroutines
- [ ] Modules
- [ ] Modern concurrency

---

## Next Steps

1. **Set up development environment**
   ```bash
   cd project
   mkdir -p 01_smart_pointers 02_move_semantics 03_lambdas
   # Create CMakeLists.txt files
   ```

2. **Start with Project 1**
   - Read about smart pointers
   - Implement resource manager
   - Write tests
   - Benchmark

3. **Progress sequentially**
   - Each project builds on previous knowledge
   - Don't skip projects
   - Complete all deliverables

4. **Build final project**
   - Integrates all learned concepts
   - Real-world application
   - Portfolio piece

---

## Additional Challenges

### Expert Level Features (C++23)
- Deducing `this` parameter
- `std::expected` for error handling
- `std::mdspan` for multidimensional arrays
- Explicit object parameters
- Pattern matching (if available)

### Optional Additions
- Add benchmarking to all projects
- Implement comprehensive test suites
- Create performance profiling reports
- Write technical blog posts
- Contribute to open-source C++ projects

---

## Success Metrics

By completing this project, you should be able to:
- ✅ Write idiomatic modern C++ code
- ✅ Understand value categories and move semantics deeply
- ✅ Use smart pointers effectively
- ✅ Write generic, reusable code with templates and concepts
- ✅ Utilize ranges and functional programming patterns
- ✅ Implement concurrent and async code safely
- ✅ Apply RAII and other modern C++ idioms
- ✅ Build production-quality C++ applications

---

**Start Date:** ___________
**Target Completion:** 9-12 weeks
**Daily Time Commitment:** 2-3 hours

Good luck on your modern C++ journey! 🚀
