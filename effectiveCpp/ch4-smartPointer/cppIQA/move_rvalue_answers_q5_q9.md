# Answers — Q5 to Q9: `std::move` — Deep Understanding

**Source:** `std::move`, Lvalue & Rvalue Interview Questions  
**Scope:** Section 2 — `std::move`

---

## Q5. What `std::move` actually does

### The Misconception

> "std::move moves the object."

This is **wrong**. `std::move` does not move anything. It has no runtime cost, generates no machine instructions for moving, and does not touch the object's contents.

### What It Actually Does

`std::move` is a **compile-time cast** — a `static_cast` to an rvalue reference. It changes the *value category* of the expression so that overload resolution selects the move constructor or move assignment operator instead of the copy version.

### Implementation

```cpp
template<typename T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}
```

#### Breaking it down piece by piece:

**1. `T&&` parameter (forwarding reference)**

Because `T` is a deduced template parameter, `T&&` is a forwarding reference — it can bind to both lvalues and rvalues.

- If called with an lvalue `Widget w`: `T` = `Widget&`, parameter type = `Widget& &&` → collapses to `Widget&`
- If called with an rvalue `Widget{}`: `T` = `Widget`, parameter type = `Widget&&`

**2. `std::remove_reference_t<T>`**

Strips any reference from `T`:

- If `T` = `Widget&` → `remove_reference_t<T>` = `Widget`
- If `T` = `Widget` → `remove_reference_t<T>` = `Widget`

This ensures we always cast to `Widget&&`, regardless of what was passed.

**3. `static_cast<std::remove_reference_t<T>&&>(...)`**

The actual work: a `static_cast` to an rvalue reference. This changes the expression's value category to xvalue ("I'm done with this, you may steal its resources").

**4. `constexpr`**

Usable in constant expressions (compile-time evaluation contexts).

**5. `noexcept`**

A cast cannot throw. Marking it `noexcept` makes this explicit and allows callers that depend on `noexcept` guarantees (like `std::vector` reallocation) to reason about it.

### The Two-Step Process

Moving an object always involves two steps:

```
Step 1: std::move(obj)      →  produces an xvalue (rvalue reference)
                                THIS IS JUST A CAST — nothing happens to obj

Step 2: SomeType t = <xvalue> →  overload resolution picks the move constructor
                                  THE MOVE CONSTRUCTOR does the actual resource theft
```

If no move constructor exists, the copy constructor is selected instead — `std::move` compiles but results in a copy.

### Demonstrating That `std::move` Does Nothing On Its Own

```cpp
std::string s = "hello";
std::move(s);            // xvalue is created and immediately discarded
std::cout << s;           // prints "hello" — s is completely untouched

std::string t = std::move(s);  // NOW the move constructor runs
// s is in a valid-but-unspecified state
```

### Compiler Output

On any major compiler with optimisation enabled, `std::move` generates **zero instructions**. It is purely a type-system annotation consumed during overload resolution.

---

## Q6. `std::move` on a `const` object

### The Code

```cpp
const std::string s = "hello";
std::string t = std::move(s);
```

### What Happens — Step by Step

1. `std::move(s)` casts `s` to `const std::string&&` (rvalue reference to **const**).

2. Overload resolution for constructing `t` considers:
   - `std::string(std::string&&)` — the move constructor. Requires `std::string&&` (non-const). **Does not match** `const std::string&&`.
   - `std::string(const std::string&)` — the copy constructor. `const std::string&&` can bind to `const std::string&`. **Matches.**

3. The **copy constructor** is selected. `s` is **copied**, not moved.

### Why `const T&&` Doesn't Match the Move Constructor

The move constructor's parameter is `T&&` (non-const), because it needs to modify the source (null out pointers, reset sizes, etc.). A `const` source cannot be modified, so `const T&&` is rejected by the move constructor.

```cpp
// std::string's move constructor (simplified):
string(string&& other) noexcept {
    data_ = other.data_;
    size_ = other.size_;
    other.data_ = nullptr;   // ← must modify source
    other.size_ = 0;         // ← must modify source
}
// Can't do this with a const source!
```

### The Silent Performance Bug

The dangerous part is that this **compiles without any warning** on most compilers by default. The code looks like it's moving but is actually copying. You only discover the problem through profiling or careful code review.

### Real-World Scenarios Where This Bites

**1. `const` member variables**

```cpp
class Config {
    const std::string name_;   // const member
public:
    // Compiler-generated move constructor tries to move name_,
    // but it's const → silently copies instead
};
```

**2. Functions taking `const T` by value**

```cpp
void processName(const std::string name) {  // const parameter
    names_.push_back(std::move(name));       // copies, not moves!
}
```

**3. Returning a `const` local variable**

```cpp
const std::string makeName() {
    const std::string result = compute();
    return result;  // const inhibits move AND NRVO
}
```

### Detection

| Tool | Flag / Check |
|------|-------------|
| Clang-Tidy | `performance-move-const-arg` |
| GCC | `-Wpessimizing-move` (for returns) |
| Clang | `-Wpessimizing-move`, `-Wredundant-move` |

### Key Takeaway

> `std::move` is a request to move, not a command. If the type is `const`, the request is silently denied and a copy happens instead.

---

## Q7. State of a moved-from object

### The Standard's Guarantee

After:

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = std::move(a);
```

The standard says `a` is in a **"valid but unspecified"** state. This means:

### What You CAN Do With a Moved-From Object

| Operation | Safe? | Why |
|-----------|-------|-----|
| Destroy it (let it go out of scope) | ✅ | Destructor must work on any valid state |
| Assign a new value to it | ✅ | Assignment has no preconditions |
| Call `a.clear()` | ✅ | No preconditions |
| Call `a.size()` | ✅ | No preconditions — but the result is unspecified |
| Call `a.empty()` | ✅ | No preconditions — result is unspecified |
| Call `a = {4, 5, 6}` then use normally | ✅ | After assignment, fully usable again |

### What You Must NOT Assume

| Assumption | Valid? |
|-----------|--------|
| `a` is empty | ❌ Not guaranteed (though typical in practice) |
| `a.size() == 0` | ❌ Not guaranteed |
| `a.data() == nullptr` | ❌ Not guaranteed |
| `a[0]` is safe | ❌ No — size is unspecified, could be 0 |

### What "Valid But Unspecified" Means Precisely

- **Valid:** The object satisfies its class invariants. The destructor will not cause undefined behaviour. Any operation without preconditions will work.
- **Unspecified:** You don't know *which* valid state it's in. It could be empty, could still have elements, could have a different capacity — the standard does not say.

### Notable Exceptions with Stronger Guarantees

Some types in the standard library provide **explicit guarantees** after move:

| Type | Guarantee After Move |
|------|---------------------|
| `std::unique_ptr<T>` | Guaranteed to be `nullptr` |
| `std::shared_ptr<T>` | Guaranteed to be `nullptr` |
| `std::weak_ptr<T>` | Guaranteed to be expired / empty |
| `std::optional<T>` | Guaranteed to be disengaged (`has_value() == false`) **only if** the move was from `optional` itself (not from the contained `T`) |
| `std::string` (libstdc++/libc++) | In practice empty, but NOT guaranteed by the standard |
| `std::vector` (libstdc++/libc++) | In practice empty, but NOT guaranteed by the standard |

### The Safe Pattern

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = std::move(a);

// SAFE:
a.clear();                   // reset to known state
a = {10, 20, 30};           // assign new value
// Now a is fully usable again

// UNSAFE — don't do this:
// for (int x : a) { ... }  // a's contents are unspecified
```

### Why Not Guarantee Empty After Move?

The standard deliberately leaves this unspecified to allow optimised implementations. For example, a small-string-optimised `std::string` may leave the source with the short buffer still containing data, since there's no heap pointer to steal — the "move" degrades to a copy of the internal buffer. Mandating an empty state would require extra work (zeroing the buffer) for no benefit.

### Best Practice

> Treat moved-from objects as if they hold garbage. Either destroy them, assign to them, or call `.clear()`. Never read their contents.

---

## Q8. `std::move` in a return statement

### The Code

```cpp
Widget makeWidget() {
    Widget w;
    // ... configure w ...
    return std::move(w);  // ← DON'T DO THIS
}
```

### Why This Is a Pessimisation

This code **prevents** an optimisation that is more efficient than moving.

#### Without `std::move` — NRVO applies:

```cpp
Widget makeWidget() {
    Widget w;
    return w;  // ← compiler constructs w directly in caller's memory
}
// Result: ZERO copies, ZERO moves
```

**Named Return Value Optimisation (NRVO)** constructs `w` directly in the caller's return slot. The object is never copied or moved — it's built in-place at the final destination. This is the most efficient possible outcome.

#### With `std::move` — NRVO is inhibited:

```cpp
Widget makeWidget() {
    Widget w;
    return std::move(w);  // type is Widget&&, not Widget
}
// Result: ONE move (NRVO prevented)
```

`std::move(w)` changes the expression's type from `Widget` to `Widget&&`. NRVO requires the return expression to be a **named local variable** — `std::move(w)` is a function call expression, not a named variable. NRVO is disabled, and the compiler must perform a move construction from `w` into the return slot.

### The Three Levels of Efficiency

```
Best:    return w;              →  NRVO: 0 copies, 0 moves
Good:    return w;              →  Implicit move (if NRVO fails): 0 copies, 1 move
Bad:     return std::move(w);   →  Forced move (NRVO impossible): 0 copies, 1 move
```

### The Implicit Move Rule

Even when NRVO is not applied (e.g., multiple return paths), the standard **already mandates** that returning a local variable performs a move:

> **C++11 §12.8/32 (C++20 §11.10.5):** When the criteria for elision are met (or would be met but for the fact that the source is a function parameter), overload resolution is performed as if the object were an rvalue.

```cpp
Widget makeWidget(bool flag) {
    Widget w1, w2;
    if (flag) return w1;   // implicit move (NRVO may not apply with two candidates)
    return w2;             // implicit move
}
```

No `std::move` needed — the compiler automatically treats `w1` and `w2` as rvalues in the return statement.

### When `std::move` IS Needed in a Return Statement

There is **one case** where implicit move does not apply: **returning a function parameter**.

```cpp
Widget transform(Widget w) {
    // ... modify w ...
    return w;              // C++11/14: implicit move applies (parameter)
    // Actually, C++11 does apply implicit move for parameters too.
}
```

**Update (C++11–C++20):** In C++11 through C++17, implicit move applies to local variables and parameters. In C++20, the rules were further relaxed (P1825R0) to allow implicit move in more situations, including throwing. In C++23, the implicit move rules are even broader (P2266R3 — simulated copy/move elision in return statements).

The practical advice is simple:

> **Never write `return std::move(x);` for a local variable or parameter.** The compiler handles it.

### Exceptional case — returning a member or different type:

```cpp
struct Holder {
    Widget widget_;
    Widget getWidget() {
        return std::move(widget_);  // ✅ CORRECT — widget_ is a member, not a local
    }
};
```

Members are not eligible for NRVO or implicit move in return statements. Here `std::move` is necessary.

### Compiler Warnings

| Compiler | Flag |
|----------|------|
| GCC | `-Wpessimizing-move` (warns when `std::move` prevents elision) |
| Clang | `-Wpessimizing-move` |
| Clang | `-Wredundant-move` (warns when `std::move` is unnecessary) |
| MSVC | `/W4` may warn in some cases |

---

## Q9. `std::move` vs `std::forward`

### The Core Difference

| | `std::move` | `std::forward<T>` |
|---|---|---|
| **Cast type** | Unconditional cast to `T&&` | Conditional cast — rvalue if `T` is non-reference, lvalue if `T` is lvalue reference |
| **Use case** | "I'm done with this, steal it" | "Pass it along with the same value category the caller used" |
| **Context** | Known object you own | Forwarding reference (`T&&` with deduced `T`) |

### `std::move` — Unconditional

```cpp
void sink(Widget w);

void caller() {
    Widget w;
    // ... use w ...
    sink(std::move(w));  // "I'm done with w, move it into sink's parameter"
}
```

`std::move` always produces an rvalue. It says: "I, the programmer, am explicitly surrendering this resource."

### `std::forward<T>` — Conditional

```cpp
template<typename T>
void relay(T&& arg) {
    target(std::forward<T>(arg));
}
```

`std::forward<T>` preserves the value category of the original argument:
- If `relay` was called with an **lvalue**, `T` = `Widget&`, `std::forward<Widget&>(arg)` returns `Widget&` (lvalue)
- If `relay` was called with an **rvalue**, `T` = `Widget`, `std::forward<Widget>(arg)` returns `Widget&&` (rvalue)

### What Goes Wrong If You Use `std::move` on a Forwarding Reference

```cpp
template<typename T>
void relay(T&& arg) {
    target(std::move(arg));  // ⚠️ BUG
}

Widget w;
relay(w);  // caller passes lvalue, expects w to be intact after the call
```

**The problem:** `std::move(arg)` unconditionally casts `arg` to an rvalue. When the caller passed an lvalue (`w`), they expect `w` to remain valid. But `std::move` tells `target` that it can steal `arg`'s resources — which are actually `w`'s resources. After `relay(w)` returns, `w` is silently moved-from.

```
Caller's expectation:  "I passed w by reference, it should still be usable"
What happened:         "relay stole w's guts via std::move"
```

With `std::forward`:

```cpp
template<typename T>
void relay(T&& arg) {
    target(std::forward<T>(arg));  // ✅ CORRECT
}

Widget w;
relay(w);          // lvalue → forwarded as lvalue → w is untouched
relay(Widget{});   // rvalue → forwarded as rvalue → resources can be moved
```

### Decision Table

| Situation | Use |
|-----------|-----|
| You have a concrete object and you're done with it | `std::move` |
| You're in a template with `T&&` and want to pass `arg` to another function | `std::forward<T>` |
| You're in a template with `T&&` and it's the **last use** and you know it's the only use | `std::forward<T>` is still correct (don't use `std::move`) |
| You're returning a local variable from a function | Neither — just `return x;` |
| You're passing a universal reference to a constructor for storage | `std::forward<T>` |

### The Mechanical Difference

```cpp
// std::move: unconditional
template<typename T>
decltype(auto) move(T&& t) {
    return static_cast<remove_reference_t<T>&&>(t);  // ALWAYS &&
}

// std::forward: conditional — depends on T
template<typename T>
T&& forward(remove_reference_t<T>& t) {
    return static_cast<T&&>(t);
    // If T = Widget&  → returns Widget& && → Widget&  (lvalue)
    // If T = Widget   → returns Widget&&              (rvalue)
}
```

The key insight: `std::forward` uses reference collapsing on `T&&`. When `T` is an lvalue reference, `T&&` collapses to `T&` (lvalue). When `T` is not a reference, `T&&` stays `T&&` (rvalue). `std::move` strips all references first and always produces `&&`.

### Common Mistake: Using `std::forward` Without a Forwarding Reference

```cpp
void process(Widget&& w) {         // NOT a forwarding reference — Widget is concrete
    use(std::forward<Widget>(w));   // "works" but misleading
    use(std::move(w));              // ✅ clearer intent
}
```

`std::forward` should only appear with deduced `T&&`. Using it with a concrete rvalue reference is technically valid but confusing — use `std::move` instead to clearly communicate unconditional move intent.

### Summary

> **`std::move`** = "Cast to rvalue. Always."  
> **`std::forward`** = "Cast to rvalue only if it was originally an rvalue. Preserve the caller's intent."
