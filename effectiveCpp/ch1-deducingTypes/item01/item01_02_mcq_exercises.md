# MCQ and Practical Exercises: Template and Auto Type Deduction

---

## Part A: Multiple Choice Questions (15 Questions)

### Question 1: Basic Template Type Deduction
```cpp
template<typename T>
void func(T param);

int x = 10;
func(x);
```
What is the type of T and param?

A) T = int&, param = int&  
B) T = int, param = int  
C) T = const int, param = const int  
D) T = int*, param = int*

**Answer:** B

**Explanation:** When passing by value (Case 3), reference-ness is ignored and T is deduced as int.

---

### Question 2: Reference Parameter Deduction
```cpp
template<typename T>
void func(T& param);

const int cx = 10;
func(cx);
```
What is the type of T and param?

A) T = int, param = int&  
B) T = const int, param = const int&  
C) T = const int&, param = const int&  
D) T = int, param = const int&

**Answer:** B

**Explanation:** With reference parameters (Case 1), const-ness becomes part of T. T = const int, param = const int&.

---

### Question 3: Universal Reference with Lvalue
```cpp
template<typename T>
void func(T&& param);

int x = 10;
func(x);
```
What is the type of T and param?

A) T = int, param = int&&  
B) T = int&, param = int&  
C) T = int&&, param = int&&  
D) T = int, param = int&

**Answer:** B

**Explanation:** When an lvalue is passed to a universal reference (Case 2), T becomes a reference type (int&) and reference collapsing makes param also int&.

---

### Question 4: Universal Reference with Rvalue
```cpp
template<typename T>
void func(T&& param);

func(10);
```
What is the type of T and param?

A) T = int, param = int&&  
B) T = int&, param = int&  
C) T = int&&, param = int&&  
D) T = const int, param = const int&&

**Answer:** A

**Explanation:** When an rvalue is passed to a universal reference, normal rules apply. T = int, param = int&&.

---

### Question 5: Const Propagation in Pass-by-Value
```cpp
template<typename T>
void func(T param);

const int cx = 10;
func(cx);
```
What is the type of T and param?

A) T = const int, param = const int  
B) T = int, param = int  
C) T = const int&, param = const int  
D) T = int, param = const int

**Answer:** B

**Explanation:** When passing by value (Case 3), const-ness is stripped because param is an independent copy.

---

### Question 6: Array Decay
```cpp
template<typename T>
void func(T param);

const char name[] = "Hello";
func(name);
```
What is the type of T?

A) T = const char[6]  
B) T = const char*  
C) T = char*  
D) T = const char[]

**Answer:** B

**Explanation:** Arrays decay to pointers when passed by value. T = const char*.

---

### Question 7: Array Without Decay
```cpp
template<typename T>
void func(T& param);

int arr[5] = {1, 2, 3, 4, 5};
func(arr);
```
What is the type of T and param?

A) T = int*, param = int*&  
B) T = int[5], param = int(&)[5]  
C) T = int[], param = int(&)[]  
D) T = int, param = int&

**Answer:** B

**Explanation:** When passing array by reference, it doesn't decay. T = int[5], param = int(&)[5].

---

### Question 8: Auto with Braced Initializer
```cpp
auto x1 = 27;
auto x2{27};
auto x3 = {27};
```
In C++14/17, what are the types?

A) All are int  
B) x1=int, x2=int, x3=std::initializer_list<int>  
C) x1=int, x2=std::initializer_list<int>, x3=std::initializer_list<int>  
D) All are std::initializer_list<int>

**Answer:** B

**Explanation:** auto with = {27} deduces std::initializer_list<int>, but {27} alone (in C++17) deduces int.

---

### Question 9: Auto with Reference
```cpp
int x = 10;
auto& rx = x;
rx = 20;
```
What is the value of x after this code?

A) 10  
B) 20  
C) Undefined  
D) Compilation error

**Answer:** B

**Explanation:** rx is a reference to x (type int&), so modifying rx modifies x.

---

### Question 10: Auto with Universal Reference
```cpp
int x = 10;
auto&& rx = x;
auto&& ry = 20;
```
What are the types of rx and ry?

A) rx = int&&, ry = int&&  
B) rx = int&, ry = int&&  
C) rx = int, ry = int  
D) rx = int&, ry = int&

**Answer:** B

**Explanation:** x is lvalue → rx = int&. 20 is rvalue → ry = int&&.

---

### Question 11: Pointer to Const
```cpp
template<typename T>
void func(T param);

const int* ptr = nullptr;
func(ptr);
```
What is the type of T?

A) const int*  
B) int*  
C) const int* const  
D) int

**Answer:** A

**Explanation:** When passing by value, top-level const is ignored but low-level const (pointed-to const) is preserved. T = const int*.

---

### Question 12: Function Pointer Deduction
```cpp
template<typename T>
void func(T param);

void someFunc(int, double);
func(someFunc);
```
What is the type of T?

A) void(int, double)  
B) void(*)(int, double)  
C) void(&)(int, double)  
D) Function type (ill-formed)

**Answer:** B

**Explanation:** Functions decay to function pointers when passed by value. T = void(*)(int, double).

---

### Question 13: Auto in Function Return Type
```cpp
auto func() {
    return {1, 2, 3};
}
```
What happens?

A) Returns std::initializer_list<int>  
B) Returns std::vector<int>  
C) Returns int  
D) Compilation error

**Answer:** D

**Explanation:** auto in return type uses template type deduction, which cannot deduce braced initializers. This is a compile error.

---

### Question 14: Const Reference to Reference
```cpp
template<typename T>
void func(T& param);

int x = 10;
int& rx = x;
func(rx);
```
What is the type of T?

A) int&  
B) int&&  
C) int  
D) int&& &

**Answer:** C

**Explanation:** Reference-ness of the argument is ignored during deduction. T = int, param = int&.

---

### Question 15: Perfect Forwarding
```cpp
template<typename T>
void wrapper(T&& param) {
    func(std::forward<T>(param));
}

int x = 10;
wrapper(x);
```
Inside wrapper, what is the type of param?

A) int&  
B) int&&  
C) int  
D) const int&

**Answer:** A

**Explanation:** x is lvalue, so T = int&, and param = int& (after reference collapsing).

---

## Part B: Practical Exercises

---

## Exercise 1: Code Review - Find the Type Deduction Errors

### Task
Review the following code and identify all type deduction issues. Explain what's wrong and how to fix it.

```cpp
#include <iostream>
#include <vector>
#include <memory>

// Problem 1: Template function
template<typename T>
void processValue(T value) {
    value = 42;  // Intent: Modify the original
}

// Problem 2: Auto with braced initializers
void configSystem() {
    auto config = {100, 200, 300};
    auto firstValue = config[0];  // What's wrong here?
}

// Problem 3: Universal reference misuse
template<typename T>
void forwardData(T&& data) {
    std::vector<T> storage;
    storage.push_back(data);  // What's wrong?
}

// Problem 4: Array size deduction
template<typename T>
void printArraySize(T arr) {
    std::cout << "Size: " << sizeof(arr) / sizeof(arr[0]) << "\n";
}

// Problem 5: Const correctness
template<typename T>
void processContainer(T& container) {
    container.clear();  // What if we pass const container?
}

// Problem 6: Return type deduction
auto createList() {
    return {1, 2, 3, 4, 5};
}

// Problem 7: Reference wrapper confusion
template<typename T>
void modifyValue(T param) {
    param++;
}

int main() {
    // Test Problem 1
    int x = 10;
    processValue(x);
    std::cout << x << "\n";  // Expected: 42, Actual: ?
    
    // Test Problem 2
    configSystem();
    
    // Test Problem 3
    std::string str = "Hello";
    forwardData(str);
    forwardData(std::string("World"));
    
    // Test Problem 4
    int numbers[] = {1, 2, 3, 4, 5};
    printArraySize(numbers);
    
    // Test Problem 5
    std::vector<int> vec = {1, 2, 3};
    const std::vector<int> cvec = {4, 5, 6};
    processContainer(vec);
    processContainer(cvec);  // Should this compile?
    
    // Test Problem 6
    auto list = createList();
    
    // Test Problem 7
    int value = 100;
    modifyValue(value);
    std::cout << value << "\n";  // Expected: 101, Actual: ?
    
    return 0;
}
```

### Your Tasks:
1. Identify each problem and explain the type deduction issue
2. Provide corrected versions of each function
3. Explain what types are actually deduced in each case
4. Write test cases that demonstrate the correct behavior

### Expected Findings:
- Problem 1: Pass by value doesn't modify original
- Problem 2: Can't index initializer_list directly
- Problem 3: T could be int& or int, leading to storage issues
- Problem 4: Array decays to pointer, losing size information
- Problem 5: No const overload or SFINAE constraint
- Problem 6: Can't return braced initializer with auto
- Problem 7: Pass by value doesn't modify original

---

## Exercise 2: Debugging Code - Fix the Type Deduction Issues

### Task
The following code has several subtle type deduction bugs that cause incorrect behavior. Debug and fix them.

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Bug 1: Trying to create a generic swap
template<typename T>
void mySwap(T a, T b) {
    T temp = a;
    a = b;
    b = temp;
}

// Bug 2: Generic container printer
template<typename Container>
void printContainer(Container c) {
    for (auto& elem : c) {
        std::cout << elem << " ";
    }
    std::cout << "\n";
}

// Bug 3: Factory with deduction
template<typename T>
std::shared_ptr<T> makeShared(T obj) {
    return std::make_shared<T>(obj);
}

// Bug 4: Value transformer
template<typename T, typename Func>
auto transform(T& value, Func f) {
    return f(value);
}

// Bug 5: Container concatenation
template<typename Container>
Container concatenate(Container c1, Container c2) {
    c1.insert(c1.end(), c2.begin(), c2.end());
    return c1;
}

// Bug 6: Reference parameter issue
template<typename T>
void processData(T&& data) {
    std::cout << "Processing: " << data << "\n";
    data = data * 2;  // Intended to double the value
}

// Test code
int main() {
    // Test Bug 1
    int a = 5, b = 10;
    mySwap(a, b);
    std::cout << "a=" << a << ", b=" << b << "\n";  // Expected: a=10, b=5
    
    // Test Bug 2
    std::vector<int> huge_vec(1000000, 42);
    printContainer(huge_vec);  // Performance issue!
    
    // Test Bug 3
    std::string str = "Hello";
    auto ptr = makeShared(str);  // Extra copy!
    
    // Test Bug 4
    int value = 10;
    auto result = transform(value, [](auto x) { return x * 2; });
    std::cout << "Value: " << value << ", Result: " << result << "\n";
    
    // Test Bug 5
    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {4, 5, 6};
    auto v3 = concatenate(v1, v2);  // Multiple copies!
    
    // Test Bug 6
    int x = 100;
    processData(x);
    std::cout << "x=" << x << "\n";  // Expected: 200, Actual: ?
    
    processData(50);  // This might not compile or behave unexpectedly
    
    return 0;
}
```

### Your Tasks:
1. Identify why each function doesn't work as intended
2. Explain the type deduction happening
3. Fix each function using proper type deduction techniques
4. Measure performance difference (for relevant functions)
5. Write unit tests to verify correctness

### Debugging Steps:
1. Add type printing to see what's deduced
2. Check value categories (lvalue vs rvalue)
3. Verify const-correctness
4. Check for unnecessary copies
5. Test with move-only types

---

## Exercise 3: Implementation from Scratch - Type-Safe Variant

### Task
Implement a simplified `std::variant`-like class that demonstrates mastery of type deduction.

### Requirements:
```cpp
template<typename... Types>
class Variant {
public:
    // Constructor with perfect forwarding and type deduction
    template<typename T>
    Variant(T&& value);
    
    // Get with auto deduction
    template<typename T>
    auto get() -> T&;
    
    template<typename T>
    auto get() const -> const T&;
    
    // Visit with deduced return type
    template<typename Visitor>
    auto visit(Visitor&& visitor);
    
    // Type checking at compile time
    template<typename T>
    constexpr bool holds() const;
    
    // Index of current type
    auto index() const -> std::size_t;
};

// Helper function with deduced return type
template<typename... Types>
auto makeVariant(auto&& value) {
    return Variant<Types...>(std::forward<decltype(value)>(value));
}
```

### Implementation Requirements:
1. Use template type deduction correctly
2. Support both lvalue and rvalue construction
3. Implement proper forwarding
4. Handle const correctness
5. Use decltype and decltype(auto) appropriately
6. Add SFINAE to restrict invalid types
7. Implement move semantics properly

### Test Cases:
```cpp
// Test basic usage
Variant<int, std::string, double> v1 = 42;
Variant<int, std::string, double> v2 = std::string("hello");
Variant<int, std::string, double> v3 = 3.14;

// Test get
auto i = v1.get<int>();
auto s = v2.get<std::string>();

// Test visit
auto result = v1.visit([](auto&& val) {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, int>) {
        return val * 2;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return val.length();
    } else {
        return static_cast<int>(val);
    }
});

// Test with move-only type
Variant<std::unique_ptr<int>, std::string> v4 = 
    std::make_unique<int>(100);

// Test const correctness
const Variant<int, std::string> cv = 42;
const auto& ref = cv.get<int>();
```

### Bonus Challenges:
1. Implement recursive variants
2. Add exception safety guarantees
3. Optimize for trivially copyable types
4. Support emplace construction
5. Implement comparison operators with auto return types

---

## Exercise 4: Performance Optimization - Zero-Cost Abstractions

### Task
Optimize the following code by fixing type deduction issues that cause performance problems.

### Original Code:
```cpp
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>

// Version 1: What's wrong here?
template<typename Container, typename Func>
void process_v1(Container container, Func func) {
    for (auto element : container) {
        func(element);
    }
}

// Version 2: Still not optimal
template<typename Container, typename Func>
void process_v2(Container& container, Func& func) {
    for (auto element : container) {
        func(element);
    }
}

// Version 3: Getting closer
template<typename Container, typename Func>
void process_v3(const Container& container, Func func) {
    for (const auto& element : container) {
        func(element);
    }
}

// Test with large objects
struct LargeObject {
    std::array<double, 1000> data;
    LargeObject() { data.fill(1.0); }
};

int main() {
    std::vector<LargeObject> objects(10000);
    
    auto process = [](const auto& obj) {
        // Some processing
        return obj.data[0] * 2.0;
    };
    
    // Benchmark each version
    auto benchmark = [&](auto func, const char* name) {
        auto start = std::chrono::high_resolution_clock::now();
        func(objects, process);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << name << ": " << duration.count() << "ms\n";
    };
    
    benchmark(process_v1<decltype(objects), decltype(process)>, "v1");
    benchmark(process_v2<decltype(objects), decltype(process)>, "v2");
    benchmark(process_v3<decltype(objects), decltype(process)>, "v3");
    
    return 0;
}
```

### Your Tasks:
1. Identify performance issues in each version
2. Explain type deduction implications
3. Create an optimal version using:
   - Perfect forwarding
   - Universal references
   - decltype(auto)
   - Forwarding references
4. Measure and compare performance
5. Verify zero-cost abstraction (compare with hand-written loop)
6. Create version that works with:
   - Lvalue containers
   - Rvalue containers (temporaries)
   - Move-only elements
   - Const and non-const containers

### Expected Analysis:
- v1: Copies entire container! Multiple LargeObject copies
- v2: Still copies each element in loop
- v3: Better, but can't modify elements, can't handle rvalues
- Optimal: Should use `Container&&` and `decltype(auto)` for element

### Performance Requirements:
- Zero unnecessary copies
- Inline-able by compiler
- Same performance as raw loop
- No heap allocations
- Cache-friendly

---

## Exercise 5: Advanced Code Review - Template Library Design

### Task
Review this template library design and identify all type deduction issues, design flaws, and suggest improvements.

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>

namespace lib {

// Library code to review

// 1. Generic algorithm
template<typename T, typename Func>
std::vector<T> map(std::vector<T> vec, Func func) {
    std::vector<T> result;
    for (auto elem : vec) {
        result.push_back(func(elem));
    }
    return result;
}

// 2. Filter implementation
template<typename T, typename Pred>
std::vector<T> filter(std::vector<T> vec, Pred pred) {
    std::vector<T> result;
    for (const auto& elem : vec) {
        if (pred(elem)) {
            result.push_back(elem);
        }
    }
    return result;
}

// 3. Reduce/fold
template<typename T, typename Func>
T reduce(std::vector<T> vec, T init, Func func) {
    auto result = init;
    for (auto elem : vec) {
        result = func(result, elem);
    }
    return result;
}

// 4. Compose functions
template<typename F, typename G>
auto compose(F f, G g) {
    return [f, g](auto x) { return f(g(x)); };
}

// 5. Curry function
template<typename Func, typename Arg>
auto curry(Func func, Arg arg) {
    return [func, arg](auto... args) {
        return func(arg, args...);
    };
}

// 6. Maybe type (optional-like)
template<typename T>
class Maybe {
    T value;
    bool has_value;
    
public:
    Maybe(T val) : value(val), has_value(true) {}
    Maybe() : has_value(false) {}
    
    T getValue() { return value; }
    bool hasValue() { return has_value; }
    
    template<typename Func>
    auto map(Func func) {
        if (has_value) {
            return Maybe(func(value));
        }
        return Maybe();
    }
};

// 7. Lazy evaluation
template<typename T>
class Lazy {
    std::function<T()> computation;
    T cached_value;
    bool computed;
    
public:
    Lazy(std::function<T()> comp) 
        : computation(comp), computed(false) {}
    
    T get() {
        if (!computed) {
            cached_value = computation();
            computed = true;
        }
        return cached_value;
    }
};

} // namespace lib

// Usage examples that reveal problems
int main() {
    using namespace lib;
    
    // Example 1: Type transformation
    std::vector<int> nums = {1, 2, 3, 4, 5};
    auto doubled = map(nums, [](int x) { return x * 2.0; });  // Problem?
    
    // Example 2: Filter with temporary
    auto evens = filter(std::vector{1, 2, 3, 4, 5}, 
                       [](int x) { return x % 2 == 0; });  // Problem?
    
    // Example 3: Reduce type mismatch
    auto sum = reduce(nums, 0.0, [](auto a, auto b) { return a + b; });  // Problem?
    
    // Example 4: Compose with different types
    auto f = [](double x) { return static_cast<int>(x); };
    auto g = [](int x) { return x * 2.0; };
    auto h = compose(f, g);
    auto result = h(5);  // What type?
    
    // Example 5: Curry issues
    auto add = [](int a, int b) { return a + b; };
    auto add5 = curry(add, 5);
    auto result2 = add5(10);  // Does this work?
    
    // Example 6: Maybe with move-only type
    Maybe<std::unique_ptr<int>> maybe_ptr(std::make_unique<int>(42));  // Problem?
    auto mapped = maybe_ptr.map([](auto ptr) { return *ptr * 2; });  // Problem?
    
    // Example 7: Lazy with references
    int x = 100;
    Lazy<int> lazy([&x]() { return x * 2; });
    x = 200;
    auto value = lazy.get();  // What value?
    
    return 0;
}
```

### Your Tasks:

#### Part 1: Identify Issues (at least 15 issues)
For each issue, explain:
- What type is deduced?
- What goes wrong?
- Why it's a problem?

#### Part 2: Categorize Problems
- Type deduction errors
- Performance issues
- Correctness issues
- Design flaws
- Missing features

#### Part 3: Redesign
Create improved versions using:
- Perfect forwarding
- Universal references
- decltype(auto)
- SFINAE/concepts
- Proper cv-qualifiers
- Move semantics

#### Part 4: Add Features
Extend the library with:
- Support for different container types
- Lazy evaluation support
- Composable operations
- Exception safety
- Move-only type support

#### Part 5: Documentation
Write comprehensive documentation explaining:
- Type deduction behavior
- Performance characteristics
- Usage guidelines
- Common pitfalls

### Expected Issues to Find:
1. map() changes return type but returns vector<T>
2. Multiple unnecessary copies
3. No support for move-only types
4. Maybe doesn't forward properly
5. Lazy captures by value (performance issue)
6. No perfect forwarding anywhere
7. Functions don't work with temporaries efficiently
8. Auto captures in lambdas might be problematic
9. No const-correctness
10. Missing noexcept specifications
11. Type erasure in Lazy (std::function overhead)
12. No SFINAE constraints
13. Reference issues in curry
14. Maybe map returns wrong type
15. Composition doesn't preserve value categories

---

## Submission Format

For each exercise:

### 1. Analysis Document
- Problem identification
- Type deduction explanation
- Design rationale

### 2. Code Files
- Corrected implementations
- Test cases
- Benchmarks (where applicable)

### 3. Compiler Output
- Show actual types using decltype
- Include compiler explorer links
- Show assembly for performance-critical code

### 4. Test Results
- Unit test output
- Performance measurements
- Edge case verification

---

## Grading Rubric

### MCQ Section (20%)
- Correct answers with explanation

### Code Review (20%)
- Issue identification accuracy
- Explanation quality
- Fix correctness

### Debugging (20%)
- Bug finding completeness
- Root cause analysis
- Fix quality

### Implementation (25%)
- Code correctness
- Type deduction mastery
- Performance optimization
- Error handling

### Performance Optimization (15%)
- Performance improvement
- Zero-cost abstraction verification
- Benchmarking methodology

---

## Tips for Success

1. **Use Type Inspection Tools:**
   ```cpp
   template<typename T>
   void print_type() {
       std::cout << __PRETTY_FUNCTION__ << "\n";
   }
   ```

2. **Static Assertions:**
   ```cpp
   static_assert(std::is_same_v<T, int>, "Expected int");
   ```

3. **Compiler Explorers:**
   - https://godbolt.org
   - https://cppinsights.io

4. **Enable Warnings:**
   ```bash
   g++ -std=c++20 -Wall -Wextra -Wpedantic -O2
   ```

5. **Profile Your Code:**
   ```bash
   g++ -std=c++20 -O3 -march=native
   perf stat ./your_program
   ```

6. **Test Edge Cases:**
   - Const objects
   - Temporary objects
   - Move-only types
   - Arrays
   - Function pointers

Good luck! 🚀
