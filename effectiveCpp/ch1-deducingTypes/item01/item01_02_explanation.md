# Chapter 1: Item 1 & 2 - Type Deduction Explained Simply

## Item 1: Understand Template Type Deduction

### What is Template Type Deduction?

Template type deduction is how C++ figures out what types to use when you call a template function. Think of it like a smart assistant that looks at what you're passing to a function and automatically determines the correct types.

### The Basic Pattern

```cpp
template<typename T>
void f(ParamType param);

f(expr);  // Compiler deduces T and ParamType from expr
```

When you call `f(expr)`, the compiler looks at `expr` and deduces two things:
1. **T** - The base type
2. **ParamType** - The full parameter type (which may include const, &, *, etc.)

### Three Cases of Type Deduction

#### **Case 1: ParamType is a Reference or Pointer (but not Universal Reference)**

**Rule:** Ignore the reference part of `expr`, then pattern-match `expr`'s type against `ParamType` to determine `T`.

```cpp
template<typename T>
void f(T& param);    // param is a reference

int x = 27;          // x is int
const int cx = x;    // cx is const int
const int& rx = x;   // rx is reference to const int

f(x);   // T is int,       param's type is int&
f(cx);  // T is const int, param's type is const int&
f(rx);  // T is const int, param's type is const int&
        // (reference-ness of rx is ignored)
```

**Key Point:** If you pass a const object to a reference parameter, the const-ness becomes part of T. The function respects the const!

**Example with Pointer:**
```cpp
template<typename T>
void f(T* param);    // param is now a pointer

int x = 27;
const int* px = &x;

f(&x);  // T is int,       param's type is int*
f(px);  // T is const int, param's type is const int*
```

#### **Case 2: ParamType is a Universal Reference (&&)**

**Rule:** This is the tricky one! It depends on whether `expr` is an lvalue or rvalue.

```cpp
template<typename T>
void f(T&& param);   // param is a universal reference

int x = 27;
const int cx = x;
const int& rx = x;

f(x);   // x is lvalue, so T is int&
        // param's type is int&

f(cx);  // cx is lvalue, so T is const int&
        // param's type is const int&

f(rx);  // rx is lvalue, so T is const int&
        // param's type is const int&

f(27);  // 27 is rvalue, so T is int
        // param's type is int&&
```

**Key Points:**
- If `expr` is an **lvalue**, both T and ParamType are deduced to be lvalue references
- If `expr` is an **rvalue**, normal rules apply (like Case 1)
- This is the only situation where T can be deduced to be a reference!

#### **Case 3: ParamType is Neither a Pointer nor a Reference (Pass by Value)**

**Rule:** Ignore reference, const, and volatile. We're making a copy, so modifications to the original don't matter.

```cpp
template<typename T>
void f(T param);     // param is passed by value (copied)

int x = 27;
const int cx = x;
const int& rx = x;

f(x);   // T and param's type are both int
f(cx);  // T and param's type are both int (const is ignored)
f(rx);  // T and param's type are both int (both const and & ignored)
```

**Why?** Because `param` is a completely independent copy. It doesn't matter if the original was const—the copy can be modified.

**Exception with Pointers:**
```cpp
template<typename T>
void f(T param);

const char* const ptr = "Fun with pointers";
// ptr is const pointer to const char

f(ptr);  // T and param's type are const char*
         // Top-level const (pointer is const) is ignored
         // Low-level const (pointed-to object is const) is preserved
```

### Array Arguments

Arrays decay to pointers in most situations, but not always!

```cpp
const char name[] = "J. P. Briggs";  // name is array of 13 chars
const char* ptrToName = name;        // array decays to pointer

template<typename T>
void f(T param);  // pass by value

f(name);  // T is const char*

// BUT with reference:
template<typename T>
void f(T& param);  // pass by reference

f(name);  // T is const char[13]
          // param is const char(&)[13]
```

**Cool Trick:** You can create a template that deduces array size!

```cpp
template<typename T, std::size_t N>
constexpr std::size_t arraySize(T (&)[N]) noexcept {
    return N;
}

int keyVals[] = {1, 3, 7, 9, 11, 22, 35};
int mappedVals[arraySize(keyVals)];  // mappedVals has 7 elements
```

### Function Arguments

Functions also decay to function pointers!

```cpp
void someFunc(int, double);  // someFunc is a function

template<typename T>
void f1(T param);   // pass by value

template<typename T>
void f2(T& param);  // pass by reference

f1(someFunc);  // param deduced as pointer-to-function
               // type is void(*)(int, double)

f2(someFunc);  // param deduced as reference-to-function
               // type is void(&)(int, double)
```

---

## Item 2: Understand Auto Type Deduction

### The Big Idea

**auto type deduction is almost identical to template type deduction!**

When you use `auto`, think of it as a template:
```cpp
auto x = 27;

// Is conceptually the same as:
template<typename T>
void func_for_x(T param);

func_for_x(27);
```

The `auto` plays the role of `T` in the template, and the type specifier for the variable acts like `ParamType`.

### The Same Three Cases

#### **Case 1: Type specifier is a pointer or reference**

```cpp
auto x = 27;        // Case 3 (neither pointer nor reference)
auto& rx = x;       // Case 1 (rx is reference)
const auto& crx = x; // Case 1 (crx is const reference)

auto* px = &x;      // Case 1 (px is pointer)
```

#### **Case 2: Type specifier is a universal reference**

```cpp
auto&& uref1 = x;   // x is int and lvalue
                    // uref1's type is int&

auto&& uref2 = cx;  // cx is const int and lvalue
                    // uref2's type is const int&

auto&& uref3 = 27;  // 27 is int and rvalue
                    // uref3's type is int&&
```

#### **Case 3: Type specifier is neither pointer nor reference**

```cpp
auto x = 27;        // x is int
const auto cx = x;  // cx is const int
```

### Array and Function Name Decay

Just like template deduction:

```cpp
const char name[] = "R. N. Briggs";

auto arr1 = name;   // arr1's type is const char*
auto& arr2 = name;  // arr2's type is const char(&)[13]

void someFunc(int, double);

auto func1 = someFunc;   // func1's type is void(*)(int, double)
auto& func2 = someFunc;  // func2's type is void(&)(int, double)
```

### The ONE Exception: Braced Initializers

**This is where auto differs from template type deduction!**

```cpp
auto x1 = 27;      // type is int, value is 27
auto x2(27);       // type is int, value is 27
auto x3 = {27};    // type is std::initializer_list<int>, value is {27}
auto x4{27};       // C++11: std::initializer_list<int>
                   // C++14/17: int
```

**With templates, this doesn't work:**
```cpp
template<typename T>
void f(T param);

f({11, 23, 9});  // ERROR! Can't deduce type for T

// BUT this works:
template<typename T>
void f(std::initializer_list<T> param);

f({11, 23, 9});  // OK! T deduced as int
                 // param's type is std::initializer_list<int>
```

### C++14: auto for Function Return Types

```cpp
auto createInitList() {
    return {1, 2, 3};  // ERROR! Can't deduce type
}

std::vector<int> v;
auto resetV = [&v](const auto& newValue) {
    v = newValue;
};

resetV({1, 2, 3});  // ERROR! Can't deduce type for {1, 2, 3}
```

**Key Takeaway:** auto in function return types and lambda parameters uses **template type deduction**, not auto type deduction, so braced initializers don't work!

---

## Summary

### Item 1: Template Type Deduction
1. **Case 1** (Reference/Pointer): Reference-ness is ignored, const-ness is preserved
2. **Case 2** (Universal Reference): Lvalue → lvalue reference, Rvalue → rvalue reference  
3. **Case 3** (By Value): Everything is ignored (const, volatile, reference) since we're copying
4. Arrays and functions decay to pointers unless passed by reference
5. Understanding template type deduction is crucial for understanding auto

### Item 2: Auto Type Deduction
1. **auto** deduction is almost identical to template deduction
2. **auto** assumes braced initializer is `std::initializer_list`, templates don't
3. **auto** in function return types and lambda parameters uses template deduction rules
4. Use the same three-case framework as template deduction
5. Remember the initializer_list exception!

---

## Quick Reference Table

| Declaration | Type Deduced | Notes |
|-------------|--------------|-------|
| `auto x = 27;` | `int` | Case 3 |
| `auto& rx = x;` | `int&` | Case 1 |
| `const auto& crx = x;` | `const int&` | Case 1 |
| `auto&& uref = x;` | `int&` | Case 2 (x is lvalue) |
| `auto&& uref = 27;` | `int&&` | Case 2 (27 is rvalue) |
| `auto x = {27};` | `std::initializer_list<int>` | Special case! |
| `auto* px = &x;` | `int*` | Case 1 |

Remember: **Think of auto as a template parameter, and the type deduction follows naturally!**
