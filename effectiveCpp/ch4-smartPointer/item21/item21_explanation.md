# Item 21: Prefer `std::make_unique` and `std::make_shared` to Direct Use of `new`

## 📌 Core Concept

Instead of writing `std::shared_ptr<Widget>(new Widget)` or `std::unique_ptr<Widget>(new Widget)`,  
prefer the **make functions**:

```cpp
auto sp = std::make_shared<Widget>();   // ✅ preferred
auto up = std::make_unique<Widget>();   // ✅ preferred
```

There are **three concrete reasons** why the make functions are better. Each reason is explained below with examples.

---

## 🎯 Reason 1 — No Type Repetition (Less Typing, Less Drift)

With `new` you write the type name **twice**:

```cpp
// ❌ type repeated — if you rename Widget you must update two places
std::shared_ptr<Widget> sp(new Widget());
std::unique_ptr<Widget> up(new Widget());
```

With make functions, the type appears **once**:

```cpp
// ✅ type written only once
auto sp = std::make_shared<Widget>();
auto up = std::make_unique<Widget>();
```

This also works beautifully with `auto` — you don't need to spell out the full smart-pointer type at all.

---

## 🎯 Reason 2 — Exception Safety (The Most Important Reason)

### The Classic Bug: Exception Leaks Memory

```cpp
void process(std::shared_ptr<Widget> sp, int priority);
int computePriority();   // may throw

// ❌ DANGEROUS — can leak memory!
process(std::shared_ptr<Widget>(new Widget), computePriority());
```

**Why is this dangerous?**

C++ is allowed to evaluate function arguments **in any order**. The compiler may choose to:
1. Execute `new Widget` (allocates memory — Widget is born)
2. Execute `computePriority()` — **throws an exception!**
3. Construct `shared_ptr<Widget>` ← **never reached**

Result: A Widget was allocated but never put inside a `shared_ptr`, so its destructor is never called → **memory leak**.

### The Fix with `make_shared`

```cpp
// ✅ SAFE — new and shared_ptr construction happen atomically inside make_shared
process(std::make_shared<Widget>(), computePriority());
```

`make_shared` allocates and wraps in a single, uninterruptible step. There is no gap where an exception could strike.

### Illustrated Side-by-Side

```cpp
// --- Without make ---
// Step 1: new Widget        ← if step 2 throws, this leaks
// Step 2: computePriority()
// Step 3: shared_ptr(...)

// --- With make_shared ---
// Step 1: make_shared<Widget>()  ← atomic: alloc + wrap together
// Step 2: computePriority()
// No leak possible!
```

---

## 🎯 Reason 3 — Efficiency: One Allocation Instead of Two (make_shared only)

`std::make_shared` is **faster and uses less memory** than `new` + `shared_ptr` constructor.

### Without `make_shared` — 2 Allocations

```cpp
std::shared_ptr<Widget> sp(new Widget());
```

This performs:
1. `new Widget()` — allocates the Widget object
2. `shared_ptr` constructor — allocates a **separate control block** (holds reference counts, deleter, etc.)

```
Heap:
  [ Widget data ]        ← allocation 1
  [ control block ]      ← allocation 2  (ref count, weak count, deleter, ...)
```

### With `make_shared` — 1 Allocation

```cpp
auto sp = std::make_shared<Widget>();
```

`make_shared` allocates a **single chunk** that holds both the Widget and the control block together:

```
Heap:
  [ Widget data | control block ]  ← single allocation
```

**Why this matters:**
- Half the allocator overhead (allocator calls are expensive)
- Better cache locality — Widget and control block are adjacent in memory
- Fewer cache misses when the runtime accesses the control block

---

## 📖 Complete Example: Seeing All Three Benefits

```cpp
#include <memory>
#include <iostream>
#include <string>

struct Widget {
    int value;
    std::string label;

    Widget(int v, std::string l) : value(v), label(std::move(l)) {
        std::cout << "Widget(" << value << ", " << label << ") constructed\n";
    }
    ~Widget() {
        std::cout << "Widget(" << value << ", " << label << ") destroyed\n";
    }
};

int riskyFunc() {
    throw std::runtime_error("oops!");
    return 42;
}

void safeDemo() {
    // Reason 1: no type duplication
    auto sp = std::make_shared<Widget>(1, "alpha");   // Widget written once
    auto up = std::make_unique<Widget>(2, "beta");    // Widget written once

    std::cout << "sp use_count: " << sp.use_count() << "\n";   // 1

    // Reason 2: exception safety
    try {
        // Safe: make_shared is atomic
        // process(std::make_shared<Widget>(3, "gamma"), riskyFunc());
        // (commented out so program doesn't terminate)
    } catch (...) {}

    // Reason 3: efficiency — nothing special to show, it just happens inside make_shared
}

int main() {
    safeDemo();
    // Both sp and up are destroyed automatically at end of safeDemo
}
```

**Output:**
```
Widget(1, alpha) constructed
Widget(2, beta) constructed
Widget(2, beta) destroyed
Widget(1, alpha) destroyed
```

---

## 🚫 Limitations of Make Functions — When You Must Use `new`

Make functions are not always usable. Here are four situations where `new` is required.

---

### Limitation 1: Custom Deleter

Make functions do not accept a custom deleter. If your resource needs special cleanup, use `new` directly.

```cpp
auto deleter = [](Widget* pw) {
    logDestruction(pw);   // custom log
    delete pw;
};

// ❌ make_shared cannot take a deleter
// auto sp = std::make_shared<Widget>(deleter);

// ✅ use new when a custom deleter is needed
std::unique_ptr<Widget, decltype(deleter)> up(new Widget(), deleter);
std::shared_ptr<Widget> sp(new Widget(), deleter);
```

---

### Limitation 2: Initializer-List Construction (`{...}`)

Make functions use perfect forwarding with `()` (parentheses), not `{}` (braces).  
If the type you're creating has a constructor that takes `std::initializer_list`, the braces won't be forwarded correctly.

```cpp
// Widget(std::initializer_list<int>) constructor
std::vector<int> v1 = {1, 2, 3, 4};   // uses initializer_list
// auto sp = std::make_shared<std::vector<int>>(1, 2, 3, 4);  // ❌ wrong constructor!

// Workaround: create an initializer_list explicitly
auto initList = {1, 2, 3, 4};
auto sp = std::make_shared<std::vector<int>>(initList);   // ✅

// Or just use new:
std::shared_ptr<std::vector<int>> sp2(new std::vector<int>{1, 2, 3, 4});
```

---

### Limitation 3: Classes with Custom `operator new` / `operator delete`

Some classes override `operator new` to allocate exactly `sizeof(Widget)` bytes. `make_shared` would allocate a combined block for Widget + control block, which has a different size — bypassing the custom allocator.

```cpp
class SpecialWidget {
public:
    static void* operator new(std::size_t size);   // custom allocator
    static void  operator delete(void* ptr, std::size_t size);
};

// make_shared would allocate sizeof(SpecialWidget) + control_block_size
// bypassing the custom allocator — use new directly:
std::shared_ptr<SpecialWidget> sp(new SpecialWidget());
```

---

### Limitation 4: Large Objects + Many `weak_ptr`s (Memory Concern)

With `make_shared`, the Widget and control block share **one allocation**.  
The control block is freed only when both the strong count **and** the weak count reach 0.

So if many `weak_ptr`s keep the control block alive long after the Widget is gone, the Widget's memory is **also stuck** (because it's part of the same allocation).

```
make_shared allocation:   [ Widget | control block ]
                                ^
                    Can't free Widget independently!
                    Freed only when weak count also = 0
```

With `new`, the Widget has its own allocation and can be freed independently:

```
new Widget:    [ Widget ]          ← freed when strong count = 0
               [ control block ]   ← freed when weak count = 0
```

**If the managed object is large and many `weak_ptr`s are involved, prefer `new`** to allow the object memory to be released promptly.

---

## 🔀 When You Must Write `new` Safely

If you must use `new` (e.g., custom deleter), do it safely by **immediately** constructing the smart pointer, not passing `new` directly as a function argument:

```cpp
// ❌ Still potentially unsafe — two operations in a function argument list
process(std::shared_ptr<Widget>(new Widget), computePriority());

// ✅ Safe: construct smart_ptr on its own statement first
std::shared_ptr<Widget> sp(new Widget);   // line 1: atomic
process(std::move(sp), computePriority());  // line 2: sp is already safe
```

---

## ⚡ `std::make_unique` Note

`std::make_unique` was introduced in **C++14**. If you're on C++11, it's easy to write your own:

```cpp
// Simplified make_unique for C++11 (single object, no array support)
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

Note: `make_unique` does **not** provide the efficiency benefit of `make_shared` (no single-allocation optimization) — its advantages are type-deduplication and exception safety only.

---

## 📋 Summary Table

| | `make_shared` / `make_unique` | Direct `new` |
|---|---|---|
| Type written once | ✅ | ❌ twice |
| Exception-safe | ✅ always | ❌ risky in argument lists |
| Single allocation (shared only) | ✅ | ❌ two allocations |
| Supports custom deleter | ❌ | ✅ |
| Supports `{}` initializer list | ❌ | ✅ |
| Works with custom `operator new` | ❌ | ✅ |
| Releases object memory promptly (weak_ptr concern) | ❌ | ✅ |

---

## 🎯 Decision Guide

```
Do you need a custom deleter?             → use new (wrap immediately!)
Does your type use initializer_list {}?   → use new
Does your class override operator new?    → use new
Large object + many long-lived weak_ptrs? → use new
Everything else?                          → use make_shared / make_unique  ✅
```
