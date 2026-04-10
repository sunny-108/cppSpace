# Item 3 (Part 2): Understand decltype - Advanced Topics

## 🚀 decltype(auto) - C++14

`decltype(auto)` is a powerful combination introduced in C++14 that says:
> "Use `auto` for convenience, but deduce the type using `decltype` rules (preserving references and const)."

### The Problem it Solves

```cpp
// Before C++14: Verbose and repetitive
template<typename Container, typename Index>
auto getElement(Container& c, Index i) -> decltype(c[i]) {
    return c[i];
}

// With C++14: Much cleaner!
template<typename Container, typename Index>
decltype(auto) getElement(Container& c, Index i) {
    return c[i];  // Returns exact type of c[i]
}
```

## 📖 Understanding decltype(auto)

### Example 1: Return Type Deduction

```cpp
#include <iostream>
#include <vector>

// Returns int& (reference to element)
decltype(auto) getElement(std::vector<int>& vec, size_t index) {
    return vec[index];  // decltype(vec[index]) is int&
}

// Returns int (copy of element)
auto getElementCopy(std::vector<int>& vec, size_t index) {
    return vec[index];  // auto strips reference, returns int
}

int main() {
    std::vector<int> vec = {10, 20, 30};
    
    // Using decltype(auto) - returns reference
    getElement(vec, 0) = 999;
    std::cout << "vec[0] = " << vec[0] << "\n";  // 999
    
    // Using auto - returns copy
    getElementCopy(vec, 1) = 888;  // Modifies temporary!
    std::cout << "vec[1] = " << vec[1] << "\n";  // 20 (unchanged)
    
    return 0;
}
```

### Example 2: Variable Declarations

```cpp
#include <iostream>

int getValue() { return 42; }
int& getReference() {
    static int x = 42;
    return x;
}

int main() {
    // auto version - strips reference
    auto x = getReference();  // x is int
    x = 100;  // Doesn't affect original
    
    // decltype(auto) version - preserves reference
    decltype(auto) y = getReference();  // y is int&
    y = 200;  // Modifies original
    
    std::cout << "Original value: " << getReference() << "\n";  // 200
    
    return 0;
}
```

### Example 3: Perfect Forwarding Return

```cpp
#include <iostream>
#include <string>

class Widget {
private:
    std::string name_;
    
public:
    Widget(std::string name) : name_(std::move(name)) {}
    
    const std::string& getName() const {
        return name_;
    }
    
    std::string& getName() {
        return name_;
    }
};

// Forward the exact return type from widget methods
template<typename T>
decltype(auto) callGetName(T&& widget) {
    return std::forward<T>(widget).getName();
}

int main() {
    Widget w("MyWidget");
    const Widget cw("ConstWidget");
    
    // Non-const widget - returns std::string&
    callGetName(w) = "NewName";
    std::cout << w.getName() << "\n";  // NewName
    
    // Const widget - returns const std::string&
    // callGetName(cw) = "Error";  // ERROR! Returns const reference
    std::cout << callGetName(cw) << "\n";  // ConstWidget
    
    return 0;
}
```

## 🔍 The Parentheses Rule (Critical!)

This is one of the most important and subtle aspects of `decltype`:

### Rule: Parentheses Change Everything!

```cpp
int x = 5;

decltype(x);    // int (x is a name)
decltype((x));  // int& (x in parentheses is an lvalue expression)
```

**Why?**
- `decltype(name)` → gives the declared type of the name
- `decltype(expression)` → gives the type of the expression, including value category

### Example 4: The Parentheses Trap

```cpp
#include <iostream>

int main() {
    int x = 10;
    
    // Without parentheses - gives type of name
    decltype(x) a = 20;     // a is int
    a = 30;                 // OK
    
    // With parentheses - treats as expression
    decltype((x)) b = x;    // b is int& (reference!)
    b = 40;
    
    std::cout << "x = " << x << "\n";   // 40 (modified via b!)
    std::cout << "a = " << a << "\n";   // 30
    std::cout << "b = " << b << "\n";   // 40
    
    return 0;
}
```

### Example 5: Dangerous decltype(auto) with Parentheses

```cpp
#include <iostream>

// DANGEROUS! Returns reference to local variable
decltype(auto) getBadReference() {
    int x = 42;
    return (x);  // Parentheses make it return int& - DANGLING REFERENCE!
}

// SAFE - Returns by value
decltype(auto) getGoodValue() {
    int x = 42;
    return x;   // No parentheses - returns int (copy)
}

int main() {
    auto val1 = getGoodValue();  // OK
    std::cout << "Good value: " << val1 << "\n";
    
    // auto val2 = getBadReference();  // UNDEFINED BEHAVIOR!
    // Dangling reference to destroyed local variable
    
    return 0;
}
```

## ⚠️ The Three Forms of decltype

### Form 1: decltype(name)

For an **unparenthesized variable name**, `decltype` gives the declared type:

```cpp
int x = 5;
const int cx = 10;
int& rx = x;

decltype(x);   // int
decltype(cx);  // const int
decltype(rx);  // int&
```

### Form 2: decltype(expression) - lvalue

For an **lvalue expression** that is not just a name, `decltype` gives `T&`:

```cpp
int x = 5;
int* p = &x;

decltype((x));   // int& (x is lvalue expression)
decltype(*p);    // int& (*p is lvalue expression)
decltype(x + 0); // int (x + 0 is rvalue expression)
```

### Form 3: decltype(expression) - rvalue

For an **rvalue expression**, `decltype` gives `T`:

```cpp
int getValue() { return 42; }

decltype(getValue());  // int (rvalue)
decltype(5);           // int (rvalue)
decltype(1 + 2);       // int (rvalue)
```

## 💻 Complete Real-World Example

### Example 6: Smart Container Wrapper

```cpp
#include <iostream>
#include <vector>
#include <string>

template<typename Container>
class SmartWrapper {
private:
    Container container_;
    
public:
    SmartWrapper(Container c) : container_(std::move(c)) {}
    
    // Perfect forwarding of operator[]
    // Returns exact type that container returns
    template<typename Index>
    decltype(auto) operator[](Index&& index) {
        std::cout << "Accessing element...\n";
        return container_[std::forward<Index>(index)];
    }
    
    // Const version
    template<typename Index>
    decltype(auto) operator[](Index&& index) const {
        std::cout << "Accessing element (const)...\n";
        return container_[std::forward<Index>(index)];
    }
    
    size_t size() const {
        return container_.size();
    }
};

int main() {
    SmartWrapper<std::vector<std::string>> wrapper(
        std::vector<std::string>{"Alice", "Bob", "Charlie"}
    );
    
    // Non-const access - returns std::string&
    wrapper[0] = "Alicia";
    std::cout << "Modified: " << wrapper[0] << "\n";
    
    // Const access
    const auto& constWrapper = wrapper;
    std::cout << "Const: " << constWrapper[1] << "\n";
    // constWrapper[1] = "Error";  // ERROR! Returns const reference
    
    return 0;
}
```

### Example 7: Function Result Cache

```cpp
#include <iostream>
#include <map>
#include <string>
#include <optional>

template<typename Func>
class ResultCache {
private:
    Func func_;
    mutable std::map<std::string, decltype(func_(std::string{}))> cache_;
    
public:
    ResultCache(Func f) : func_(std::move(f)) {}
    
    // Return exact type that func_ returns
    decltype(auto) operator()(const std::string& key) const {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            std::cout << "Cache hit for: " << key << "\n";
            return it->second;
        }
        
        std::cout << "Computing for: " << key << "\n";
        auto result = func_(key);
        cache_[key] = result;
        return result;
    }
};

int expensiveComputation(const std::string& input) {
    std::cout << "  [Expensive computation running...]\n";
    return input.length() * 100;
}

int main() {
    ResultCache cache(expensiveComputation);
    
    std::cout << "Result 1: " << cache("hello") << "\n\n";
    std::cout << "Result 2: " << cache("hello") << "\n\n";  // Cached!
    std::cout << "Result 3: " << cache("world") << "\n";
    
    return 0;
}
```

## 🎯 Advanced Use Cases

### Use Case 1: Generic Lambda Return Type

```cpp
#include <iostream>

int main() {
    // Lambda with decltype(auto) return
    auto lambda = [](auto& container, auto index) -> decltype(auto) {
        return container[index];
    };
    
    std::vector<int> vec = {10, 20, 30};
    
    // Returns reference, can modify
    lambda(vec, 0) = 999;
    std::cout << "vec[0] = " << vec[0] << "\n";  // 999
    
    return 0;
}
```

### Use Case 2: SFINAE with decltype

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

// Only enabled if Container has size() method
template<typename Container>
auto getSize(const Container& c) -> decltype(c.size()) {
    return c.size();
}

// Fallback for arrays
template<typename T, size_t N>
size_t getSize(const T (&arr)[N]) {
    return N;
}

int main() {
    std::vector<int> vec = {1, 2, 3};
    int arr[] = {1, 2, 3, 4, 5};
    
    std::cout << "Vector size: " << getSize(vec) << "\n";  // 3
    std::cout << "Array size: " << getSize(arr) << "\n";   // 5
    
    return 0;
}
```

### Use Case 3: Deducing Member Types

```cpp
#include <iostream>
#include <map>
#include <string>

template<typename Map>
void printFirstKey(const Map& m) {
    if (!m.empty()) {
        // Deduce the key type
        decltype(m.begin()->first) firstKey = m.begin()->first;
        std::cout << "First key: " << firstKey << "\n";
    }
}

int main() {
    std::map<std::string, int> ages = {
        {"Alice", 30},
        {"Bob", 25}
    };
    
    printFirstKey(ages);
    
    return 0;
}
```

## 📊 Decision Guide

### When to Use What?

```
Need type deduction?
├─ Want exact type preservation?
│  ├─ For return type? → decltype(auto)
│  └─ For variable? → decltype(expr)
│
├─ Want convenience, don't care about exact type?
│  └─ Use auto (or auto&, const auto&, etc.)
│
└─ Know the type already?
   └─ Just write it explicitly
```

### Quick Reference Table

| Situation | Use | Example |
|-----------|-----|---------|
| Return exact type from function | `decltype(auto)` | `decltype(auto) f() { return expr; }` |
| Variable with exact type of expr | `decltype(expr)` | `decltype(vec[0]) elem = vec[0];` |
| Local variable, want copy | `auto` | `auto x = getValue();` |
| Local variable, want reference | `auto&` or `decltype(auto)` | `auto& x = vec[0];` |
| Trailing return type | `-> decltype(expr)` | `auto f() -> decltype(expr) { }` |

## ⚠️ Common Pitfalls

### Pitfall 1: Forgetting Parentheses Matter

```cpp
int x = 5;

auto func1() {
    return x;   // Returns int
}

auto func2() {
    return (x); // Still returns int (auto doesn't care about parens)
}

decltype(auto) func3() {
    return x;   // Returns int
}

decltype(auto) func4() {
    return (x); // Returns int& - DANGLING REFERENCE!
}
```

### Pitfall 2: Accidental Reference

```cpp
Widget makeWidget() {
    return Widget();
}

decltype(auto) w1 = makeWidget();  // OK: w1 is Widget
decltype(auto) w2 = (makeWidget()); // ERROR: Can't bind rvalue to non-const lvalue ref
```

### Pitfall 3: Const Confusion

```cpp
const Widget cw;

decltype(auto) w1 = cw;  // w1 is const Widget (copy)
decltype((cw)) w2 = cw;  // w2 is const Widget& (reference)
```

## 🎓 Best Practices

### ✅ DO:

```cpp
// 1. Use decltype(auto) for return types that forward references
template<typename T>
decltype(auto) forward_call(T&& t) {
    return std::forward<T>(t).method();
}

// 2. Use decltype for variable declarations when exact type needed
decltype(container[0]) elem = container[0];

// 3. Be explicit about parentheses
decltype(auto) func() {
    return x;  // Clear intention: return by value
}

// 4. Use trailing return type when needed
template<typename T, typename U>
auto add(T t, U u) -> decltype(t + u) {
    return t + u;
}
```

### ❌ DON'T:

```cpp
// 1. Don't accidentally add parentheses in return
decltype(auto) bad() {
    int x = 5;
    return (x);  // BAD! Returns dangling reference
}

// 2. Don't use decltype(auto) when auto is sufficient
decltype(auto) x = 5;  // Overkill, just use: auto x = 5;

// 3. Don't forget decltype sees through references
std::vector<int> vec;
decltype(vec[0]) elem;  // ERROR! int& must be initialized
```

## 💡 Summary (Part 2)

1. **`decltype(auto)`** = auto convenience + decltype preservation
2. **Parentheses matter**: `decltype(x)` ≠ `decltype((x))`
3. **Three forms**: name, lvalue expression, rvalue expression
4. **Perfect for forwarding** return types while preserving references
5. **Be careful** with parentheses in return statements
6. **Use in templates** when you need exact type preservation
7. **Prefer auto** for simple cases, decltype for exact type needs

## 🔍 Key Differences Summary

| Feature | auto | decltype | decltype(auto) |
|---------|------|----------|----------------|
| Strips references | ✅ Yes | ❌ No | ❌ No |
| Strips const | ✅ Yes | ❌ No | ❌ No |
| Convenient | ✅ Very | ❌ Verbose | ✅ Yes |
| Exact type | ❌ No | ✅ Yes | ✅ Yes |
| Parentheses matter | ❌ No | ✅ Yes | ✅ Yes |
| Since | C++11 | C++11 | C++14 |

## 🎯 Real-World Recommendation

```cpp
// Most of the time: use auto
auto x = getValue();

// When forwarding references: use decltype(auto)
decltype(auto) forward_get() {
    return container[0];
}

// When you need exact type in variable: use decltype
decltype(container[0]) ref = container[0];

// When you know the type: write it explicitly
std::string name = getName();
```

## 🔑 Final Thoughts

> **"Use `auto` by default. Use `decltype(auto)` when you need to preserve references. Use `decltype(expr)` when you need the exact type of an expression."**

The evolution:
- **C++98/03**: Write types explicitly (verbose but clear)
- **C++11**: `auto` for convenience, `decltype` for exactness
- **C++14**: `decltype(auto)` for best of both worlds

Modern C++ gives you the tools - choose based on your needs:
- **Clarity** → Explicit types
- **Convenience** → `auto`
- **Precision** → `decltype` or `decltype(auto)`

