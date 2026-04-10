# `std::move`, Lvalue & Rvalue — Senior C++ Interview Questions

**Target:** C++ Developer with 10+ years C++ experience, 14+ years overall  
**Topics:** Value categories (lvalue, rvalue, xvalue, prvalue, glvalue), `std::move`, move semantics, perfect forwarding, RVO/NRVO

---

## Section 1: Value Categories — Foundations

### Q1. Taxonomy of value categories

C++11 introduced a refined value category taxonomy. Name all five categories as defined by the standard, explain the two fundamental properties they represent (has identity, can be moved from), and draw the relationship diagram.

**Expected depth:** The five categories: lvalue, xvalue, prvalue, glvalue (= lvalue ∪ xvalue), rvalue (= xvalue ∪ prvalue). Two properties: "has identity" (i = can take address), "can be moved from" (m = resources may be stolen). lvalue: i & ¬m. xvalue: i & m. prvalue: ¬i & m. The candidate should mention that this replaced the simpler C++03 lvalue/rvalue split.

---

### Q2. Identify the value category

For each expression below, state its value category and explain why:

```cpp
int x = 42;
int& ref = x;
int&& rref = std::move(x);

// (a) x
// (b) std::move(x)
// (c) ref
// (d) rref
// (e) 42
// (f) x + 1
// (g) std::string("hello")
// (h) static_cast<int&&>(x)
```

**Expected depth:** (a) lvalue — named variable. (b) xvalue — `std::move` casts to `T&&`. (c) lvalue — named reference. (d) **lvalue** — a named rvalue reference is itself an lvalue (critical gotcha). (e) prvalue — literal. (f) prvalue — result of arithmetic. (g) prvalue — temporary (pre-C++17), or prvalue initializer (C++17 with guaranteed copy elision). (h) xvalue — equivalent to `std::move(x)`. The (d) case is the most important — many senior developers get it wrong.

---

### Q3. "A named rvalue reference is an lvalue"

Explain the rationale behind this rule. Why did the committee decide that a named `T&&` variable should bind as an lvalue? What would go wrong if it didn't?

**Expected depth:** If a named rvalue reference were an rvalue, its resources could be silently stolen every time it was used in an expression, potentially before the programmer intended. The "use it exactly once as an rvalue" pattern is enforced by requiring an explicit `std::move()` at the point where the programmer consciously surrenders the resource. This prevents accidental double-move and use-after-move bugs.

---

### Q4. Reference collapsing rules

Given the template:

```cpp
template<typename T>
void foo(T&& param);
```

If `foo` is called with an lvalue of type `Widget`, what is `T`? What is the type of `param`? State all four reference collapsing rules and explain which one applies here.

**Expected depth:** Four rules: `& &` → `&`, `& &&` → `&`, `&& &` → `&`, `&& &&` → `&&`. When called with an lvalue `Widget w`, `T` is deduced as `Widget&`. `param` type becomes `Widget& &&` → collapses to `Widget&`. This is the "universal reference" / "forwarding reference" mechanism. With an rvalue, `T` = `Widget`, `param` = `Widget&&`.

---

## Section 2: `std::move` — Deep Understanding

### Q5. What `std::move` actually does

A junior developer says "`std::move` moves the object." Correct them. Show a plausible implementation of `std::move` and explain each component.

**Expected depth:** `std::move` performs zero work at runtime — it is a pure `static_cast` to an rvalue reference:
```cpp
template<typename T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}
```
It unconditionally casts its argument to an rvalue reference, enabling overload resolution to pick the move constructor/assignment. The actual "moving" (resource theft) happens inside the move constructor/assignment operator that is subsequently called. `std::move` itself moves nothing.

---

### Q6. `std::move` on a `const` object

```cpp
const std::string s = "hello";
std::string t = std::move(s);
```

What happens? Is `s` moved from? What constructor is called for `t`? When is this a real-world problem?

**Expected depth:** `std::move(s)` yields `const std::string&&`. The move constructor takes `std::string&&` (non-const), so it doesn't match. The copy constructor (`const std::string&`) is selected instead. `s` is silently **copied**, not moved. Real-world problem: classes storing `const` members, or functions taking parameters by `const T` instead of `T` — the `std::move` compiles but does nothing. This is a known source of performance bugs that compilers/linters should flag.

---

### Q7. State of a moved-from object

After:

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = std::move(a);
```

What can you legally do with `a`? What does the standard guarantee about its state? Do all standard library types behave the same way?

**Expected depth:** The standard requires moved-from objects to be in a "valid but unspecified" state. You may: (1) destroy it, (2) assign to it, (3) call operations with no preconditions (e.g. `a.size()`, `a.empty()`, `a.clear()`). You must NOT assume `a` is empty — that's implementation-specific (though in practice most implementations do leave it empty). `std::unique_ptr` is a notable exception where the standard explicitly guarantees it is `nullptr` after move.

---

### Q8. `std::move` in a return statement

```cpp
Widget makeWidget() {
    Widget w;
    // ... configure w ...
    return std::move(w);  // Good or bad?
}
```

Why is this actually **pessimisation** rather than optimisation? What compiler optimisation does it prevent?

**Expected depth:** NRVO (Named Return Value Optimisation) constructs `w` directly in the caller's return slot — zero copies, zero moves. `std::move(w)` changes the return expression's type to `Widget&&`, which inhibits NRVO because the returned expression no longer matches the local variable. The compiler then performs a move instead of eliding entirely. Even without NRVO, the standard (C++11 §12.8/32) mandates implicit move on return of a local variable, so `std::move` is redundant at best, harmful at worst. Exception: the implicit move doesn't apply when returning a function parameter.

---

### Q9. `std::move` vs `std::forward`

When would you use `std::move` and when `std::forward`? Can you use `std::move` inside a template function that takes a forwarding reference? What goes wrong if you do?

**Expected depth:** `std::move` unconditionally casts to rvalue — use when you know you're done with the object. `std::forward` conditionally casts to rvalue only if the original argument was an rvalue — use with forwarding references (`T&&`) to preserve the value category. Using `std::move` on a forwarding reference steals resources from lvalue arguments, which is a bug — the caller still expects to own the object. `std::forward<T>(param)` preserves the caller's intent.

---

## Section 3: Move Semantics in Class Design

### Q10. Writing a move constructor

Write a correct move constructor and move assignment for a class managing a raw resource:

```cpp
class Buffer {
    int* data_;
    size_t size_;
public:
    // ...
};
```

What happens if you forget to null-out the source's members? What is the role of `noexcept`?

**Expected depth:**
```cpp
Buffer(Buffer&& other) noexcept
    : data_(other.data_), size_(other.size_) {
    other.data_ = nullptr;
    other.size_ = 0;
}
Buffer& operator=(Buffer&& other) noexcept {
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}
```
If source isn't nulled: double-free when source is destroyed. `noexcept` is critical: `std::vector` reallocations use `move_if_noexcept` — without `noexcept`, the vector will copy instead of move for strong exception safety.

---

### Q11. The Rule of Five and the Rule of Zero

Explain the Rule of Five. Then explain the Rule of Zero and why it is the preferred modern C++ style. Under what circumstances must you fall back to the Rule of Five?

**Expected depth:** Rule of Five: if you define any of {destructor, copy ctor, copy assign, move ctor, move assign}, you should define all five. Rule of Zero: design classes so they manage no resources directly — use smart pointers and RAII containers so the compiler-generated defaults are correct. Fall back to Rule of Five when: wrapping a C resource (file descriptor, socket, GPU handle), implementing a custom allocator, or building the RAII wrappers themselves.

---

### Q12. Implicitly deleted and suppressed moves

```cpp
class Widget {
    std::string name_;
    ~Widget() { log("destroyed"); }
};
```

Does `Widget` have a move constructor? Does it have a copy constructor? Explain the suppression rules and the C++ standard's rationale. How would you fix this class to be movable?

**Expected depth:** User-declared destructor suppresses implicit generation of move operations. The copy constructor is still implicitly generated (deprecated in C++11 but allowed). So `Widget` is copyable but not movable — moves silently fall back to copies. Rationale: if you need a destructor, the compiler-generated move might not be safe. Fix: explicitly default the move operations: `Widget(Widget&&) = default;` and `Widget& operator=(Widget&&) = default;`.

---

### Q13. `noexcept` and `move_if_noexcept`

Why does `std::vector::push_back` use `std::move_if_noexcept` during reallocation instead of unconditional `std::move`? What happens if your class's move constructor is not `noexcept`?

**Expected depth:** During reallocation, if the move constructor throws after some elements have been moved, the original vector is in an inconsistent state — some elements are moved-from, some are not — and strong exception safety is lost. `move_if_noexcept` returns an rvalue reference if move is `noexcept`, otherwise an lvalue reference (forcing a copy). If your move ctor is not `noexcept`, `vector` copies every element during reallocation — potentially enormous performance penalty. This is one of the most common performance traps in C++.

---

### Q14. Move semantics with inheritance

```cpp
class Base {
    std::string name_;
public:
    Base(Base&& other) noexcept : name_(std::move(other.name_)) {}
};

class Derived : public Base {
    std::vector<int> data_;
public:
    Derived(Derived&& other) noexcept
        : Base(other), data_(std::move(other.data_)) {}
};
```

There is a bug in the `Derived` move constructor. Find it and explain the fix.

**Expected depth:** `Base(other)` passes `other` as an lvalue (it's a named parameter), so the **copy** constructor of `Base` is called instead of the move constructor. Fix: `Base(std::move(other))`. This is the classic "forgot to move the base sub-object" bug. Every member and base must be explicitly `std::move`'d in a hand-written move constructor.

---

## Section 4: Perfect Forwarding

### Q15. Implement `std::forward`

Write an implementation of `std::forward` and explain why the `typename std::remove_reference<T>::type&` overload exists. What happens if someone calls `std::forward` without an explicit template argument?

**Expected depth:**
```cpp
template<typename T>
constexpr T&& forward(std::remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}
template<typename T>
constexpr T&& forward(std::remove_reference_t<T>&& t) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value,
                  "cannot forward an rvalue as an lvalue");
    return static_cast<T&&>(t);
}
```
The lvalue overload handles the common case (named parameters are lvalues). The rvalue overload prevents misuse. Without explicit `<T>`, argument deduction cannot work because the parameter type uses `remove_reference_t` (a non-deduced context). The explicit template argument carries the original value category information deduced by the outer template.

---

### Q16. Forwarding reference vs rvalue reference

```cpp
void f(Widget&& w);           // (A)
template<typename T>
void g(T&& t);                // (B)
auto&& x = getWidget();       // (C)
```

Which of these are rvalue references and which are forwarding references? What test determines the difference?

**Expected depth:** (A) rvalue reference — `Widget` is a concrete type. (B) forwarding reference — `T&& ` where `T` is deduced. (C) forwarding reference — `auto&&` with type deduction. The test: if type deduction occurs on the `T&&` parameter (either `template<typename T>` or `auto&&`), it's a forwarding reference. If the type is already known/fixed (e.g. `Widget&&`, `const T&&`), it's an rvalue reference. Note: `const T&&` is NOT a forwarding reference — `const` disqualifies it.

---

### Q17. Perfect forwarding failure cases

Name at least four scenarios where perfect forwarding fails (the forwarded call behaves differently from a direct call). For each, explain the root cause.

**Expected depth:** (1) **Braced initializers**: `{1, 2, 3}` — cannot deduce `std::initializer_list<T>` from braces. (2) **0 or NULL as null pointer**: deduced as `int`, not pointer type. (3) **Declaration-only `static const` integral members**: no address → linker error when forwarding binds a reference. (4) **Overloaded function names / templates**: ambiguous — can't deduce which overload. (5) **Bitfields**: can't bind a reference to a bitfield. Workaround patterns: `auto` variable for braces, `nullptr` instead of 0, providing a definition for static members (or `inline constexpr` in C++17), explicit cast to resolve overloads, copy bitfield to a local.

---

### Q18. Forwarding variadic arguments

Write a factory function `create<T>(args...)` that perfectly forwards an arbitrary number of arguments to `T`'s constructor. Then write a version that forwards to `std::make_shared<T>`. Explain the pack expansion mechanism.

**Expected depth:**
```cpp
template<typename T, typename... Args>
T create(Args&&... args) {
    return T(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
std::shared_ptr<T> createShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
```
`std::forward<Args>(args)...` is a pack expansion — the pattern `std::forward<Args>(args)` is applied to each element of the packs `Args` and `args` simultaneously. Each argument retains its original value category.

---

## Section 5: Advanced / Scenario-Based

### Q19. Universal reference and overload resolution

```cpp
class Person {
    std::string name_;
public:
    template<typename T>
    explicit Person(T&& n) : name_(std::forward<T>(n)) {}

    Person(const Person& rhs) : name_(rhs.name_) {}
};

Person p("Alice");
Person p2(p);   // What happens?
```

Explain why the copy fails. What are three different approaches to fix this?

**Expected depth:** `p` is a non-const lvalue. The template deduces `T = Person&`, producing `Person(Person&)` — a better match than `Person(const Person&)` (no const conversion needed). The forwarding constructor wins, then tries to construct `std::string` from a `Person` — compile error. Fixes: (1) SFINAE/`enable_if` to disable the template when `T` decays to `Person`. (2) `if constexpr` tag-dispatch (C++17). (3) C++20 concepts: `requires (!std::same_as<std::decay_t<T>, Person>)`. Effective Modern C++ Item 26–27 covers this in detail.

---

### Q20. Move semantics and exception safety

You are implementing `operator=` using the copy-and-swap idiom:

```cpp
Widget& Widget::operator=(Widget rhs) {
    swap(*this, rhs);
    return *this;
}
```

How does this interact with move semantics? If called with an rvalue, how many copies and moves occur? If called with an lvalue? Is this approach still optimal in C++11+?

**Expected depth:** Called with lvalue: `rhs` is copy-constructed → 1 copy, then swap. Called with rvalue: `rhs` is move-constructed → 1 move, then swap. This idiom leverages overload resolution at the call site — the compiler picks move or copy for the by-value parameter. still provides strong exception safety. It's generally optimal and concise. Downside: always allocates a new resource even for self-assignment, and the swap may be slightly less efficient than a direct move-assignment for the rvalue case.

---

### Q21. `std::move` in lambda captures (C++14)

```cpp
auto ptr = std::make_unique<Widget>();
auto lambda = [p = std::move(ptr)]() {
    p->doWork();
};
```

Explain the generalised lambda capture (init-capture). What is the value category of `p` inside the lambda body? How would you move `p` out of the lambda?

**Expected depth:** Init-capture (`p = std::move(ptr)`) move-constructs `p` inside the lambda's closure. Inside the body, `p` is an lvalue (it's a named member of the closure object). To move it out, the lambda must be marked `mutable` (to allow modifying members) and you'd use `std::move(p)`. In C++23, `std::move_only_function` replaces `std::function` for move-only callables, since `std::function` requires copyable callables and cannot store a lambda with `unique_ptr` captures.

---

### Q22. Sink parameters: by-value vs by-reference overloads

Compare these three signatures for a setter that stores the argument:

```cpp
// (A) Overload pair
void setName(const std::string& name);
void setName(std::string&& name);

// (B) By-value sink
void setName(std::string name);

// (C) Forwarding reference
template<typename T>
void setName(T&& name);
```

For each, count the number of copies and moves for: (i) lvalue argument, (ii) rvalue argument, (iii) string literal. Which would you choose for a non-template class and why?

**Expected depth:**

| Approach | lvalue | rvalue | string literal |
|----------|--------|--------|----------------|
| (A) overloads | 1 copy | 1 move | 1 temp + 1 move |
| (B) by-value | 1 copy + 1 move | 1 move + 1 move (or elided) | 1 temp + 1 move (or direct) |
| (C) forwarding ref | 1 copy | 1 move | 0 copy, direct construct |

(A) is optimal but verbose (2^N overloads for N parameters). (B) is simple, one extra move per call (often negligible). (C) is maximally efficient but brings template bloat, header dependencies, and the overload-resolution trap from Q19. For a non-template class, (B) is the pragmatic default per Effective Modern C++ Item 41.

---

### Q23. Detecting accidental copies in a move-heavy codebase

Your profiler shows unexpected copy constructor calls in a performance-critical path. Describe at least three techniques or tools you would use to find and eliminate unwanted copies.

**Expected depth:** (1) Mark copy constructor `= delete` temporarily to get compile errors at copy sites. (2) Add a `#ifdef DEBUG` counter/log in the copy constructor. (3) Clang-Tidy checks: `performance-unnecessary-copy-initialization`, `performance-move-const-arg`, `bugprone-use-after-move`. (4) Compiler flags: `-Wpessimizing-move`, `-Wredundant-move` (GCC/Clang). (5) Use `-flifetime-dse` and address sanitizer to detect use-after-move. (6) Static analysis: mark classes as `[[clang::trivial_abi]]` and audit ABI boundaries.

---

### Q24. Conditional move in generic code

Implement a generic `optional_move` that moves from its argument only if a runtime condition is true, otherwise leaves the source intact. Why is this fundamentally at odds with C++ move semantics? What is the recommended pattern instead?

**Expected depth:** A naive attempt:
```cpp
template<typename T>
T optional_move(bool should_move, T& value) {
    if (should_move) return std::move(value);
    else return value;
}
```
This is problematic because the *type* of the return is the same either way — the compiler decides copy vs move at compile time based on the expression's value category, not at runtime. In practice, both branches must have consistent semantics. The recommended pattern: use different code paths at the call site, or use `std::move_if_noexcept` which is a compile-time decision. True conditional-move is an anti-pattern — it breaks the invariant that a moved-from object should not be reused.

---

### Q25. RVO, NRVO, and mandatory copy elision (C++17)

Explain the differences between RVO, NRVO, and C++17 mandatory copy/move elision (guaranteed copy elision). For each, state whether a move/copy constructor must exist. Then explain what this code does in C++14 vs C++17:

```cpp
struct NoCopy {
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy(NoCopy&&) = delete;
};

NoCopy make() { return NoCopy{}; }
NoCopy x = make();
```

**Expected depth:** RVO (unnamed return): compiler may elide copy/move of a temporary — widespread but optional pre-C++17. NRVO (named return): compiler may elide copy/move of a named local — optional even in C++17. Both require an accessible move/copy constructor in C++14 even if elision occurs. C++17 mandatory elision: returning a prvalue of the same type constructs directly in the destination — no temporary exists. Move/copy constructors need not exist. The code above: fails in C++14 (deleted move/copy ctor needed for potential materialisation), compiles in C++17 (prvalue initialisation, no temporary).
