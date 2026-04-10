# Item 3 (Part 1): Understand decltype - Basics

## 📌 Core Concept

`decltype` is a C++11 keyword that tells you the **exact type** of a name or expression. Unlike `auto`, which may modify types during deduction, `decltype` gives you the **true, unmodified type**.

Think of it as: "Tell me the **declared type** of this thing, exactly as it was declared."

## 🔑 The Fundamental Difference

### auto vs decltype

```cpp
const int x = 42;
auto y = x;         // y is int (const removed)
decltype(x) z = x;  // z is const int (exact type preserved)
```

**Key Difference:**
- `auto` follows **template type deduction rules** (may strip const, references)
- `decltype` gives you the **exact type** (preserves everything)

## 📖 Basic Usage of decltype

### Example 1: Simple Variables

```cpp
#include <iostream>
#include <string>

int main() {
    int x = 5;
    const int cx = x;
    const int& rx = x;
    
    decltype(x) a = 10;    // a is int
    decltype(cx) b = 20;   // b is const int
    decltype(rx) c = x;    // c is const int& (reference!)
    
    std::cout << "Type of a: int\n";
    std::cout << "Type of b: const int\n";
    std::cout << "Type of c: const int&\n";
    
    // Verify reference behavior
    x = 100;
    std::cout << "After x = 100, c = " << c << "\n";  // c also becomes 100
    
    return 0;
}
```

**Output:**
```
Type of a: int
Type of b: const int
Type of c: const int&
After x = 100, c = 100
```

### Example 2: Containers

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // decltype preserves exact type
    decltype(vec) vec2;              // vec2 is std::vector<int>
    decltype(vec[0]) firstElem = vec[0];  // firstElem is int& (reference!)
    
    // Modifying through reference
    firstElem = 100;
    std::cout << "vec[0] = " << vec[0] << "\n";  // 100
    
    // Compare with auto
    auto elem = vec[0];              // elem is int (copy, not reference)
    elem = 200;
    std::cout << "vec[0] = " << vec[0] << "\n";  // Still 100
    
    return 0;
}
```

### Example 3: Functions

```cpp
#include <iostream>

int getValue() {
    return 42;
}

int& getReference() {
    static int value = 42;
    return value;
}

const int& getConstReference() {
    static int value = 42;
    return value;
}

int main() {
    decltype(getValue()) a = 10;           // a is int
    decltype(getReference()) b = a;        // b is int& (reference)
    decltype(getConstReference()) c = a;   // c is const int&
    
    b = 100;  // Modifies a
    std::cout << "a = " << a << "\n";  // 100
    
    // c = 200;  // Error! c is const
    
    return 0;
}
```

## 🎯 Primary Uses of decltype

### Use Case 1: Template Programming

When you want to declare a variable with the **same type** as another variable:

```cpp
#include <vector>
#include <iostream>

template<typename Container, typename Index>
auto getElement(Container& c, Index i) {
    // We want to return the exact type that c[i] returns
    // But what is that type? Use decltype!
    decltype(c[i]) element = c[i];
    return element;
}

int main() {
    std::vector<int> vec = {10, 20, 30};
    
    auto elem = getElement(vec, 0);
    elem = 999;  // Doesn't modify vec[0]
    
    std::cout << "vec[0] = " << vec[0] << "\n";  // Still 10
    
    return 0;
}
```

### Use Case 2: Variable Declarations

```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    std::map<std::string, int> ages;
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    
    // Instead of typing the complex type:
    // std::pair<const std::string, int>& person = *ages.begin();
    
    // Use decltype:
    decltype(ages)::value_type person = *ages.begin();
    
    // Or even simpler:
    decltype(*ages.begin()) firstPerson = *ages.begin();
    
    std::cout << firstPerson.first << ": " << firstPerson.second << "\n";
    
    return 0;
}
```

### Use Case 3: Return Type Deduction

```cpp
#include <iostream>
#include <vector>

template<typename Container, typename Index>
auto accessElement(Container& c, Index i) -> decltype(c[i]) {
    // Trailing return type using decltype
    // Returns exact type of c[i] (including references!)
    return c[i];
}

int main() {
    std::vector<int> vec = {10, 20, 30};
    
    // Returns int& (reference to element)
    accessElement(vec, 0) = 999;
    
    std::cout << "vec[0] = " << vec[0] << "\n";  // 999
    
    return 0;
}
```

## 🔍 decltype Rules - The Basics

### Rule 1: For Names (Variables, Functions)

`decltype` gives you the **declared type** exactly as declared.

```cpp
int x = 5;
const int cx = x;
const int& rx = x;
int* px = &x;

decltype(x);   // int
decltype(cx);  // const int
decltype(rx);  // const int&
decltype(px);  // int*
```

### Rule 2: For Function Calls

`decltype` gives you the **return type** of the function.

```cpp
int func();
int& funcRef();
const int& funcConstRef();

decltype(func());          // int
decltype(funcRef());       // int&
decltype(funcConstRef());  // const int&
```

## 💡 Practical Examples

### Example 4: Generic Programming

```cpp
#include <iostream>
#include <vector>
#include <list>

// Works with any container
template<typename Container>
void printFirstElement(const Container& c) {
    if (!c.empty()) {
        // Get exact type of first element
        decltype(*c.begin()) first = *c.begin();
        std::cout << "First element: " << first << "\n";
    }
}

int main() {
    std::vector<int> vec = {1, 2, 3};
    std::list<double> lst = {1.1, 2.2, 3.3};
    
    printFirstElement(vec);  // Works
    printFirstElement(lst);  // Also works
    
    return 0;
}
```

### Example 5: Widget Factory

```cpp
#include <iostream>
#include <memory>
#include <vector>

class Widget {
public:
    Widget(int id) : id_(id) {
        std::cout << "Widget " << id_ << " created\n";
    }
    
    int getId() const { return id_; }
    
private:
    int id_;
};

class WidgetFactory {
private:
    std::vector<Widget> widgets_;
    
public:
    Widget& createWidget(int id) {
        widgets_.emplace_back(id);
        return widgets_.back();
    }
    
    const Widget& getWidget(size_t index) const {
        return widgets_[index];
    }
};

int main() {
    WidgetFactory factory;
    
    // decltype figures out the return type
    decltype(factory.createWidget(1)) widget1 = factory.createWidget(1);
    // widget1 is Widget&
    
    decltype(factory.getWidget(0)) widget2 = factory.getWidget(0);
    // widget2 is const Widget&
    
    std::cout << "Widget 1 ID: " << widget1.getId() << "\n";
    std::cout << "Widget 2 ID: " << widget2.getId() << "\n";
    
    return 0;
}
```

## 🔄 Comparison: auto vs decltype

### Scenario 1: Copying vs Referencing

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> vec = {10, 20, 30};
    
    // Using auto - makes a COPY
    auto elem1 = vec[0];
    elem1 = 100;
    std::cout << "vec[0] after auto: " << vec[0] << "\n";  // 10 (unchanged)
    
    // Using auto& - creates a REFERENCE
    auto& elem2 = vec[1];
    elem2 = 200;
    std::cout << "vec[1] after auto&: " << vec[1] << "\n";  // 200 (changed)
    
    // Using decltype - gets EXACT type (which is int& for vec[0])
    decltype(vec[2]) elem3 = vec[2];
    elem3 = 300;
    std::cout << "vec[2] after decltype: " << vec[2] << "\n";  // 300 (changed!)
    
    return 0;
}
```

**Key Insight:** `vec[0]` returns `int&`, so `decltype(vec[0])` is `int&`, not `int`!

### Scenario 2: Const Preservation

```cpp
#include <iostream>

int main() {
    const int cx = 42;
    
    // auto strips const
    auto x1 = cx;        // x1 is int
    x1 = 100;            // OK
    
    // decltype preserves const
    decltype(cx) x2 = cx;  // x2 is const int
    // x2 = 100;           // ERROR! x2 is const
    
    std::cout << "x1 = " << x1 << "\n";
    std::cout << "x2 = " << x2 << "\n";
    
    return 0;
}
```

## 📊 When to Use What?

| Scenario | Use | Reason |
|----------|-----|--------|
| Want a copy | `auto` | Simple, strips references/const |
| Want exact type | `decltype` | Preserves references/const |
| Generic code | `decltype` | Preserves original type characteristics |
| Return type matches expression | `decltype` | Ensures return type matches exactly |
| Simple local variable | `auto` | Cleaner, less verbose |

## 🎓 Summary (Part 1)

1. **`decltype` gives you the exact declared type** - no modifications
2. **Preserves references** - if expression is a reference, decltype includes it
3. **Preserves const/volatile** - qualifiers are maintained
4. **For names** - gives declared type
5. **For expressions** - gives type of expression result
6. **Different from auto** - auto follows template deduction (strips stuff), decltype doesn't
7. **Primary use** - when you need the EXACT type, especially in templates

## 💭 Common Patterns

```cpp
// Pattern 1: Declare variable with same type as another
int x = 5;
decltype(x) y = 10;  // y has same type as x

// Pattern 2: Match container element type
std::vector<int> vec;
decltype(vec[0]) elem = vec[0];  // elem is int&

// Pattern 3: Match function return type
int& getRef();
decltype(getRef()) ref = getRef();  // ref is int&

// Pattern 4: Trailing return type
template<typename T, typename U>
auto add(T t, U u) -> decltype(t + u) {
    return t + u;
}
```

## 🎯 Coming in Part 2

- `decltype(auto)` - best of both worlds
- Special rules for expressions vs names
- Parentheses matter: `decltype(x)` vs `decltype((x))`
- Advanced use cases and gotchas
- Real-world applications in modern C++

## 🔑 Remember

> **"`decltype` tells you the truth, the whole truth, and nothing but the truth about a type."**
> 
> If you want the exact type without modifications, use `decltype`.  
> If you want convenience and don't care about const/references, use `auto`.

