/**
 * Modern C++ Exercise 04: Templates and Concepts
 * 
 * Topics covered:
 * - Function templates
 * - Class templates
 * - Template specialization
 * - Variadic templates
 * - Template template parameters
 * - SFINAE and enable_if (C++11/14/17)
 * - Concepts and constraints (C++20)
 * - Requires clauses (C++20)
 * 
 * Compile: g++ -std=c++20 -Wall -Wextra -o templates 04_templates_concepts.cpp
 * Note: C++20 support required for concepts
 */

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <concepts>
#include <algorithm>
#include <cmath>

// ============================================================================
// TASK 1: Basic function templates
// ============================================================================

// TODO: Implement a generic max function that works with any comparable type
template<typename T>
T maximum(T a, T b) {
    // Your code here
}

// TODO: Implement a generic swap function
template<typename T>
void swap(T& a, T& b) {
    // Your code here
}

// TODO: Implement a function template with multiple type parameters
template<typename T, typename U>
auto add(T a, U b) {
    // Your code here - return the sum with appropriate type
}

void demonstrateFunctionTemplates() {
    std::cout << "\n=== Task 1: Function Templates ===\n";
    
    // TODO: Test your templates with different types
    // Your code here
}

// ============================================================================
// TASK 2: Class templates
// ============================================================================

// TODO: Implement a generic Pair class template
template<typename T1, typename T2>
class Pair {
private:
    T1 first_;
    T2 second_;
    
public:
    // TODO: Implement constructors
    Pair(const T1& first, const T2& second) {
        // Your code here
    }
    
    // TODO: Implement getters
    T1 getFirst() const {
        // Your code here
    }
    
    T2 getSecond() const {
        // Your code here
    }
    
    // TODO: Implement a method to print the pair
    void print() const {
        // Your code here
    }
};

// TODO: Implement a generic Stack class template
template<typename T>
class Stack {
private:
    std::vector<T> elements_;
    
public:
    // TODO: Implement push, pop, top, empty, size methods
    void push(const T& element) {
        // Your code here
    }
    
    void pop() {
        // Your code here
    }
    
    T& top() {
        // Your code here
    }
    
    const T& top() const {
        // Your code here
    }
    
    bool empty() const {
        // Your code here
    }
    
    size_t size() const {
        // Your code here
    }
};

void demonstrateClassTemplates() {
    std::cout << "\n=== Task 2: Class Templates ===\n";
    
    // TODO: Test your Pair and Stack classes
    // Your code here
}

// ============================================================================
// TASK 3: Template specialization
// ============================================================================

// Generic template
template<typename T>
class Printer {
public:
    void print(const T& value) {
        std::cout << "Generic: " << value << "\n";
    }
};

// TODO: Implement full specialization for bool
template<>
class Printer<bool> {
public:
    void print(const bool& value) {
        // Your code here - print "true" or "false" instead of 1/0
    }
};

// TODO: Implement full specialization for const char*
template<>
class Printer<const char*> {
public:
    void print(const char* const& value) {
        // Your code here - add quotes around strings
    }
};

// TODO: Implement partial specialization for pointers
template<typename T>
class Printer<T*> {
public:
    void print(T* const& value) {
        // Your code here - print address and value
    }
};

void demonstrateSpecialization() {
    std::cout << "\n=== Task 3: Template Specialization ===\n";
    
    // TODO: Test your specializations
    // Your code here
}

// ============================================================================
// TASK 4: Variadic templates
// ============================================================================

// TODO: Implement a variadic print function
// Base case
void print() {
    std::cout << "\n";
}

// Recursive case
template<typename T, typename... Args>
void print(T first, Args... args) {
    // Your code here
}

// TODO: Implement a variadic sum function
template<typename... Args>
auto sum(Args... args) {
    // Your code here - use fold expression (C++17)
}

// TODO: Implement a function that counts the number of arguments
template<typename... Args>
constexpr size_t count(Args... args) {
    // Your code here
}

// TODO: Implement a variadic max function
template<typename T>
T max_value(T value) {
    return value;
}

template<typename T, typename... Args>
T max_value(T first, Args... args) {
    // Your code here - recursively find maximum
}

void demonstrateVariadicTemplates() {
    std::cout << "\n=== Task 4: Variadic Templates ===\n";
    
    // TODO: Test your variadic functions
    // Your code here
}

// ============================================================================
// TASK 5: SFINAE and enable_if (Pre-C++20)
// ============================================================================

// TODO: Implement a function that only works with integral types
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
double_value(T value) {
    // Your code here
}

// TODO: Implement a function that only works with floating point types
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
square_root(T value) {
    // Your code here
}

// TODO: Implement a function that works differently for integral vs floating point
template<typename T>
auto process(T value) -> typename std::enable_if<std::is_integral<T>::value, int>::type {
    std::cout << "Processing integral type\n";
    return static_cast<int>(value * 2);
}

template<typename T>
auto process(T value) -> typename std::enable_if<std::is_floating_point<T>::value, double>::type {
    std::cout << "Processing floating point type\n";
    return value / 2.0;
}

void demonstrateSFINAE() {
    std::cout << "\n=== Task 5: SFINAE and enable_if ===\n";
    
    // TODO: Test your SFINAE functions
    // Your code here
}

// ============================================================================
// TASK 6: C++20 Concepts
// ============================================================================

// TODO: Define concepts for different type categories

// Concept for arithmetic types
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

// Concept for types that can be printed
template<typename T>
concept Printable = requires(T t, std::ostream& os) {
    { os << t } -> std::convertible_to<std::ostream&>;
};

// Concept for containers
template<typename T>
concept Container = requires(T t) {
    typename T::value_type;
    { t.begin() } -> std::same_as<typename T::iterator>;
    { t.end() } -> std::same_as<typename T::iterator>;
    { t.size() } -> std::convertible_to<size_t>;
};

// TODO: Implement functions using concepts

// Function that only accepts arithmetic types
template<Arithmetic T>
T multiply(T a, T b) {
    return a * b;
}

// TODO: Implement a function that prints any printable type
template<Printable T>
void print_value(const T& value) {
    // Your code here
}

// TODO: Implement a function that works with any container
template<Container C>
void print_container(const C& container) {
    // Your code here
}

void demonstrateConcepts() {
    std::cout << "\n=== Task 6: C++20 Concepts ===\n";
    
    // TODO: Test your concept-constrained functions
    // Your code here
}

// ============================================================================
// TASK 7: Requires clauses and expressions
// ============================================================================

// TODO: Implement a function with requires clause
template<typename T>
    requires std::integral<T> || std::floating_point<T>
T absolute(T value) {
    // Your code here
}

// TODO: Implement a function with compound requirements
template<typename T>
    requires requires(T a, T b) {
        { a + b } -> std::convertible_to<T>;
        { a - b } -> std::convertible_to<T>;
        { a * b } -> std::convertible_to<T>;
    }
T compute(T a, T b) {
    return (a + b) * (a - b);
}

// TODO: Create a concept for comparable types
template<typename T>
concept Comparable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
    { a == b } -> std::convertible_to<bool>;
};

// TODO: Implement a generic sort function using Comparable concept
template<typename T>
    requires Comparable<T>
void sort_three(T& a, T& b, T& c) {
    // Your code here - sort three values
}

void demonstrateRequires() {
    std::cout << "\n=== Task 7: Requires Clauses ===\n";
    
    // TODO: Test your requires-constrained functions
    // Your code here
}

// ============================================================================
// TASK 8: Template template parameters
// ============================================================================

// TODO: Implement a function that works with any container template
template<template<typename, typename> class Container, typename T, typename Allocator>
void print_generic_container(const Container<T, Allocator>& container) {
    // Your code here
}

// TODO: Implement a wrapper class that can hold any container
template<typename T, template<typename...> class Container = std::vector>
class DataHolder {
private:
    Container<T> data_;
    
public:
    // TODO: Implement methods to add, remove, and access elements
    void add(const T& item) {
        // Your code here
    }
    
    void print() const {
        // Your code here
    }
    
    size_t size() const {
        // Your code here
    }
};

void demonstrateTemplateTemplateParams() {
    std::cout << "\n=== Task 8: Template Template Parameters ===\n";
    
    // TODO: Test with different container types
    // Your code here
}

// ============================================================================
// BONUS TASK: Advanced template metaprogramming
// ============================================================================

// TODO: Implement compile-time factorial
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// TODO: Implement type traits
template<typename T>
struct RemoveConst {
    using type = T;
};

template<typename T>
struct RemoveConst<const T> {
    using type = T;
};

// TODO: Implement is_same type trait
template<typename T, typename U>
struct IsSame {
    static constexpr bool value = false;
};

template<typename T>
struct IsSame<T, T> {
    static constexpr bool value = true;
};

// TODO: Implement a compile-time list (type list)
template<typename... Ts>
struct TypeList {};

template<typename List>
struct ListSize;

template<typename... Ts>
struct ListSize<TypeList<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

void demonstrateBonusTask() {
    std::cout << "\n=== Bonus Task: Template Metaprogramming ===\n";
    
    // TODO: Test compile-time computations
    std::cout << "Factorial<5> = " << Factorial<5>::value << "\n";
    
    // TODO: Test type traits
    std::cout << "IsSame<int, int> = " << IsSame<int, int>::value << "\n";
    std::cout << "IsSame<int, double> = " << IsSame<int, double>::value << "\n";
    
    // TODO: Test type list
    std::cout << "ListSize<int, double, char> = " 
              << ListSize<TypeList<int, double, char>>::value << "\n";
}

// ============================================================================
// PRACTICAL TASK: Generic data structure
// ============================================================================

// TODO: Implement a generic binary tree with concepts
template<typename T>
    requires Comparable<T>
class BinaryTree {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;
        
        Node(const T& d) : data(d), left(nullptr), right(nullptr) {}
    };
    
    Node* root_;
    
    // TODO: Implement helper methods
    void insert_helper(Node*& node, const T& value) {
        // Your code here
    }
    
    void print_helper(Node* node) const {
        // Your code here - inorder traversal
    }
    
    void destroy_helper(Node* node) {
        // Your code here
    }
    
public:
    BinaryTree() : root_(nullptr) {}
    
    ~BinaryTree() {
        destroy_helper(root_);
    }
    
    void insert(const T& value) {
        insert_helper(root_, value);
    }
    
    void print() const {
        print_helper(root_);
        std::cout << "\n";
    }
};

void demonstratePracticalTask() {
    std::cout << "\n=== Practical Task: Generic Binary Tree ===\n";
    
    // TODO: Test your binary tree with different types
    // Your code here
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "Modern C++ Templates and Concepts Exercise\n";
    std::cout << "===========================================\n";
    
    demonstrateFunctionTemplates();
    demonstrateClassTemplates();
    demonstrateSpecialization();
    demonstrateVariadicTemplates();
    demonstrateSFINAE();
    demonstrateConcepts();
    demonstrateRequires();
    demonstrateTemplateTemplateParams();
    demonstrateBonusTask();
    demonstratePracticalTask();
    
    std::cout << "\n=== All tasks completed! ===\n";
    return 0;
}

/*
 * KEY CONCEPTS TO REMEMBER:
 * 
 * 1. Templates provide compile-time polymorphism
 * 2. Template specialization allows type-specific behavior
 * 3. Variadic templates handle arbitrary number of arguments
 * 4. SFINAE = Substitution Failure Is Not An Error
 * 5. C++20 concepts provide cleaner constraints than SFINAE
 * 6. Concepts improve error messages dramatically
 * 7. Template metaprogramming runs at compile time
 * 8. constexpr and consteval for compile-time computation
 */
