# Modern C++ Exercises - Solutions Guide

This document provides hints and solutions for the exercises. Try to solve the exercises yourself before looking at the solutions!

## Exercise 01: Smart Pointers

### Key Solutions

**Task 1: unique_ptr Demo**
```cpp
void demonstrateUniquePtr() {
    auto ptr1 = std::make_unique<Resource>("Task1", 42);
    ptr1->use();
    
    auto ptr2 = std::move(ptr1);  // Transfer ownership
    if (ptr1 == nullptr) {
        std::cout << "ptr1 is now null\n";
    }
    ptr2->use();
}
```

**Task 4: weak_ptr Solution**
- Change `std::shared_ptr<Person> partner;` to `std::weak_ptr<Person> partner;`
- Use `lock()` to access the object: `if (auto p = partner.lock()) { use p }`

**Bonus Task: SimpleUniquePtr**
```cpp
~SimpleUniquePtr() {
    delete ptr_;
}

T* release() {
    T* temp = ptr_;
    ptr_ = nullptr;
    return temp;
}
```

---

## Exercise 02: Move Semantics

### Key Solutions

**Task 2: Move Constructor**
```cpp
DynamicArray(DynamicArray&& other) noexcept 
    : data_(other.data_), size_(other.size_) {
    std::cout << "Move constructor: Moving array of size " << size_ << "\n";
    other.data_ = nullptr;
    other.size_ = 0;
}
```

**Task 2: Move Assignment**
```cpp
DynamicArray& operator=(DynamicArray&& other) noexcept {
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "Move assignment\n";
    }
    return *this;
}
```

**Task 4: Perfect Forwarding**
```cpp
template<typename T>
Widget createWidget(T&& name, int id) {
    return Widget(std::forward<T>(name), id);
}
```

**Key Point**: Use `std::move` for rvalues you own, `std::forward` for universal references.

---

## Exercise 03: Lambda Expressions

### Key Solutions

**Task 2: Capture Examples**
```cpp
int x = 10, y = 20;
auto captureByValue = [x, y]() { /* cannot modify x, y */ };
auto captureByRef = [&x, &y]() { x++; y++; };
auto captureAll = [=]() { /* all by value */ };
auto captureAllRef = [&]() { /* all by reference */ };
auto mixed = [=, &x]() { /* all by value except x */ };
```

**Task 3: Mutable Lambda**
```cpp
int counter = 0;
auto increment = [counter]() mutable {
    return ++counter;
};
// counter in lambda is separate from outer counter
```

**Task 6: std::function Map**
```cpp
std::map<std::string, std::function<int(int,int)>> ops;
ops["+"] = [](int a, int b) { return a + b; };
ops["-"] = [](int a, int b) { return a - b; };
ops["*"] = [](int a, int b) { return a * b; };
ops["/"]; = [](int a, int b) { return b != 0 ? a / b : 0; };
```

---

## Exercise 04: Templates and Concepts

### Key Solutions

**Task 4: Variadic Print**
```cpp
void print() { std::cout << "\n"; }

template<typename T, typename... Args>
void print(T first, Args... args) {
    std::cout << first << " ";
    print(args...);
}
```

**Task 4: Variadic Sum (C++17 Fold Expression)**
```cpp
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}
```

**Task 6: Concepts**
```cpp
template<Printable T>
void print_value(const T& value) {
    std::cout << value << "\n";
}

template<Container C>
void print_container(const C& container) {
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << "\n";
}
```

---

## Exercise 05: STL Algorithms and Ranges

### Key Solutions

**Task 1: Searching**
```cpp
auto even = std::find_if(numbers.begin(), numbers.end(), 
    [](int n) { return n % 2 == 0; });

int evenCount = std::count_if(numbers.begin(), numbers.end(),
    [](int n) { return n % 2 == 0; });

bool hasLarge = std::any_of(numbers.begin(), numbers.end(),
    [](int n) { return n > 10; });
```

**Task 6: Ranges Basics**
```cpp
auto even = numbers | std::views::filter([](int n) { return n % 2 == 0; });
auto squared = even | std::views::transform([](int n) { return n * n; });
auto first3 = squared | std::views::take(3);

// Chain together:
auto result = numbers 
    | std::views::filter([](int n) { return n % 2 == 0; })
    | std::views::transform([](int n) { return n * n; })
    | std::views::take(3);
```

**Task 8: Range with Projection**
```cpp
std::ranges::sort(people, {}, &Person::age);
auto it = std::ranges::find(people, "Alice", &Person::name);
```

---

## Exercise 06: Concurrency

### Key Solutions

**Task 2: Thread-Safe Counter**
```cpp
void increment() {
    std::lock_guard<std::mutex> lock(mutex_);
    value_++;
}
```

**Task 3: Transfer Between Accounts**
```cpp
static bool transfer(BankAccount& from, BankAccount& to, double amount) {
    std::scoped_lock lock(from.mutex_, to.mutex_);  // C++17
    if (from.balance_ >= amount) {
        from.balance_ -= amount;
        to.balance_ += amount;
        return true;
    }
    return false;
}
```

**Task 4: Condition Variable**
```cpp
void push(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(message);
    cv_.notify_one();
}

std::string pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty() || done_; });
    if (queue_.empty()) return "";
    std::string msg = queue_.front();
    queue_.pop();
    return msg;
}
```

**Task 6: std::async**
```cpp
auto future = std::async(std::launch::async, computeSum, 1, 1000);
int result = future.get();  // Blocks until ready
```

---

## Exercise 07: Miscellaneous Features

### Key Solutions

**Task 3: Structured Bindings**
```cpp
auto [id, name] = getIdAndName();
auto [x, y, z] = Point{1.0, 2.0, 3.0};

std::map<std::string, int> ages;
for (const auto& [name, age] : ages) {
    std::cout << name << " is " << age << "\n";
}
```

**Task 4: std::optional**
```cpp
std::optional<int> findIndex(const std::vector<int>& vec, int target) {
    auto it = std::find(vec.begin(), vec.end(), target);
    if (it != vec.end()) {
        return std::distance(vec.begin(), it);
    }
    return std::nullopt;
}

// Usage:
if (auto index = findIndex(vec, 42); index.has_value()) {
    std::cout << "Found at: " << *index << "\n";
}
```

**Task 5: std::variant with std::visit**
```cpp
void processNumber(const Number& num) {
    std::visit([](const auto& value) {
        std::cout << value << "\n";
    }, num);
}
```

**Task 8: constexpr**
```cpp
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int fact5 = factorial(5);  // Computed at compile time
```

---

## General Tips

### Smart Pointers
- Use `unique_ptr` by default
- Use `shared_ptr` only when you need shared ownership
- Use `weak_ptr` to break circular references
- Always prefer `make_unique` and `make_shared`

### Move Semantics
- Move constructor: steal resources, nullify source
- Always mark move operations as `noexcept`
- Don't use `std::move` on return values (prevents RVO)
- Use `std::forward` for perfect forwarding

### Lambda Expressions
- Capture by reference `[&]` for local variables you'll modify
- Capture by value `[=]` for small values you'll read
- Use `mutable` to modify value-captured variables
- Generic lambdas with `auto` work like templates

### Templates and Concepts
- Prefer C++20 concepts over SFINAE
- Use fold expressions for variadic templates
- Concepts provide much better error messages
- Template metaprogramming happens at compile time

### Concurrency
- Always join or detach threads
- Use RAII locks (never lock/unlock manually)
- Check predicates when using condition variables
- Prefer `std::async` for simple async tasks
- Use atomics for simple lock-free operations

### Modern Features
- Use `auto` where type is obvious
- Use structured bindings to unpack values
- Prefer `std::optional` over null pointers
- Use `std::variant` instead of unions
- Leverage `constexpr` for compile-time computation

---

## Compilation Commands

### Basic C++17
```bash
g++ -std=c++17 -Wall -Wextra -o exercise exercise.cpp
```

### C++20 (for concepts, ranges, etc.)
```bash
g++ -std=c++20 -Wall -Wextra -o exercise exercise.cpp
```

### With Threading
```bash
g++ -std=c++20 -Wall -Wextra -pthread -o exercise exercise.cpp
```

### With Optimizations
```bash
g++ -std=c++20 -Wall -Wextra -O3 -pthread -o exercise exercise.cpp
```

---

## Further Reading

- **Books**:
  - "Effective Modern C++" by Scott Meyers
  - "C++ Concurrency in Action" by Anthony Williams
  - "C++17 in Detail" by Bartłomiej Filipek
  - "C++20 - The Complete Guide" by Nicolai Josuttis

- **Websites**:
  - [cppreference.com](https://en.cppreference.com/)
  - [isocpp.org](https://isocpp.org/)
  - [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)

- **Practice**:
  - [LeetCode](https://leetcode.com/)
  - [HackerRank C++](https://www.hackerrank.com/domains/cpp)
  - [Exercism C++ Track](https://exercism.org/tracks/cpp)

Happy coding! 🚀
