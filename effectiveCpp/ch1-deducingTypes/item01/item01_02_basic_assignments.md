# Basic Programming Assignments: Template and Auto Type Deduction

## Difficulty Level: Easy to Medium

These assignments are designed to build foundational understanding of template and auto type deduction through hands-on practice.

---

## Assignment 1: Type Deduction Detective (Easy)

### Objective
Learn to predict and verify type deduction results in various scenarios.

### Task
Create a program that demonstrates type deduction in all three cases and prints the deduced types.

### Requirements

1. **Implement a type printer utility:**
```cpp
#include <iostream>
#include <typeinfo>
#include <string>

// Helper to print type information
template<typename T>
void printType(const std::string& varName) {
    std::cout << varName << " has type: " << typeid(T).name() << std::endl;
}
```

2. **Create three template functions demonstrating each case:**
```cpp
// Case 1: Reference parameter
template<typename T>
void caseOne(T& param) {
    // Print T and param types
}

// Case 2: Universal reference parameter
template<typename T>
void caseTwo(T&& param) {
    // Print T and param types
}

// Case 3: Pass by value parameter
template<typename T>
void caseThree(T param) {
    // Print T and param types
}
```

3. **Test with various inputs:**
```cpp
int main() {
    int x = 10;
    const int cx = 20;
    const int& rx = x;
    
    // Test Case 1 with all three
    caseOne(x);
    caseOne(cx);
    caseOne(rx);
    
    // Test Case 2 with all three
    caseTwo(x);
    caseTwo(cx);
    caseTwo(10);  // rvalue
    
    // Test Case 3 with all three
    caseThree(x);
    caseThree(cx);
    caseThree(rx);
    
    return 0;
}
```

### Expected Learning Outcomes
- Understand how const-ness is preserved or stripped
- See how references are handled differently
- Observe lvalue vs rvalue behavior

### Hints
- Use `__PRETTY_FUNCTION__` or `__FUNCSIG__` for better type names
- Try `typeid(T).name()` but note it may show mangled names
- Use an online compiler like godbolt.org to see deduced types

### Bonus Challenge
Add a compile-time type checker using `static_assert` to verify your predictions.

---

## Assignment 2: Auto Variable Practice (Easy)

### Objective
Master auto type deduction by creating variables with different type specifiers.

### Task
Create a program that demonstrates auto deduction in various scenarios.

### Requirements

1. **Basic auto declarations:**
```cpp
int main() {
    // Task 1: Deduce as int
    auto a = 42;
    
    // Task 2: Deduce as double
    auto b = 3.14;
    
    // Task 3: Deduce as pointer
    int x = 10;
    auto c = &x;
    
    // Task 4: Deduce as reference
    auto& d = x;
    
    // Task 5: Deduce as const reference
    const auto& e = x;
    
    // Print all types
}
```

2. **Test const propagation:**
```cpp
void testConstPropagation() {
    const int cx = 100;
    
    auto a = cx;          // What type?
    auto& b = cx;         // What type?
    const auto& c = cx;   // What type?
    
    // Try to modify each - which ones compile?
    // a = 200;
    // b = 200;
    // c = 200;
}
```

3. **Universal references with auto:**
```cpp
void testUniversalRef() {
    int x = 10;
    
    auto&& r1 = x;      // What type?
    auto&& r2 = 20;     // What type?
    
    // Can you modify x through r1?
    r1 = 100;
    std::cout << "x = " << x << std::endl;
}
```

4. **The braced initializer special case:**
```cpp
void testBracedInit() {
    auto x1 = 27;       // int
    auto x2{27};        // int (C++17)
    auto x3 = {27};     // std::initializer_list<int>
    
    // Try to print types and values
}
```

### Expected Output Format
```
Variable a: type=int, value=42
Variable b: type=double, value=3.14
Variable c: type=int*, value=<address>
...
```

### Bonus Challenge
Create a template function that accepts auto parameters (C++20 abbreviated function template).

---

## Assignment 3: Array Type Deduction (Easy-Medium)

### Objective
Understand how arrays behave differently in template type deduction.

### Task
Implement functions that demonstrate array decay and preservation.

### Requirements

1. **Array decay demonstration:**
```cpp
template<typename T>
void printArrayDecay(T param) {
    std::cout << "Array decayed to pointer" << std::endl;
    std::cout << "sizeof(param) = " << sizeof(param) << std::endl;
}

template<typename T>
void printArrayNoDecay(T& param) {
    std::cout << "Array did NOT decay" << std::endl;
    std::cout << "sizeof(param) = " << sizeof(param) << std::endl;
}
```

2. **Array size deduction:**
```cpp
// Implement a function that returns array size at compile time
template<typename T, std::size_t N>
constexpr std::size_t getArraySize(T (&)[N]) {
    return N;
}

// Use it
int arr1[5] = {1, 2, 3, 4, 5};
int arr2[10];

std::cout << "arr1 size: " << getArraySize(arr1) << std::endl;
std::cout << "arr2 size: " << getArraySize(arr2) << std::endl;
```

3. **Create a safe array print function:**
```cpp
template<typename T, std::size_t N>
void printArray(const T (&arr)[N]) {
    std::cout << "Array of size " << N << ": ";
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
}
```

4. **Test with different array types:**
```cpp
int main() {
    int nums[] = {1, 2, 3, 4, 5};
    const char str[] = "Hello";
    double values[] = {1.1, 2.2, 3.3};
    
    printArray(nums);
    printArray(str);
    printArray(values);
    
    return 0;
}
```

### Expected Learning Outcomes
- Understand when arrays decay to pointers
- Learn to preserve array size information
- See how to make type-safe array functions

### Bonus Challenge
Implement a function that creates a std::array from a C-style array using template deduction.

---

## Assignment 4: Function Template Parameter Matching (Medium)

### Objective
Practice matching argument types to template parameters.

### Task
Create overloaded template functions and understand which one gets called.

### Requirements

1. **Create three overloaded process functions:**
```cpp
// Version 1: By value
template<typename T>
void process(T value) {
    std::cout << "Called by-value version" << std::endl;
}

// Version 2: By lvalue reference
template<typename T>
void process(T& ref) {
    std::cout << "Called by-reference version" << std::endl;
}

// Version 3: By const lvalue reference
template<typename T>
void process(const T& cref) {
    std::cout << "Called by-const-reference version" << std::endl;
}
```

2. **Test which version is called:**
```cpp
int main() {
    int x = 10;
    const int cx = 20;
    
    process(x);         // Which version?
    process(cx);        // Which version?
    process(30);        // Which version?
    process(std::move(x)); // Which version?
    
    return 0;
}
```

3. **Implement a swap function using templates:**
```cpp
template<typename T>
void mySwap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

// Test it
int a = 5, b = 10;
mySwap(a, b);
std::cout << "a=" << a << ", b=" << b << std::endl;
```

4. **Create a generic max function:**
```cpp
template<typename T>
T max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

// Test with different types
auto maxInt = max(10, 20);
auto maxDouble = max(3.14, 2.71);
auto maxString = max(std::string("apple"), std::string("banana"));
```

### Expected Learning Outcomes
- Understand overload resolution with templates
- Learn when references are necessary
- Practice const-correctness

### Bonus Challenge
Add SFINAE to restrict max function to comparable types only.

---

## Assignment 5: Auto Return Type Deduction (Medium)

### Objective
Learn to use auto and decltype(auto) for return type deduction.

### Task
Implement functions with various return type deduction strategies.

### Requirements

1. **Simple auto return:**
```cpp
auto add(int a, int b) {
    return a + b;  // Returns int
}

auto multiply(double a, double b) {
    return a * b;  // Returns double
}

// What about this?
auto mixed(int a, double b) {
    return a + b;  // What type?
}
```

2. **Returning references with decltype(auto):**
```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// Version 1: Returns int (copy)
auto getElement1(int index) {
    return vec[index];
}

// Version 2: Returns int& (reference)
decltype(auto) getElement2(int index) {
    return vec[index];
}

// Test both versions
auto a = getElement1(0);
a = 100;  // Does this modify vec?

decltype(auto) b = getElement2(0);
b = 100;  // Does this modify vec?
```

3. **Create a generic getter:**
```cpp
template<typename Container, typename Index>
decltype(auto) get(Container&& c, Index i) {
    return std::forward<Container>(c)[i];
}

// Test it
std::vector<int> v = {10, 20, 30};
const std::vector<int> cv = {40, 50, 60};

auto x = get(v, 0);        // Returns int&
auto y = get(cv, 0);       // Returns const int&
auto z = get(std::vector{1, 2, 3}, 0);  // Returns int&&
```

4. **Lambda with auto return:**
```cpp
auto makeLambda() {
    return [](auto x, auto y) {
        return x + y;
    };
}

auto lambda = makeLambda();
auto result1 = lambda(5, 10);      // int + int
auto result2 = lambda(3.14, 2.71); // double + double
auto result3 = lambda(std::string("Hello"), std::string(" World"));
```

### Expected Learning Outcomes
- Understand auto vs decltype(auto)
- Learn when to use each return type deduction method
- Practice with generic lambdas

### Bonus Challenge
Implement a function that returns different types based on a condition using if constexpr.

---

## Assignment 6: Perfect Forwarding Basics (Medium)

### Objective
Learn the fundamentals of perfect forwarding with universal references.

### Task
Create wrapper functions that forward arguments without copying.

### Requirements

1. **Simple forwarding wrapper:**
```cpp
template<typename Func, typename Arg>
void callWith(Func func, Arg&& arg) {
    func(std::forward<Arg>(arg));
}

// Test function
void printValue(int& x) {
    std::cout << "Lvalue: " << x << std::endl;
}

void printValue(int&& x) {
    std::cout << "Rvalue: " << x << std::endl;
}

int main() {
    int x = 42;
    callWith(printValue, x);          // Forwards lvalue
    callWith(printValue, 100);        // Forwards rvalue
}
```

2. **Variadic forwarding:**
```cpp
template<typename Func, typename... Args>
auto callFunction(Func func, Args&&... args) {
    return func(std::forward<Args>(args)...);
}

// Test with multiple arguments
int sum(int a, int b, int c) {
    return a + b + c;
}

auto result = callFunction(sum, 1, 2, 3);
```

3. **Factory function with forwarding:**
```cpp
template<typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Test with a class
class Person {
public:
    Person(std::string name, int age) 
        : name_(std::move(name)), age_(age) {}
private:
    std::string name_;
    int age_;
};

auto person = makeUnique<Person>("Alice", 30);
```

4. **Measure move vs copy:**
```cpp
class MoveTracker {
public:
    MoveTracker() { std::cout << "Default constructor\n"; }
    MoveTracker(const MoveTracker&) { std::cout << "Copy constructor\n"; }
    MoveTracker(MoveTracker&&) { std::cout << "Move constructor\n"; }
};

template<typename T>
void withoutForward(T arg) {
    MoveTracker m = arg;  // What gets called?
}

template<typename T>
void withForward(T&& arg) {
    MoveTracker m = std::forward<T>(arg);  // What gets called?
}
```

### Expected Learning Outcomes
- Understand std::forward purpose
- Learn to preserve value categories
- See the performance benefits of perfect forwarding

### Bonus Challenge
Create a timer wrapper that measures execution time while perfectly forwarding arguments.

---

## Assignment 7: Type Deduction in Real Scenarios (Medium)

### Objective
Apply type deduction knowledge to solve practical problems.

### Task
Implement useful utilities using type deduction techniques.

### Requirements

1. **Generic print function:**
```cpp
#include <iostream>
#include <vector>
#include <list>

// Print any container
template<typename Container>
void print(const Container& container) {
    std::cout << "[ ";
    for (const auto& elem : container) {
        std::cout << elem << " ";
    }
    std::cout << "]" << std::endl;
}

// Test
std::vector<int> vec = {1, 2, 3};
std::list<double> lst = {1.1, 2.2, 3.3};
print(vec);
print(lst);
```

2. **Generic comparison function:**
```cpp
template<typename T>
auto compare(const T& a, const T& b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// Use it to sort
std::vector<int> numbers = {5, 2, 8, 1, 9};
std::sort(numbers.begin(), numbers.end(), 
    [](const auto& a, const auto& b) { return compare(a, b) < 0; });
```

3. **Generic accumulator:**
```cpp
template<typename Container>
auto sum(const Container& container) {
    using ValueType = typename Container::value_type;
    ValueType result = 0;
    for (const auto& elem : container) {
        result += elem;
    }
    return result;
}

// Or with auto:
template<typename Container>
auto sum2(const Container& container) {
    auto result = typename Container::value_type{};
    for (const auto& elem : container) {
        result += elem;
    }
    return result;
}
```

4. **Generic finder:**
```cpp
template<typename Container, typename Value>
auto find(const Container& container, const Value& value) {
    for (auto it = container.begin(); it != container.end(); ++it) {
        if (*it == value) {
            return it;
        }
    }
    return container.end();
}

// Test
std::vector<int> vec = {10, 20, 30, 40};
auto it = find(vec, 30);
if (it != vec.end()) {
    std::cout << "Found: " << *it << std::endl;
}
```

5. **Configuration helper:**
```cpp
class Config {
    std::map<std::string, std::string> data;
    
public:
    template<typename T>
    void set(const std::string& key, const T& value) {
        data[key] = std::to_string(value);
    }
    
    template<typename T>
    auto get(const std::string& key, const T& defaultValue) const {
        auto it = data.find(key);
        if (it != data.end()) {
            // Convert string back to T
            if constexpr (std::is_same_v<T, int>) {
                return std::stoi(it->second);
            } else if constexpr (std::is_same_v<T, double>) {
                return std::stod(it->second);
            }
        }
        return defaultValue;
    }
};
```

### Expected Learning Outcomes
- Apply type deduction to real problems
- Understand iterator types and auto
- Practice with generic programming

### Bonus Challenge
Add support for nested containers and custom types.

---

## Assignment 8: Debugging Type Deduction (Easy-Medium)

### Objective
Learn to debug and understand type deduction issues.

### Task
Find and fix type deduction bugs in provided code.

### Requirements

1. **Fix this broken code:**
```cpp
// Bug 1: Why doesn't this compile?
template<typename T>
void process(T value) {
    value = value * 2;
}

int main() {
    const int x = 10;
    process(x);
    std::cout << x << std::endl;  // Expected: 20
}
```

2. **Fix the reference issue:**
```cpp
// Bug 2: Why doesn't x change?
template<typename T>
void increment(T value) {
    value++;
}

int main() {
    int x = 10;
    increment(x);
    std::cout << x << std::endl;  // Expected: 11, Actual: 10
}
```

3. **Fix the initializer list problem:**
```cpp
// Bug 3: Why won't this compile?
template<typename T>
void printSize(T container) {
    std::cout << container.size() << std::endl;
}

int main() {
    printSize({1, 2, 3, 4, 5});  // ERROR!
}
```

4. **Fix the array size issue:**
```cpp
// Bug 4: Why is the size always wrong?
template<typename T>
void getSize(T arr) {
    std::cout << sizeof(arr) / sizeof(arr[0]) << std::endl;
}

int main() {
    int numbers[10] = {};
    getSize(numbers);  // Expected: 10, Actual: probably 1 or 2
}
```

### Expected Learning Outcomes
- Learn to identify common type deduction pitfalls
- Understand error messages related to type deduction
- Practice debugging template code

### Solution Guidelines
Each bug demonstrates a specific type deduction concept from Items 1 and 2.

---

## General Tips for All Assignments

### 1. Type Inspection Tools
```cpp
// Method 1: Use typeid (may show mangled names)
#include <typeinfo>
std::cout << typeid(variable).name() << std::endl;

// Method 2: Use compiler-specific macros
std::cout << __PRETTY_FUNCTION__ << std::endl;  // GCC/Clang
std::cout << __FUNCSIG__ << std::endl;          // MSVC

// Method 3: Force a compile error to see the type
template<typename T>
class ShowType;

ShowType<decltype(variable)> dummy;  // Compile error shows type
```

### 2. Compiler Flags
```bash
# Enable all warnings
g++ -std=c++17 -Wall -Wextra -Wpedantic your_file.cpp

# See template instantiations
clang++ -Xclang -ast-print -fsyntax-only your_file.cpp
```

### 3. Online Tools
- **Compiler Explorer:** https://godbolt.org
- **C++ Insights:** https://cppinsights.io
- **Quick Bench:** https://quick-bench.com (for performance)

### 4. Testing Strategy
- Start with simple cases
- Add const, reference variations
- Test with lvalues and rvalues
- Verify with static_assert when possible
- Check compiler output

---

## Submission Checklist

For each assignment:
- [ ] Code compiles without warnings
- [ ] All test cases pass
- [ ] Comments explain type deduction behavior
- [ ] Output is clear and formatted
- [ ] Edge cases are considered
- [ ] Code follows C++ best practices

---

## Learning Resources

- **Effective Modern C++** by Scott Meyers (Items 1 & 2)
- **cppreference.com** - Template argument deduction
- **isocpp.org** - Core Guidelines
- **YouTube:** CppCon talks on type deduction
- **Compiler documentation** for your specific compiler

---

## Next Steps

After completing these assignments:
1. Review the advanced assignments for deeper challenges
2. Practice with MCQs to test your knowledge
3. Read Items 3 and 4 of Effective Modern C++
4. Experiment with C++20 concepts for type constraints

Good luck and happy coding! 🎯
