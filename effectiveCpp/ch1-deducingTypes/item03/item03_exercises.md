# Item 3 Exercises: MCQs and Practical Tasks

## 📝 Multiple Choice Questions (15 Questions)

### Question 1
What is the primary difference between `auto` and `decltype`?

A) `auto` is faster than `decltype`  
B) `auto` follows template type deduction rules, `decltype` preserves exact types  
C) `decltype` only works with functions  
D) There is no difference  

**Answer: B**  
**Explanation:** `auto` uses template type deduction rules (may strip const and references), while `decltype` preserves the exact declared type including all qualifiers.

---

### Question 2
What type does `decltype(x)` return if `x` is declared as `const int& x`?

A) `int`  
B) `int&`  
C) `const int`  
D) `const int&`  

**Answer: D**  
**Explanation:** `decltype` preserves the exact declared type, including const qualifiers and references.

---

### Question 3
What is the difference between `decltype(x)` and `decltype((x))`?

A) No difference  
B) `decltype(x)` gives declared type, `decltype((x))` gives lvalue reference  
C) `decltype((x))` is a syntax error  
D) They are the same only for integers  

**Answer: B**  
**Explanation:** For a variable `x`, `decltype(x)` gives the declared type, but `decltype((x))` treats `x` as an lvalue expression and returns an lvalue reference.

---

### Question 4
When was `decltype(auto)` introduced in C++?

A) C++11  
B) C++14  
C) C++17  
D) C++20  

**Answer: B**  
**Explanation:** `decltype(auto)` was introduced in C++14 to combine the convenience of `auto` with the type preservation of `decltype`.

---

### Question 5
What does this code return?

```cpp
int x = 5;
decltype(auto) func() {
    return (x);
}
```

A) `int` (value)  
B) `int&` (reference to x)  
C) `const int&`  
D) Compilation error  

**Answer: B**  
**Explanation:** The parentheses around `x` make it an lvalue expression, so `decltype(auto)` returns `int&`. This creates a dangling reference if `x` is local!

---

### Question 6
What type is `elem` in this code?

```cpp
std::vector<int> vec = {1, 2, 3};
decltype(vec[0]) elem = vec[0];
```

A) `int`  
B) `int&`  
C) `const int&`  
D) `int&&`  

**Answer: B**  
**Explanation:** `std::vector<int>::operator[]` returns `int&`, so `decltype(vec[0])` is `int&`.

---

### Question 7
Which statement is TRUE about `decltype(auto)`?

A) It always strips references  
B) It preserves references like `decltype`  
C) It's slower than regular `auto`  
D) It can only be used with functions  

**Answer: B**  
**Explanation:** `decltype(auto)` uses `decltype` rules for type deduction, preserving references and const qualifiers.

---

### Question 8
What is the primary use case for `decltype` in template code?

A) To make code run faster  
B) To preserve exact return types including references  
C) To create new types  
D) To replace auto completely  

**Answer: B**  
**Explanation:** In template code, `decltype` is primarily used to preserve exact types, especially return types with references, when the actual type is not known at template definition time.

---

### Question 9
What happens in this code?

```cpp
auto func1() {
    int x = 42;
    return (x);
}

decltype(auto) func2() {
    int x = 42;
    return (x);
}
```

A) Both return `int`  
B) Both return `int&`  
C) `func1` returns `int`, `func2` returns dangling `int&`  
D) Both cause compilation errors  

**Answer: C**  
**Explanation:** `auto` ignores parentheses and returns `int` by value. `decltype(auto)` with `(x)` treats it as an lvalue expression, returning `int&` - a dangling reference to local variable!

---

### Question 10
For trailing return types, which syntax is correct?

A) `auto func() -> decltype(expr) { return expr; }`  
B) `decltype(expr) auto func() { return expr; }`  
C) `decltype auto func() -> expr { return expr; }`  
D) `auto -> decltype(expr) func() { return expr; }`  

**Answer: A**  
**Explanation:** Trailing return type syntax uses `auto` before the function name, followed by `->` and the return type after parameters.

---

### Question 11
What does `decltype` return for function calls?

A) Always `void`  
B) The return type of the function  
C) A reference to the function  
D) The parameter types  

**Answer: B**  
**Explanation:** For function calls, `decltype(func())` returns the return type of the function, including any references or const qualifiers.

---

### Question 12
In which scenario is `decltype(auto)` most useful?

A) Declaring local integer variables  
B) Perfect forwarding of return types in generic code  
C) Optimizing performance  
D) Replacing all uses of auto  

**Answer: B**  
**Explanation:** `decltype(auto)` is most useful in generic code where you want to forward the exact return type (including references) from another function or expression.

---

### Question 13
What type is `y` in this code?

```cpp
const int x = 10;
decltype(x) y = 20;
```

A) `int`  
B) `const int`  
C) `int&`  
D) `const int&`  

**Answer: B**  
**Explanation:** `decltype(x)` preserves the exact declared type of `x`, which is `const int`.

---

### Question 14
Which is a valid reason to use `decltype` over `auto`?

A) Better performance  
B) Need to preserve references and const qualifiers  
C) Shorter syntax  
D) Required by the compiler  

**Answer: B**  
**Explanation:** Use `decltype` when you need the exact type with all qualifiers preserved, unlike `auto` which follows template deduction rules.

---

### Question 15
What is the result type of this expression?

```cpp
int a = 5, b = 10;
decltype(a + b) result = a + b;
```

A) `int&`  
B) `const int`  
C) `int`  
D) `int&&`  

**Answer: C**  
**Explanation:** `a + b` is an rvalue expression of type `int`, so `decltype(a + b)` is `int`.

---

## 💻 Practical Exercises

### Exercise 1: Code Review - Find the Bugs 🐛

**Difficulty:** Medium  
**Objective:** Identify type-related bugs and dangerous patterns in code using `decltype` and `decltype(auto)`.

```cpp
#include <iostream>
#include <vector>
#include <string>

// Bug 1: Dangling reference
decltype(auto) getBadValue() {
    int x = 42;
    return (x);  // Returns int&, but x is destroyed!
}

// Bug 2: Unintended copy
template<typename Container>
auto getFirstElement(Container& c) {
    return c[0];  // Returns copy, not reference
}

// Bug 3: Missing const preservation
class DataHolder {
private:
    std::vector<int> data_;
    
public:
    DataHolder() : data_{1, 2, 3, 4, 5} {}
    
    // Should preserve const when called on const object
    auto getData(size_t index) {
        return data_[index];  // Always returns int, even for const object
    }
    
    auto getData(size_t index) const {
        return data_[index];  // Returns int, loses const
    }
};

// Bug 4: Incorrect decltype usage
template<typename T>
class Wrapper {
private:
    T value_;
    
public:
    Wrapper(T v) : value_(std::move(v)) {}
    
    // Bug: decltype(value_) may not be what you expect
    decltype(value_) get() {
        return value_;  // May return reference or value depending on T
    }
};

// Bug 5: Parentheses problem
class Point {
public:
    int x, y;
    
    Point(int x_, int y_) : x(x_), y(y_) {}
    
    // Dangerous: returns reference to temporary
    decltype(auto) getX() {
        return (x);  // Returns int&, but what if Point is temporary?
    }
};

// Bug 6: Type mismatch
void processData() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // Bug: elem is int&, but should it be?
    decltype(vec[0]) elem = vec[0];
    
    // Now vec is moved, elem is dangling!
    std::vector<int> vec2 = std::move(vec);
}

// Bug 7: Reference to rvalue
auto getTemporary() {
    return std::vector<int>{1, 2, 3};
}

decltype(auto) getElement() {
    return getTemporary()[0];  // Dangling reference to temporary!
}

// Bug 8: Const confusion
template<typename T>
class Container {
private:
    std::vector<T> items_;
    
public:
    void add(T item) {
        items_.push_back(std::move(item));
    }
    
    // Bug: Should return const T& for const Container
    decltype(auto) operator[](size_t index) {
        return items_[index];
    }
    
    // Missing const version!
};

// Bug 9: Perfect forwarding broken
template<typename Func>
decltype(auto) callFunction(Func& func) {  // Bug: should use Func&&
    return func();
}

// Bug 10: Initialization problem
void initializationBug() {
    std::vector<std::string> names = {"Alice", "Bob"};
    
    // Bug: Reference must be initialized
    decltype(names[0]) name;  // Error! int& must be initialized
    
    name = names[0];  // Too late!
}

int main() {
    // Test Bug 1
    // auto val = getBadValue();  // Dangling reference!
    // std::cout << val << "\n";  // Undefined behavior
    
    // Test Bug 2
    std::vector<int> vec = {10, 20, 30};
    auto elem = getFirstElement(vec);
    elem = 999;
    std::cout << "vec[0] = " << vec[0] << "\n";  // Still 10, not 999!
    
    // Test Bug 3
    const DataHolder holder;
    auto data = holder.getData(0);
    // data is int, not const int&
    
    // Test Bug 5
    Point{1, 2}.getX();  // Returns reference to temporary's member!
    
    return 0;
}
```

**Your Task:**
1. **Identify all 10+ bugs** in the code above
2. **Explain why each is a bug** (dangling reference, unintended copy, etc.)
3. **Provide corrected code** for each issue
4. **Add test cases** to verify the fixes

**Specific Issues to Address:**
- Dangling references from local variables
- Parentheses creating unexpected references
- Missing const correctness
- Unintended copies vs references
- Perfect forwarding issues
- Uninitialized references

---

### Exercise 2: Debugging - Type Deduction Detective 🔍

**Difficulty:** Medium-Advanced  
**Objective:** Debug complex type deduction issues in template code.

```cpp
#include <iostream>
#include <vector>
#include <type_traits>
#include <string>

// Problem 1: Why doesn't this compile?
template<typename Container>
auto processContainer(Container& c) -> decltype(c[0]) {
    std::cout << "Processing...\n";
    return c[0];  // Trying to return reference to first element
}

// Problem 2: Type mismatch
template<typename T>
class SmartPointer {
private:
    T* ptr_;
    
public:
    SmartPointer(T* p) : ptr_(p) {}
    
    // Want to return T& but getting wrong type
    decltype(*ptr_) get() {  // What's the problem here?
        return *ptr_;
    }
};

// Problem 3: Perfect forwarding failure
template<typename Func>
decltype(auto) callTwice(Func f) {
    f();
    return f();  // Second call works, but type might be wrong
}

// Problem 4: Conditional return type
template<typename T>
auto getValue(T& value, bool copy) {
    if (copy) {
        return value;  // Returns by value
    } else {
        return (value);  // Tries to return by reference - ERROR!
    }
    // Error: inconsistent deduction for auto return type
}

// Problem 5: Lambda return type
auto createLambda() {
    int x = 42;
    return [&]() -> decltype(auto) {
        return (x);  // Returns reference to local variable!
    };
}

// Problem 6: Template deduction issue
template<typename T>
void print(T value) {
    std::cout << "Value: " << value << "\n";
}

template<typename Container>
void printFirst(Container& c) {
    decltype(c[0]) first = c[0];  // Is this a reference?
    print(first);  // Does this copy or pass reference?
    first = 999;  // Does this modify container?
}

// Problem 7: Const propagation
template<typename T>
class Holder {
private:
    T value_;
    
public:
    Holder(T v) : value_(std::move(v)) {}
    
    decltype(auto) get() {
        return value_;
    }
    
    decltype(auto) get() const {
        return value_;
    }
    
    // Problem: What if someone does this?
    // auto val = constHolder.get();
    // val.modify();  // Should this compile?
};

// Problem 8: Deducing member function return type
struct Widget {
    std::string& getName();
    const std::string& getName() const;
};

template<typename T>
auto callGetName(T& widget) {
    return widget.getName();  // Which overload? What's the return type?
}

// Problem 9: Array decay
template<typename T>
decltype(auto) getElement(T& arr, size_t index) {
    return arr[index];
}

void testArray() {
    int arr[] = {1, 2, 3};
    auto elem = getElement(arr, 0);
    elem = 999;
    // Did arr[0] change?
}

// Problem 10: Reference collapsing confusion
template<typename T>
void process(T&& param) {
    decltype(param) local = param;  // What type is local?
    // If T is int&, what is decltype(param)?
}

int main() {
    // Debug each problem:
    
    // Problem 1:
    std::vector<int> vec = {1, 2, 3};
    // auto& first = processContainer(vec);  // Does this compile?
    
    // Problem 2:
    int x = 42;
    SmartPointer<int> sp(&x);
    // auto& val = sp.get();  // What's the issue?
    
    // Problem 3:
    int count = 0;
    auto result = callTwice([&count]() { return ++count; });
    std::cout << "Result: " << result << "\n";
    
    // Problem 4:
    int value = 42;
    // auto result = getValue(value, false);  // Doesn't compile!
    
    // Problem 5:
    auto lambda = createLambda();
    // lambda();  // Dangling reference!
    
    // Problem 6:
    std::vector<int> vec2 = {10, 20, 30};
    printFirst(vec2);
    std::cout << "vec2[0] = " << vec2[0] << "\n";  // Is it 999?
    
    return 0;
}
```

**Your Task:**

1. **Identify the problem** in each code section (1-10)
2. **Explain why the code doesn't work** or produces unexpected results
3. **Fix each problem** with proper use of `decltype`/`decltype(auto)`
4. **Add type checking** using `std::is_same` or similar to verify fixes
5. **Write tests** to demonstrate correct behavior

**Example Fix:**

```cpp
// Problem 1 - Fixed:
template<typename Container>
decltype(auto) processContainer(Container& c) {
    // decltype(auto) preserves reference from c[0]
    return c[0];
}

// Verification:
std::vector<int> vec = {1, 2, 3};
processContainer(vec) = 999;
assert(vec[0] == 999);  // Now it works!
```

---

### Exercise 3: Implementation from Scratch - Type-Safe Wrapper 🔨

**Difficulty:** Advanced  
**Objective:** Implement a type-safe property wrapper that uses `decltype` to preserve exact types.

```cpp
#include <iostream>
#include <functional>
#include <optional>
#include <string>

// Your task: Complete this implementation

template<typename T>
class Property {
private:
    T value_;
    std::function<void(const T&)> onChange_;
    std::optional<std::function<bool(const T&)>> validator_;
    
public:
    // Constructor
    explicit Property(T initial) : value_(std::move(initial)) {}
    
    // TODO: Implement getter that returns exact type
    // Should return T& for non-const, const T& for const
    decltype(auto) get() {
        // TODO: Return value_ with correct type
    }
    
    decltype(auto) get() const {
        // TODO: Return value_ with correct type (const)
    }
    
    // TODO: Implement setter with perfect forwarding
    template<typename U>
    void set(U&& newValue) {
        // TODO:
        // 1. Validate if validator exists
        // 2. Update value_
        // 3. Call onChange_ if set
    }
    
    // TODO: Implement change listener
    void setOnChange(std::function<void(const T&)> callback) {
        onChange_ = std::move(callback);
    }
    
    // TODO: Implement validator
    void setValidator(std::function<bool(const T&)> validator) {
        validator_ = std::move(validator);
    }
    
    // TODO: Implement conversion operator
    operator decltype(auto)() {
        // Should return reference to value_
    }
    
    // TODO: Implement assignment operator
    template<typename U>
    Property& operator=(U&& value) {
        set(std::forward<U>(value));
        return *this;
    }
};

// TODO: Implement PropertyRef - a reference to another Property
template<typename T>
class PropertyRef {
private:
    // TODO: Store reference to original property
    // Hint: Use decltype to deduce correct type
    
public:
    // TODO: Constructor taking Property
    
    // TODO: Forward all operations to original property
    decltype(auto) get() {
        // TODO
    }
    
    decltype(auto) get() const {
        // TODO
    }
    
    template<typename U>
    void set(U&& value) {
        // TODO
    }
};

// TODO: Implement make_property helper
template<typename T>
auto make_property(T&& value) {
    // TODO: Create Property with perfect forwarding
}

// TODO: Implement computed property that depends on other properties
template<typename Func>
class ComputedProperty {
private:
    Func func_;
    
public:
    explicit ComputedProperty(Func f) : func_(std::move(f)) {}
    
    // TODO: Implement get() that calls func_ and returns result
    decltype(auto) get() const {
        // TODO: Return result of func_() with correct type
    }
    
    // TODO: Implement auto-conversion
    operator decltype(auto)() const {
        return get();
    }
};

// TODO: Helper to create computed property
template<typename Func>
auto make_computed(Func&& func) {
    // TODO
}

// Test your implementation
int main() {
    // Test 1: Basic property
    Property<int> age(25);
    std::cout << "Initial age: " << age.get() << "\n";
    
    age.set(26);
    std::cout << "Updated age: " << age.get() << "\n";
    
    // Test 2: Change listener
    age.setOnChange([](const int& newAge) {
        std::cout << "Age changed to: " << newAge << "\n";
    });
    
    age.set(27);  // Should trigger callback
    
    // Test 3: Validator
    age.setValidator([](const int& value) {
        return value >= 0 && value <= 150;
    });
    
    try {
        age.set(200);  // Should fail validation
    } catch (const std::exception& e) {
        std::cout << "Validation failed: " << e.what() << "\n";
    }
    
    // Test 4: Reference modification
    age.get() = 28;  // Should modify through reference
    std::cout << "After reference modification: " << age.get() << "\n";
    
    // Test 5: PropertyRef
    PropertyRef<int> ageRef(age);
    ageRef.set(29);
    std::cout << "Age through ref: " << age.get() << "\n";
    
    // Test 6: Computed property
    Property<int> width(10);
    Property<int> height(20);
    
    auto area = make_computed([&]() {
        return width.get() * height.get();
    });
    
    std::cout << "Area: " << area.get() << "\n";
    
    width.set(15);
    std::cout << "Updated area: " << area.get() << "\n";
    
    // Test 7: String property
    Property<std::string> name("Alice");
    name.setValidator([](const std::string& s) {
        return !s.empty() && s.length() <= 50;
    });
    
    name.set("Bob");
    std::cout << "Name: " << name.get() << "\n";
    
    return 0;
}
```

**Requirements:**

1. **Use `decltype(auto)` correctly** for getters
2. **Perfect forwarding** in setters
3. **Const correctness** - const methods return const references
4. **Validator support** with error handling
5. **Change listeners** that trigger on modifications
6. **PropertyRef** that references another property
7. **Computed properties** that derive from others
8. **Type safety** - preserve exact types throughout

**Bonus Challenges:**
- Add property bindings (two properties stay in sync)
- Add lazy evaluation for computed properties
- Add property history/undo functionality
- Thread-safe property access
- Property serialization

---

### Exercise 4: Performance Optimization - Minimize Copies ⚡

**Difficulty:** Advanced  
**Objective:** Optimize code that uses `auto` and `decltype` to minimize unnecessary copies.

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <map>

// Inefficient code - optimize this!

class HeavyObject {
private:
    std::vector<int> data_;
    std::string name_;
    
public:
    HeavyObject(std::string name, size_t size) 
        : data_(size, 42), name_(std::move(name)) {
        std::cout << "HeavyObject '" << name_ << "' created\n";
    }
    
    HeavyObject(const HeavyObject& other) 
        : data_(other.data_), name_(other.name_) {
        std::cout << "HeavyObject '" << name_ << "' COPIED\n";
    }
    
    const std::string& getName() const { return name_; }
    size_t getSize() const { return data_.size(); }
};

class Container {
private:
    std::vector<HeavyObject> objects_;
    
public:
    void add(HeavyObject obj) {
        objects_.push_back(std::move(obj));
    }
    
    // Issue 1: Returns by value, causes copy
    HeavyObject getFirst() const {
        return objects_[0];
    }
    
    // Issue 2: Returns by value for operator[]
    HeavyObject operator[](size_t index) const {
        return objects_[index];
    }
    
    size_t size() const { return objects_.size(); }
};

// Issue 3: auto causes unnecessary copies
void processObjects(const Container& container) {
    for (size_t i = 0; i < container.size(); ++i) {
        auto obj = container[i];  // COPY!
        std::cout << "Processing: " << obj.getName() << "\n";
    }
}

// Issue 4: Incorrect decltype usage
template<typename Container>
void printFirst(const Container& c) {
    decltype(c[0]) first = c[0];  // Type might cause copy
    std::cout << "First: " << first.getName() << "\n";
}

// Issue 5: Range-based for with auto
void iterateObjects(const Container& container) {
    // Makes copies on each iteration!
    for (auto obj : container) {
        std::cout << "Name: " << obj.getName() << "\n";
    }
}

// Issue 6: Returning from function
auto findObject(const Container& container, const std::string& name) {
    for (size_t i = 0; i < container.size(); ++i) {
        auto obj = container[i];  // COPY!
        if (obj.getName() == name) {
            return obj;  // Another COPY!
        }
    }
    return HeavyObject("NotFound", 0);
}

// Issue 7: Map iteration
using ObjectMap = std::map<std::string, HeavyObject>;

void printMap(const ObjectMap& map) {
    for (auto pair : map) {  // COPIES both key and value!
        std::cout << pair.first << ": " << pair.second.getName() << "\n";
    }
}

// Issue 8: Chained calls
class Builder {
private:
    std::vector<HeavyObject> objects_;
    
public:
    auto addObject(HeavyObject obj) {
        objects_.push_back(std::move(obj));
        return *this;  // Returns by value - COPY!
    }
    
    auto getObjects() {
        return objects_;  // Returns by value - COPY!
    }
};

// Issue 9: Lambda captures
auto createProcessor(Container container) {  // COPY!
    return [container]() {  // Another COPY!
        for (auto obj : container) {  // Yet another COPY per iteration!
            std::cout << obj.getName() << "\n";
        }
    };
}

// Issue 10: Unnecessary materialization
template<typename Func>
void applyToFirst(const Container& container, Func func) {
    auto first = container.getFirst();  // COPY!
    func(first);
}

// Performance benchmark
void benchmark() {
    auto start = std::chrono::high_resolution_clock::now();
    
    Container container;
    
    // Add objects
    for (int i = 0; i < 100; ++i) {
        container.add(HeavyObject("Object" + std::to_string(i), 10000));
    }
    
    // Process (inefficiently)
    processObjects(container);
    
    // Iterate (inefficiently)
    iterateObjects(container);
    
    // Search (inefficiently)
    auto found = findObject(container, "Object50");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "\nTime: " << duration << "ms\n";
    std::cout << "Optimization needed!\n";
}

int main() {
    benchmark();
    return 0;
}
```

**Your Task:**

1. **Profile the code** and identify all copy operations
2. **Optimize each issue** (1-10) using proper `decltype`, `auto&`, `const auto&`

**Optimizations to implement:**

**Optimization 1: Return by const reference**
```cpp
// Before
HeavyObject getFirst() const {
    return objects_[0];
}

// After
const HeavyObject& getFirst() const {
    return objects_[0];
}

// Or with decltype
decltype(auto) getFirst() const {
    return (objects_[0]);  // Returns const HeavyObject&
}
```

**Optimization 2: Reference in operator[]**
```cpp
const HeavyObject& operator[](size_t index) const {
    return objects_[index];
}
```

**Optimization 3: Use const auto&**
```cpp
void processObjects(const Container& container) {
    for (size_t i = 0; i < container.size(); ++i) {
        const auto& obj = container[i];  // Reference!
        std::cout << "Processing: " << obj.getName() << "\n";
    }
}
```

**Optimization 4: decltype(auto) for exact type**
```cpp
template<typename Container>
void printFirst(const Container& c) {
    decltype(auto) first = c[0];  // Preserves reference
    std::cout << "First: " << first.getName() << "\n";
}
```

**Optimization 5: Range-based for with const auto&**
```cpp
for (const auto& obj : container) {
    std::cout << "Name: " << obj.getName() << "\n";
}
```

**Optimization 6: Return optional or index**
```cpp
std::optional<size_t> findObject(const Container& container, 
                                 const std::string& name) {
    for (size_t i = 0; i < container.size(); ++i) {
        if (container[i].getName() == name) {
            return i;  // Return index instead of object
        }
    }
    return std::nullopt;
}
```

**Optimization 7: const auto& in range-for**
```cpp
for (const auto& [key, value] : map) {  // Structured binding by reference
    std::cout << key << ": " << value.getName() << "\n";
}
```

**Optimization 8: Return reference**
```cpp
Builder& addObject(HeavyObject obj) {
    objects_.push_back(std::move(obj));
    return *this;  // Returns reference
}

const std::vector<HeavyObject>& getObjects() const {
    return objects_;  // Returns reference
}
```

**Optimization 9: Lambda capture by reference**
```cpp
auto createProcessor(const Container& container) {
    return [&container]() {  // Capture by reference
        for (const auto& obj : container) {
            std::cout << obj.getName() << "\n";
        }
    };
}
```

**Optimization 10: Direct function call**
```cpp
template<typename Func>
void applyToFirst(const Container& container, Func func) {
    func(container.getFirst());  // No local variable needed
}
```

3. **Measure improvement** - show before/after timings
4. **Count copies** - add logging to verify copies eliminated
5. **Document results**:
   - Original: X copies, Y ms
   - Optimized: Z copies, W ms
   - Speedup: X/Z times fewer copies, Y/W times faster

**Expected Results:**
- Original: ~300+ copies
- Optimized: ~100 copies (only necessary ones)
- Speedup: 3-5x depending on object size

---

### Exercise 5: Type Traits and decltype Integration 🎨

**Difficulty:** Advanced  
**Objective:** Implement type traits and SFINAE using `decltype` for compile-time type checking.

```cpp
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>

// Your task: Complete these type trait implementations

// 1. has_size_method - Check if type has size() method
template<typename T, typename = void>
struct has_size_method : std::false_type {};

template<typename T>
struct has_size_method<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

// TODO: Implement has_push_back
template<typename T, typename = void>
struct has_push_back : std::false_type {};

// Your implementation here

// TODO: Implement has_iterator
template<typename T, typename = void>
struct has_iterator : std::false_type {};

// Your implementation here

// TODO: Implement result_of_index - Get type of T[Index]
template<typename T, typename Index>
struct result_of_index {
    // Hint: Use decltype(std::declval<T>()[std::declval<Index>()])
    using type = /* TODO */;
};

template<typename T, typename Index>
using result_of_index_t = typename result_of_index<T, Index>::type;

// TODO: Implement is_reference_wrapper
template<typename T>
struct is_reference_wrapper : std::false_type {};

template<typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

// TODO: Implement function that only works with containers having size()
template<typename Container>
auto getSize(const Container& c) 
    -> std::enable_if_t<has_size_method<Container>::value, size_t> {
    return c.size();
}

// TODO: Overload for arrays
template<typename T, size_t N>
size_t getSize(const T (&arr)[N]) {
    return N;
}

// TODO: Implement safe_index that returns optional
template<typename Container, typename Index>
auto safe_index(Container& c, Index idx) 
    -> std::optional<std::reference_wrapper<
        std::remove_reference_t<decltype(c[idx])>>> {
    // TODO: Check bounds and return optional reference
}

// TODO: Implement compile-time type checker
template<typename T>
void checkType() {
    std::cout << "Type analysis for: " << typeid(T).name() << "\n";
    
    if constexpr (has_size_method<T>::value) {
        std::cout << "  Has size() method\n";
    }
    
    if constexpr (has_push_back<T>::value) {
        std::cout << "  Has push_back() method\n";
    }
    
    if constexpr (has_iterator<T>::value) {
        std::cout << "  Has iterators\n";
    }
}

// TODO: Implement perfect_forward that preserves exact types
template<typename Func, typename... Args>
decltype(auto) perfect_forward(Func&& func, Args&&... args) {
    // TODO: Forward to func with perfect forwarding
}

int main() {
    // Test has_size_method
    static_assert(has_size_method<std::vector<int>>::value);
    static_assert(!has_size_method<int>::value);
    
    // Test has_push_back
    static_assert(has_push_back<std::vector<int>>::value);
    static_assert(!has_push_back<int>::value);
    
    // Test has_iterator
    static_assert(has_iterator<std::vector<int>>::value);
    
    // Test result_of_index
    using VecIndexResult = result_of_index_t<std::vector<int>, size_t>;
    static_assert(std::is_same_v<VecIndexResult, int&>);
    
    // Test getSize
    std::vector<int> vec = {1, 2, 3};
    int arr[] = {1, 2, 3, 4};
    
    std::cout << "Vector size: " << getSize(vec) << "\n";
    std::cout << "Array size: " << getSize(arr) << "\n";
    
    // Test checkType
    checkType<std::vector<int>>();
    checkType<std::string>();
    
    return 0;
}
```

**Requirements:**

1. **Complete all TODO sections**
2. **Use `decltype` with `std::declval`** for type checking
3. **Implement SFINAE** with `std::enable_if_t`
4. **Add static_asserts** to verify traits
5. **Test with various types** (STL containers, arrays, custom types)

**Bonus Challenges:**
- Implement `is_container` trait
- Create `is_callable_with` trait
- Implement return type deduction for generic functions
- Add support for member function detection

---

## 🎯 Solutions Checklist

For each exercise, ensure:

- [ ] **Compiles without warnings** (`-Wall -Wextra -Werror`)
- [ ] **Correct use of decltype** (preserves references and const)
- [ ] **No dangling references** (check with sanitizers)
- [ ] **Proper const correctness**
- [ ] **Performance optimized** (minimal copies)
- [ ] **Type safety verified** (use static_assert)
- [ ] **Well documented** (comments explaining type decisions)
- [ ] **Comprehensive tests** covering edge cases

## 📊 Answer Key for MCQs

1. **B** - auto follows template deduction, decltype preserves exact types
2. **D** - const int& (exact type preserved)
3. **B** - decltype(x) vs decltype((x)) difference
4. **B** - C++14 introduced decltype(auto)
5. **B** - int& (dangling reference with parentheses)
6. **B** - int& (vector::operator[] returns reference)
7. **B** - Preserves references like decltype
8. **B** - Preserve exact return types
9. **C** - func1 returns int, func2 returns dangling int&
10. **A** - Trailing return type syntax
11. **B** - Return type of function
12. **B** - Perfect forwarding return types
13. **B** - const int (preserved)
14. **B** - Need exact type preservation
15. **C** - int (rvalue expression)

## 🔧 Testing Commands

**Compile with type checking:**
```bash
g++ -std=c++17 -Wall -Wextra -Werror program.cpp -o program
```

**Check for dangling references:**
```bash
g++ -std=c++17 -fsanitize=address -g program.cpp -o program
./program
```

**Performance profiling:**
```bash
g++ -std=c++17 -O2 -pg program.cpp -o program
./program
gprof program gmon.out
```

**Type information at compile time:**
```bash
g++ -std=c++17 -fdump-tree-original program.cpp
```

## 📚 Additional Resources

- **CppReference decltype**: https://en.cppreference.com/w/cpp/language/decltype
- **Scott Meyers' Blog**: Articles on auto and decltype
- **C++ Core Guidelines**: ES.11 on using auto
- **Compiler Explorer**: https://godbolt.org for type analysis

