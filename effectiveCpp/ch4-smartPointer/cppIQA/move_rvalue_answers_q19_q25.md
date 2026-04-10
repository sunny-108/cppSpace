# Answers — Q19 to Q25: Advanced / Scenario-Based

**Source:** `std::move`, Lvalue & Rvalue Interview Questions  
**Scope:** Section 5 — Advanced / Scenario-Based

---

## Q19. Universal reference and overload resolution

### The Code

```cpp
class Person {
    std::string name_;
public:
    template<typename T>
    explicit Person(T&& n) : name_(std::forward<T>(n)) {}

    Person(const Person& rhs) : name_(rhs.name_) {}
};

Person p("Alice");
Person p2(p);   // ❌ What happens?
```

### What Goes Wrong

`p` is a **non-const lvalue** of type `Person`. Two constructors are viable:

| Candidate | Instantiated signature | Conversion needed |
|-----------|----------------------|-------------------|
| Template (`T&&`) | `Person(Person&)` (T = Person&) | **Exact match** — no conversion |
| Copy ctor | `Person(const Person&)` | Requires adding `const` |

The template wins because `Person&` is a **better match** than `const Person&` for a non-const lvalue — the copy constructor requires a const qualification, the template does not.

Once the template is selected, the body executes:

```cpp
name_(std::forward<Person&>(p))
// → std::string constructed from a Person& 
// → compile error: no conversion from Person to std::string
```

### Why `const` Makes a Difference

```cpp
const Person cp("Bob");
Person p3(cp);    // ✅ Works!
```

With a `const` lvalue, the template deduces `T = const Person&`, instantiating `Person(const Person&)` — the **exact same** signature as the copy constructor. When two candidates are equally viable and one is a non-template, the non-template wins. So the copy constructor is selected, and it works.

The problem only manifests with **non-const** lvalues (and rvalues of type `Person`).

### Fix 1: SFINAE with `enable_if` (C++11/14)

Disable the template when `T` decays to `Person`:

```cpp
class Person {
    std::string name_;
public:
    template<typename T,
             typename = std::enable_if_t<
                 !std::is_same_v<std::decay_t<T>, Person>
             >>
    explicit Person(T&& n) : name_(std::forward<T>(n)) {}

    Person(const Person& rhs) : name_(rhs.name_) {}
};
```

When `T` deduces to `Person&` or `Person&&`, `decay_t<T>` is `Person`, the `enable_if` condition is `false`, SFINAE removes the template from the overload set, and the copy constructor is selected.

**`std::decay_t` is crucial here** — it strips references and cv-qualifiers so that `Person`, `Person&`, `const Person&`, `Person&&` all map to `Person`.

### Fix 2: `if constexpr` + tag dispatch (C++17)

```cpp
class Person {
    std::string name_;

    // Implementation detail: construct from string-like argument
    template<typename T>
    void initName(T&& n) { name_ = std::forward<T>(n); }

public:
    template<typename T>
    explicit Person(T&& n) {
        if constexpr (std::is_same_v<std::decay_t<T>, Person>) {
            name_ = n.name_;  // act like copy
        } else {
            name_ = std::forward<T>(n);  // forward to string
        }
    }
};
```

This keeps a single constructor but branches at compile time. The unused branch is discarded, so there's no overhead.

### Fix 3: C++20 Concepts (Cleanest)

```cpp
class Person {
    std::string name_;
public:
    template<typename T>
        requires (!std::same_as<std::decay_t<T>, Person>)
    explicit Person(T&& n) : name_(std::forward<T>(n)) {}

    Person(const Person& rhs) : name_(rhs.name_) {}
};
```

The `requires` clause cleanly removes the template from consideration when `T` is `Person`. This gives the best error messages and the most readable code.

**Even cleaner with a concept:**

```cpp
template<typename T, typename Self>
concept NotSelf = !std::same_as<std::decay_t<T>, Self>;

class Person {
    std::string name_;
public:
    template<NotSelf<Person> T>
    explicit Person(T&& n) : name_(std::forward<T>(n)) {}

    Person(const Person&) = default;
};
```

### Extending to Derived Classes

The `std::decay_t` check alone doesn't handle derived classes:

```cpp
class Employee : public Person { ... };
Employee e;
Person p(e);  // T = Employee& → decay_t = Employee ≠ Person → template wins → error!
```

Fix: also check for derived types:

```cpp
// C++11/14
template<typename T,
         typename = std::enable_if_t<
             !std::is_base_of_v<Person, std::decay_t<T>>
         >>
explicit Person(T&& n);

// C++20
template<typename T>
    requires (!std::derived_from<std::decay_t<T>, Person>)
explicit Person(T&& n);
```

---

## Q20. Move semantics and exception safety

### The Copy-and-Swap Idiom

```cpp
class Widget {
    int* data_;
    size_t size_;
public:
    friend void swap(Widget& a, Widget& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }

    // Single unified assignment operator
    Widget& operator=(Widget rhs) {   // by VALUE
        swap(*this, rhs);
        return *this;
    }
};
```

### How It Interacts with Move Semantics

The parameter `rhs` is taken **by value**. The compiler selects move or copy construction based on the argument's value category — no special code needed.

#### Case 1: Called with an lvalue

```cpp
Widget a, b;
a = b;   // b is an lvalue
```

```
1. rhs is COPY-constructed from b         → 1 copy
2. swap(*this, rhs)                        → 3 pointer/int swaps (cheap)
3. rhs (holding old *this data) is destroyed → 1 destructor call
Total: 1 copy + 1 swap + 1 destroy
```

#### Case 2: Called with an rvalue

```cpp
Widget a;
a = Widget{100};   // rvalue
```

```
1. rhs is MOVE-constructed from the rvalue  → 1 move (or elided entirely)
2. swap(*this, rhs)                          → 3 pointer/int swaps (cheap)
3. rhs (holding old *this data) is destroyed  → 1 destructor call
Total: 1 move + 1 swap + 1 destroy
```

#### Case 3: Called with `std::move`

```cpp
Widget a, b;
a = std::move(b);   // xvalue
```

```
1. rhs is MOVE-constructed from b          → 1 move
2. swap(*this, rhs)                         → 3 pointer/int swaps
3. rhs destroyed
Total: 1 move + 1 swap + 1 destroy
```

### Exception Safety

The idiom provides the **strong exception guarantee**:

1. The copy/move into `rhs` happens **before** any modification to `*this`
2. If the copy constructor throws, `*this` is completely untouched
3. `swap` is `noexcept` (swapping pointers/ints can't throw)
4. The destructor of `rhs` is `noexcept`

If anything goes wrong, it goes wrong during step 1 — before `*this` is modified.

### Is This Still Optimal in C++11+?

**Mostly yes**, with some caveats:

| Aspect | Verdict |
|--------|---------|
| Code simplicity | ✅ One function handles both copy and move |
| Exception safety | ✅ Strong guarantee |
| Lvalue performance | ✅ Same as dedicated copy-assign |
| Rvalue performance | ⚠️ Slightly suboptimal — swap does extra work |
| Self-assignment | ⚠️ Allocates a copy then swaps — wasteful but safe |

**The rvalue case in detail:**

With copy-and-swap:
```
1. Move-construct rhs from source  (steal pointer)
2. swap(*this, rhs)                (swap two pointers — 2 assignments each)
3. Destroy rhs                     (delete old data)
```

With a dedicated move-assignment:
```
1. Delete this->data_
2. Steal source's pointer
3. Null out source
```

The dedicated version is 1 delete + 2 assignments. Copy-and-swap is 1 move + 3 swaps (6 assignments) + 1 delete. The difference is usually negligible but measurable in extremely hot paths.

**Pragmatic recommendation:** Copy-and-swap is still excellent for most code. Only write separate copy/move assignments if profiling shows the swap overhead matters.

---

## Q21. `std::move` in lambda captures (C++14)

### Generalised Lambda Capture (Init-Capture)

```cpp
auto ptr = std::make_unique<Widget>();
auto lambda = [p = std::move(ptr)]() {
    p->doWork();
};
```

Before C++14, lambda captures could only copy or reference existing variables. There was no way to **move** an object into a lambda. C++14 introduced init-captures (generalised captures) that allow arbitrary initialisation expressions.

#### What happens step by step:

```
1. ptr owns the Widget
2. Lambda closure is created
3. p (a member of the closure) is MOVE-CONSTRUCTED from ptr
4. ptr is now nullptr
5. p inside the closure exclusively owns the Widget
```

The closure class the compiler generates looks roughly like:

```cpp
class __lambda_xyz {
    std::unique_ptr<Widget> p;  // member — the captured variable
public:
    __lambda_xyz(std::unique_ptr<Widget> init_p)
        : p(std::move(init_p)) {}

    void operator()() const {   // const by default!
        p->doWork();
    }
};
```

### Value Category of `p` Inside the Lambda

`p` is a **named member** of the closure object. Inside the lambda body, the expression `p` is an **lvalue** — just like any other named variable.

```cpp
auto lambda = [p = std::move(ptr)]() {
    // p is an lvalue here
    auto* raw = p.get();      // ✅ fine — reading an lvalue
    // auto p2 = std::move(p); // ❌ error — operator() is const, can't move a const member
};
```

### Moving `p` Out of the Lambda

To move `p`, you need two things:

1. **`mutable`** — the default `operator()` is `const`, which makes all captured members `const`. `mutable` removes the `const`.
2. **`std::move(p)`** — explicitly convert the lvalue to an rvalue.

```cpp
auto lambda = [p = std::move(ptr)]() mutable {
    //                                ^^^^^^^ allows modifying p
    auto transferred = std::move(p);  // ✅ p is moved out
    // p is now nullptr
    transferred->doWork();
};

lambda();   // works once
// lambda();  // second call: p is nullptr → undefined behaviour
```

### The `std::function` Problem

```cpp
auto ptr = std::make_unique<Widget>();
auto lambda = [p = std::move(ptr)]() mutable {
    p->doWork();
};

// std::function<void()> fn = std::move(lambda);  // ❌ compile error!
```

`std::function` requires its callable to be **CopyConstructible**. A lambda capturing a `unique_ptr` is not copyable (because `unique_ptr` is not copyable). This means you **cannot store move-only lambdas in `std::function`**.

**Workarounds:**

| Approach | Standard | Details |
|----------|----------|---------|
| `std::move_only_function<void()>` | C++23 | Drop-in replacement that only requires MoveConstructible |
| `template<typename F> void accept(F&& fn)` | C++11 | Deduce the exact lambda type, avoid type erasure |
| Wrap in `shared_ptr` | C++11 | `auto sp = make_shared<unique_ptr<Widget>>(std::move(ptr)); auto fn = [sp]() { (*sp)->doWork(); };` — now copyable, but shared ownership |

### Other Init-Capture Patterns

```cpp
// Move a non-copyable object
auto socket = createSocket();
auto handler = [s = std::move(socket)]() mutable { s.send("hello"); };

// Capture by value with a different name
int x = computeValue();
auto fn = [val = x]() { return val * 2; };

// Capture an expression result
auto fn2 = [s = std::to_string(42)]() { return s.size(); };

// Capture *this by copy (C++17 alternative: [*this])
auto fn3 = [self = *this]() { return self.name_; };
```

---

## Q22. Sink parameters: by-value vs by-reference overloads

### The Three Approaches

Suppose the setter stores the argument into a member:

```cpp
class Widget {
    std::string name_;
public:
    void setName(/* ??? */);  // stores into name_
};
```

### Approach A: Overload Pair

```cpp
void setName(const std::string& name) { name_ = name; }        // for lvalues
void setName(std::string&& name)      { name_ = std::move(name); }  // for rvalues
```

### Approach B: By-Value Sink

```cpp
void setName(std::string name) { name_ = std::move(name); }
```

### Approach C: Forwarding Reference

```cpp
template<typename T>
void setName(T&& name) { name_ = std::forward<T>(name); }
```

### Cost Analysis

#### (i) Lvalue argument: `std::string s = "hello"; w.setName(s);`

| Approach | What happens | Total cost |
|----------|-------------|------------|
| A (overloads) | `const string&` overload selected → `name_ = s` (copy-assign) | **1 copy** |
| B (by-value) | `name` copy-constructed from `s` → `name_ = std::move(name)` | **1 copy + 1 move** |
| C (forwarding) | `T = string&` → `name_ = s` (copy-assign) | **1 copy** |

#### (ii) Rvalue argument: `w.setName(std::string("hello"));`

| Approach | What happens | Total cost |
|----------|-------------|------------|
| A (overloads) | `string&&` overload selected → `name_ = std::move(name)` | **1 move** |
| B (by-value) | `name` move-constructed from rvalue → `name_ = std::move(name)` | **1 move + 1 move** (second may be elided) |
| C (forwarding) | `T = string` → `name_ = std::move(name)` | **1 move** |

#### (iii) String literal: `w.setName("hello");`

| Approach | What happens | Total cost |
|----------|-------------|------------|
| A (overloads) | Implicit conversion `"hello"` → temp `string` → `string&&` overload → move-assign | **1 construction + 1 move** |
| B (by-value) | `name` constructed directly from `"hello"` (no temp) → `name_ = std::move(name)` | **1 construction + 1 move** |
| C (forwarding) | `T = const char(&)[6]` → `name_ = "hello"` → `string::operator=(const char*)` directly | **1 direct assign** (most efficient) |

### Summary Table

| | lvalue | rvalue | literal | Overloads needed |
|---|---|---|---|---|
| **(A) Overloads** | 1 copy | 1 move | 1 ctor + 1 move | 2^N for N params |
| **(B) By-value** | 1 copy + 1 move | 2 moves (1 elided) | 1 ctor + 1 move | 1 |
| **(C) Forwarding** | 1 copy | 1 move | 1 direct assign | 1 (template) |

### Trade-Off Analysis

| Consideration | A (Overloads) | B (By-value) | C (Forwarding) |
|--------------|:---:|:---:|:---:|
| Efficiency | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ |
| Code simplicity | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ |
| Scales to N params | ❌ (2^N overloads) | ✅ | ✅ |
| Works in non-template class | ✅ | ✅ | ⚠️ (puts impl in header) |
| Error messages | ✅ (clear) | ✅ (clear) | ❌ (template noise) |
| Overload-resolution trap (Q19) | ✅ (safe) | ✅ (safe) | ❌ (possible) |

### Recommendation

**For a non-template class, use (B) by-value sink** as the default:

```cpp
void setName(std::string name) {
    name_ = std::move(name);
}
```

The extra move per call (compared to A) is almost always negligible — a move is typically just copying 2-3 pointers. The simplicity and scalability benefits outweigh the micro-cost. This is the advice from Effective Modern C++ Item 41.

**Use (A) overloads** when profiling proves the extra move matters in a hot path.

**Use (C) forwarding** in generic/library code where maximal efficiency for all argument types is required and you're willing to handle the template complications.

### The "One Extra Move" Intuition

Why is (B) always one move more than (A)?

```
Approach A (lvalue): caller's string ──copy──→ name_
Approach B (lvalue): caller's string ──copy──→ param ──move──→ name_
                                                ↑ extra step
```

The by-value approach creates an intermediate (the parameter), which is then moved into the member. The overload approach goes directly to the member. That intermediate is the cost of simplicity.

---

## Q23. Detecting accidental copies in a move-heavy codebase

### The Scenario

Your profiler shows unexpected copy constructor calls in a performance-critical path. Here are six techniques to find and eliminate them:

### Technique 1: Temporarily Delete the Copy Constructor

```cpp
class HeavyObject {
    // Temporarily make non-copyable to find all copy sites
    HeavyObject(const HeavyObject&) = delete;
    HeavyObject& operator=(const HeavyObject&) = delete;
    // ...
};
```

Recompile — every site that copies will now produce a compile error. This gives you a complete map of all copy locations. Go through each one and decide: should this be a move, a reference, or a legitimate copy?

**Pros:** 100% exhaustive, zero false negatives.  
**Cons:** Breaks the build — can't commit, only for local investigation.

### Technique 2: Copy Counter / Log in Debug Builds

```cpp
class HeavyObject {
#ifdef DEBUG_COPIES
    inline static std::atomic<int> copy_count{0};
#endif
public:
    HeavyObject(const HeavyObject& other) : data_(other.data_) {
#ifdef DEBUG_COPIES
        ++copy_count;
        // Optional: capture stack trace
        std::cerr << "COPY #" << copy_count
                  << " at " << __FILE__ << ":" << __LINE__ << "\n";
#endif
    }

#ifdef DEBUG_COPIES
    static int getCopyCount() { return copy_count; }
    static void resetCopyCount() { copy_count = 0; }
#endif
};
```

Use in tests:

```cpp
HeavyObject::resetCopyCount();
runCriticalPath();
ASSERT_EQ(HeavyObject::getCopyCount(), 0) << "Unexpected copies in critical path";
```

**Pros:** Can run in CI, regression-tests against future accidental copies.  
**Cons:** Only instruments classes you modify. No stack trace without extra work.

### Technique 3: Clang-Tidy Static Analysis Checks

| Check | What it catches |
|-------|----------------|
| `performance-unnecessary-copy-initialization` | `auto x = expensive_func();` when `const auto&` would suffice |
| `performance-move-const-arg` | `std::move` on `const` object (silent copy, Q6) |
| `performance-unnecessary-value-param` | Function takes `const T` by value when `const T&` would work |
| `bugprone-use-after-move` | Using an object after `std::move` (not a copy issue, but related) |
| `performance-no-automatic-move` | Missing implicit move on return (e.g., returning a `const` local) |
| `modernize-pass-by-value` | Could restructure to by-value + move instead of const-ref + copy |

Run across the codebase:

```bash
clang-tidy -checks='performance-*,bugprone-use-after-move' \
    -p build/ src/**/*.cpp
```

**Pros:** Automated, runs in CI, catches patterns across the whole codebase.  
**Cons:** Heuristic-based — may have false positives/negatives.

### Technique 4: Compiler Warning Flags

```bash
# GCC / Clang
-Wpessimizing-move     # warns when std::move prevents RVO
-Wredundant-move       # warns when std::move is unnecessary
-Wrange-loop-analysis  # warns about copies in range-for loops

# Example:
g++ -Wall -Wpessimizing-move -Wredundant-move main.cpp
```

Specific catches:

```cpp
Widget make() {
    Widget w;
    return std::move(w);  // -Wpessimizing-move fires!
}

for (auto elem : vec_of_strings) {  // -Wrange-loop-analysis may warn:
    // elem is copied each iteration! Use auto& or const auto&
}
```

### Technique 5: Runtime Profiling with Copy Breakpoints

Set a debugger breakpoint on the copy constructor:

```bash
# GDB
break 'HeavyObject::HeavyObject(HeavyObject const&)'

# LLDB
b HeavyObject::HeavyObject(HeavyObject const&)
```

Then run the critical path. Every time the breakpoint hits, inspect the call stack to see who triggered the copy.

**Pros:** See exact runtime call stacks, no code modification.  
**Cons:** Manual, one class at a time.

### Technique 6: AddressSanitizer / Valgrind for Use-After-Move

While not directly about copies, these tools catch a related bug — using an object after moving from it, which often indicates the programmer intended a move but the code silently fell back to a copy somewhere upstream:

```bash
# Compile with ASan
clang++ -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp

# Or use Valgrind
valgrind --tool=memcheck ./binary
```

### Summary: Recommended Workflow

```
1. Run clang-tidy with performance checks          → find static issues
2. Enable -Wpessimizing-move -Wredundant-move       → catch move misuse
3. Add copy-counter to suspect classes               → quantify at runtime
4. Temporarily delete copy ctor on hot-path classes  → exhaustive audit
5. Add regression tests asserting copy_count == 0    → prevent regressions
```

---

## Q24. Conditional move in generic code

### The Naive Attempt

```cpp
template<typename T>
T optional_move(bool should_move, T& value) {
    if (should_move) return std::move(value);  // returns T (move-constructed)
    else return value;                          // returns T (copy-constructed)
}
```

### Why This Is Fundamentally Problematic

#### Problem 1: The cost is already paid by the return type

The function returns `T` **by value**. Both branches **always create a new object**:

- `if` branch: `T` is move-constructed from `value`
- `else` branch: `T` is copy-constructed from `value`

The "conditional" aspect only determines **how** the new `T` is created (copy vs move). It doesn't give you "leave the source alone if false" — the `else` branch still copies from `value` to create the return object.

#### Problem 2: The caller can't distinguish the outcomes

```cpp
auto result = optional_move(condition, source);
// After this line:
//   If condition was true:  source is moved-from
//   If condition was false: source is intact, result is a copy
//
// But the caller has no way to know which happened at compile time!
```

The type of `result` is `T` either way. The compiler generates code for **both** branches — the binary contains both a move path and a copy path. This defeats the compile-time nature of move semantics.

#### Problem 3: Breaks the move contract

C++ move semantics are built on a clear contract:

```
std::move(x) → "I'm done with x, take its resources"
```

A "conditional move" breaks this contract:

```
optional_move(flag, x) → "I might be done with x... check at runtime"
```

After the call, the programmer must track a runtime boolean to know whether `x` is valid. This is exactly the kind of fragile state management that move semantics was designed to eliminate.

#### Problem 4: Runtime overhead

```cpp
// The compiler cannot optimise this:
if (should_move) {
    // emit move construction code
} else {
    // emit copy construction code
}
// Both code paths exist in the binary — branch prediction overhead
```

Compared to unconditional move or copy, which compiles to a single straight-line path.

### Why move_if_noexcept Works But This Doesn't

`std::move_if_noexcept` is **compile-time** conditional:

```cpp
template<typename T>
constexpr auto move_if_noexcept(T& x) noexcept {
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        return std::move(x);
    else
        return x;
}
```

The `if constexpr` is resolved at compile time — only one branch exists in the binary. The compiler and the programmer both know, at compile time, whether a move or copy occurs. There's no runtime ambiguity.

### The Recommended Patterns

**Pattern 1: Separate code paths at the call site**

```cpp
if (should_transfer_ownership) {
    destination = std::move(source);
    // source is now moved-from — we know this
} else {
    destination = source;  // copy
    // source is intact — we know this
}
```

Clear ownership semantics. Each path has unambiguous state.

**Pattern 2: Use `std::optional` to represent "maybe take it"**

```cpp
template<typename T>
std::optional<T> maybe_take(bool should_take, T& value) {
    if (should_take) return std::move(value);
    return std::nullopt;
}
```

The type system now encodes whether a value was taken.

**Pattern 3: Use different function signatures**

```cpp
void process_and_consume(Widget&& w);   // takes ownership
void process_and_observe(const Widget& w);  // borrows

if (done_with_widget) {
    process_and_consume(std::move(w));
} else {
    process_and_observe(w);
}
```

Ownership transfer is explicit in the function name and signature.

### Key Takeaway

> Move semantics are a **compile-time** mechanism — they affect which constructor overload is selected. Trying to make them **runtime-conditional** creates ambiguity about object state, defeats optimisations, and produces code that is harder to reason about. Keep move decisions at the call site where ownership intent is clear.

---

## Q25. RVO, NRVO, and mandatory copy elision (C++17)

### The Three Levels of Elision

#### RVO — Return Value Optimisation (unnamed temporaries)

```cpp
Widget make() {
    return Widget(42);   // returning a temporary (prvalue)
}
Widget w = make();
```

The compiler constructs the `Widget` directly in `w`'s memory location, skipping the temporary entirely:

```
Without RVO:  construct temporary → move/copy to w → destroy temporary
With RVO:     construct directly into w's storage
```

- **Status pre-C++17:** Optional optimisation (but applied by virtually all compilers, even at -O0)
- **Status C++17+:** **Mandatory** — the standard guarantees this (see below)
- **Move/copy ctor required?** Pre-C++17: yes (must be accessible even if not called). C++17: **no**.

#### NRVO — Named Return Value Optimisation

```cpp
Widget make() {
    Widget w(42);      // named local variable
    w.configure();
    return w;          // returning a named variable
}
Widget result = make();
```

The compiler constructs `w` directly in `result`'s storage:

```
Without NRVO: construct w → move to result → destroy w
With NRVO:    construct w directly in result's storage
```

- **Status pre-C++17:** Optional optimisation
- **Status C++17+:** **Still optional** — not mandated (compilers may or may not apply it)
- **Move/copy ctor required?** Yes, in all standards — must be accessible because NRVO may not apply
- **When NRVO typically fails:** Multiple return paths with different named variables, complex control flow

```cpp
Widget make(bool flag) {
    Widget a, b;
    if (flag) return a;    // which one to construct in the return slot?
    return b;              // NRVO typically disabled — falls back to implicit move
}
```

#### C++17 Mandatory Copy Elision (Guaranteed Elision)

C++17 redefined prvalues. A prvalue is no longer "a temporary object" — it's an **initialiser** that doesn't materialise into an object until needed. This means returning a prvalue never creates a temporary at all.

```cpp
Widget make() {
    return Widget(42);   // Widget(42) is a prvalue — an initialiser, not an object
}
Widget w = make();       // Widget(42) directly initialises w. No temporary ever exists.
```

**Key difference from pre-C++17 RVO:**

| | Pre-C++17 RVO | C++17 Mandatory Elision |
|---|---|---|
| Is it guaranteed? | No (optional optimisation) | Yes (mandated by the standard) |
| Does a temporary exist? | Conceptually yes, but elided | No temporary ever exists |
| Move/copy ctor must exist? | Yes (must be accessible) | **No** — not needed at all |
| Applies to named variables? | NRVO — optional | **No** — only to prvalues |

### The Test Case

```cpp
struct NoCopy {
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy(NoCopy&&) = delete;
};

NoCopy make() { return NoCopy{}; }
NoCopy x = make();
```

#### In C++14:

```
NoCopy{} is a prvalue (temporary).
Returning it requires a move/copy constructor (even if elision would skip the actual call).
Both are deleted → ❌ COMPILE ERROR
```

The standard says (pre-C++17): even when the compiler elides a copy/move, the constructor must be accessible. The program is ill-formed if it isn't.

#### In C++17:

```
NoCopy{} is a prvalue initialiser.
return NoCopy{}; → the prvalue is propagated to the call site.
NoCopy x = make(); → NoCopy{} directly initialises x.
No copy or move constructor is ever considered.
✅ COMPILES AND WORKS
```

No temporary is created. No copy/move constructor is needed. The prvalue flows through the function boundary as an initialiser.

### Visualising the Difference

**Pre-C++17 model (conceptual):**
```
make() body:
  ┌──────────┐
  │ NoCopy{} │  ← temporary created
  └────┬─────┘
       │ move (elided but must exist)
       ▼
  ┌──────────┐
  │    x     │  ← destination
  └──────────┘
```

**C++17 model:**
```
make() body:
  NoCopy{} is just a recipe: "call NoCopy default ctor"
       │
       │ (recipe flows through, no object created yet)
       ▼
  ┌──────────┐
  │    x     │  ← NoCopy() executes HERE, directly in x
  └──────────┘
```

### Practical Implications

#### 1. Non-movable types can be returned by value

```cpp
// C++17: works with mutex, atomic, or any non-movable type
std::mutex createMutex() {
    return std::mutex{};   // ✅ in C++17 (prvalue, no move needed)
}

std::mutex m = createMutex();  // ✅
```

#### 2. Factory functions for non-movable types

```cpp
class DatabaseConnection {
public:
    DatabaseConnection(std::string connStr);
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection(DatabaseConnection&&) = delete;
};

DatabaseConnection connect(std::string cs) {
    return DatabaseConnection(std::move(cs));  // ✅ C++17
}
```

#### 3. NRVO is still not mandatory — be aware

```cpp
NoCopy make() {
    NoCopy n;         // named variable
    return n;         // NRVO — NOT guaranteed, even in C++17
}                     // ❌ compile error even in C++17 (NoCopy is not movable)
```

Returning a **named** variable still requires a move/copy constructor because NRVO is not mandatory. Only returning a **prvalue** (`return NoCopy{};`) benefits from guaranteed elision.

### Summary Table

| Feature | Applies to | Guaranteed? | Move/Copy ctor needed? | Standard |
|---------|-----------|:-----------:|:----------------------:|----------|
| RVO | Unnamed temporaries (prvalues) | Pre-C++17: No | Yes | C++03 |
| NRVO | Named local variables | Never guaranteed | Yes | C++03 |
| Implicit move on return | Named locals/params | Yes (if eligible) | Yes (move ctor) | C++11 |
| Mandatory copy elision | Prvalue initialisation | **Yes** | **No** | **C++17** |

### Best Practice

```cpp
// Prefer returning prvalues for guaranteed elision:
Widget make() { return Widget(args...); }            // ✅ guaranteed in C++17

// Named variables get NRVO (likely) or implicit move (guaranteed):
Widget make() { Widget w(args...); return w; }       // ✅ NRVO or move

// NEVER add std::move to a return of local variable:
Widget make() { Widget w; return std::move(w); }     // ❌ prevents NRVO

// NEVER add std::move to a return of a prvalue:
Widget make() { return std::move(Widget(args...)); } // ❌ prevents mandatory elision
```
