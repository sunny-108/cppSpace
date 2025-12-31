# Modern C++ Exercises

A comprehensive collection of exercises to master modern C++ features (C++11/14/17/20).

## Overview

These exercises are designed to help you understand and practice the key features introduced in modern C++ standards. Each exercise file is self-contained with clear instructions, tasks, and compilation guidelines.

## Exercise Structure

1. **01_smart_pointers.cpp** - Smart pointers and RAII
2. **02_move_semantics.cpp** - Move semantics and perfect forwarding
3. **03_lambda_expressions.cpp** - Lambda functions and closures
4. **04_templates_concepts.cpp** - Modern templates and C++20 concepts
5. **05_stl_algorithms.cpp** - STL algorithms and ranges
6. **06_concurrency.cpp** - Modern concurrency primitives
7. **07_misc_features.cpp** - Miscellaneous modern features

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 12+, or MSVC 2019+)
- Basic understanding of C++ fundamentals

## Compilation

### For C++17 exercises:
```bash
g++ -std=c++17 -Wall -Wextra -o exercise exercise_name.cpp
```

### For C++20 exercises:
```bash
g++ -std=c++20 -Wall -Wextra -o exercise exercise_name.cpp
```

### With threading support:
```bash
g++ -std=c++20 -Wall -Wextra -pthread -o exercise exercise_name.cpp
```

## Learning Path

If you're new to modern C++, follow the exercises in order:

1. Start with **smart pointers** to understand memory management
2. Learn **move semantics** for efficient resource handling
3. Master **lambda expressions** for functional programming
4. Explore **templates and concepts** for generic programming
5. Practice **STL algorithms** for idiomatic C++
6. Study **concurrency** for parallel programming
7. Review **miscellaneous features** for additional tools

## Key Modern C++ Features Covered

### C++11
- Smart pointers (`unique_ptr`, `shared_ptr`, `weak_ptr`)
- Move semantics and rvalue references
- Lambda expressions
- `auto` and `decltype`
- Range-based for loops
- Variadic templates
- `std::thread` and concurrency primitives

### C++14
- Generic lambdas
- `std::make_unique`
- Return type deduction
- Binary literals

### C++17
- Structured bindings
- `if` and `switch` with initializers
- `std::optional`, `std::variant`, `std::any`
- Fold expressions
- `std::string_view`

### C++20
- Concepts and constraints
- Ranges library
- Coroutines
- Three-way comparison operator (`<=>`)
- Modules (limited support)

## Tips for Success

1. **Read the comments carefully** - Each exercise has detailed instructions
2. **Compile frequently** - Use compiler errors as learning opportunities
3. **Experiment** - Modify the code to see what happens
4. **Use references** - Check cppreference.com when stuck
5. **Test edge cases** - Think about what could go wrong
6. **Enable warnings** - Always compile with `-Wall -Wextra`

## Additional Resources

- [cppreference.com](https://en.cppreference.com/) - Comprehensive C++ reference
- [isocpp.org](https://isocpp.org/) - ISO C++ Foundation
- "Effective Modern C++" by Scott Meyers
- "C++ Concurrency in Action" by Anthony Williams

## Progress Tracking

- [ ] Exercise 01: Smart Pointers
- [ ] Exercise 02: Move Semantics
- [ ] Exercise 03: Lambda Expressions
- [ ] Exercise 04: Templates & Concepts
- [ ] Exercise 05: STL Algorithms
- [ ] Exercise 06: Concurrency
- [ ] Exercise 07: Miscellaneous Features

Happy coding! 🚀
