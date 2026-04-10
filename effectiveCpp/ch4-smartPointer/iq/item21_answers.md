# Item 21 — `make_unique` / `make_shared` vs `new` — Detailed Interview Answers

**Target:** Senior C++ Developer (10+ years C++, 14+ years overall)  
**Source:** Effective Modern C++ — Item 21

---

## Q16. Exception Safety in Function Arguments

> ```cpp
> processWidget(std::shared_ptr<Widget>(new Widget), computePriority());
> ```
>
> *Explain how this line can leak memory. How does `std::make_shared` prevent the leak? Is this still a problem in C++17? Why or why not?*

### Answer

#### How the leak happens (pre-C++17)

Before C++17, the standard gave compilers **complete freedom** to evaluate function arguments in any order and to **interleave** sub-expressions across arguments. A single function-call expression like the one above contains three sub-operations:

1. `new Widget` — allocates and constructs a Widget on the heap.
2. `std::shared_ptr<Widget>(...)` — wraps the raw pointer in a `shared_ptr`.
3. `computePriority()` — evaluates the second argument.

The compiler is allowed to pick **any** ordering, including this dangerous one:

```
Step 1:  new Widget              → Widget allocated on the heap (raw pointer exists)
Step 2:  computePriority()       → THROWS an exception!
Step 3:  shared_ptr<Widget>(…)   → NEVER REACHED
```

The Widget was allocated in step 1, but the `shared_ptr` that would own it was never constructed (step 3 was skipped). No RAII wrapper exists for the Widget. The exception propagates, the stack unwinds, and the Widget's memory is **leaked** — its destructor never runs.

This can happen because `new Widget` and `shared_ptr(...)` are *separate sub-expressions within the same argument*. The compiler is only required to evaluate all sub-expressions of one argument before beginning another argument *within* that same argument — but it can interleave across arguments.

#### Visual timeline of the leak

```
                   new Widget         computePriority()      shared_ptr(raw)
                       │                     │                     │
Dangerous order:       ①                     ②  ← THROWS          ③  ← never reached
                       │                     ✕
                       ▼
                  Widget on heap, no owner → LEAKED
```

#### How `make_shared` prevents it

```cpp
processWidget(std::make_shared<Widget>(), computePriority());
```

`make_shared<Widget>()` is a **single function call**. Inside `make_shared`, the allocation of the Widget and the construction of the `shared_ptr` happen as a single, indivisible operation. The raw pointer is never exposed — it goes directly from `new` into the `shared_ptr`'s internal state.

Now there are only two argument evaluations:
1. `make_shared<Widget>()` — returns a fully-constructed `shared_ptr` (RAII complete).
2. `computePriority()` — may throw.

No matter which order the compiler evaluates them:

```
Order A:  make_shared()  →  computePriority() throws  →  shared_ptr destructor runs → Widget freed ✅
Order B:  computePriority() throws  →  make_shared() never called → nothing to leak ✅
```

In both cases, either the Widget was never created, or it's safely inside a `shared_ptr` whose destructor will clean it up during stack unwinding. **No leak is possible.**

#### Is this still a problem in C++17?

**The specific interleaving leak is fixed in C++17**, but `make_shared` is still preferred.

C++17 introduced a stronger guarantee (§8.5.1.2 / [expr.call]):

> *The initialization of a parameter, including every associated value computation and side effect, is indeterminately sequenced with respect to that of any other parameter.*

This means: each function argument is **fully evaluated** (all its sub-expressions complete) before any sub-expression of another argument begins. The dangerous interleaving where `new Widget` runs, then `computePriority()` throws before `shared_ptr(...)` is constructed **can no longer happen**.

In C++17, the compiler must choose one of these two orderings:

```
Order A: [ new Widget → shared_ptr(raw) ] then [ computePriority() ]
Order B: [ computePriority() ] then [ new Widget → shared_ptr(raw) ]
```

In Order A, the `shared_ptr` is fully constructed before `computePriority()` is called. If it throws, the `shared_ptr`'s destructor cleans up.  
In Order B, `computePriority()` throws before `new Widget` ever runs. Nothing to clean up.

**However, `make_shared` is still preferred in C++17 for two other reasons:**

| Reason | Still applies in C++17? |
|---|---|
| Exception safety (interleaving leak) | ❌ Fixed by evaluation-order guarantee |
| Single allocation (performance + cache) | ✅ Still a real benefit |
| No type repetition (`auto` + make) | ✅ Still a readability benefit |

#### Safe pattern when you MUST use `new`

If `make_shared` can't be used (e.g., custom deleter), construct the `shared_ptr` on a **separate statement**:

```cpp
// ✅ Safe in all C++ versions — smart pointer created on its own statement
auto sp = std::shared_ptr<Widget>(new Widget, customDeleter);
processWidget(std::move(sp), computePriority());
```

The `shared_ptr` is fully constructed (owns the Widget) before `processWidget` is called. Even if `computePriority()` throws, `sp`'s destructor runs during stack unwinding.

---

## Q17. Single vs Dual Allocation

> *Describe the two memory allocation strategies for constructing a `shared_ptr` and their performance implications:*
>
> 1. `std::shared_ptr<Widget>(new Widget());`
> 2. `auto sp = std::make_shared<Widget>();`
>
> *What effect does each have on cache performance? On memory fragmentation?*

### Answer

#### Strategy 1: Dual allocation (`new` + `shared_ptr` constructor)

```cpp
std::shared_ptr<Widget> sp(new Widget());
```

Two separate heap allocations occur:

```
Allocation 1 (by new Widget()):
┌───────────────────────────────┐
│       Widget object           │  address: 0x1000
│       member1_, member2_, ... │
└───────────────────────────────┘

Allocation 2 (by shared_ptr constructor):
┌───────────────────────────────┐
│     Control Block             │  address: 0x5000 (possibly far away)
│     strong_count: 1           │
│     weak_count: 1             │
│     deleter: default_delete   │
│     allocator: std::allocator │
└───────────────────────────────┘
```

**Two calls to the allocator** (`operator new` or `malloc` internally). Two separate heap blocks, potentially on different cache lines, possibly far apart in the virtual address space.

#### Strategy 2: Single allocation (`make_shared`)

```cpp
auto sp = std::make_shared<Widget>();
```

One heap allocation holds both:

```
Single allocation (by make_shared):
┌───────────────────────────────────────────────────┐
│  Control Block           │    Widget object        │  address: 0x1000
│  strong_count: 1         │    member1_, member2_   │
│  weak_count: 1           │                         │
│  deleter: built-in       │                         │
│  allocator: std::alloc   │                         │
└───────────────────────────────────────────────────┘
         ~32–64 bytes               sizeof(Widget) bytes
```

One call to the allocator. Widget and control block are adjacent in memory.

#### Performance implications

**1. Allocator overhead**

Each heap allocation has a fixed cost: the allocator must search for a free block, update bookkeeping structures (free lists, page metadata), and potentially acquire a lock (in multi-threaded allocators like `ptmalloc2`, `jemalloc`, `tcmalloc`).

| Strategy | Allocator calls | Approximate overhead |
|---|---|---|
| Dual (`new` + `shared_ptr`) | 2 | 2× allocation bookkeeping, 2× possible lock acquisitions |
| Single (`make_shared`) | 1 | Half the allocator overhead |

On systems where allocation is expensive (embedded, real-time, high-frequency trading), halving the allocation count is significant.

**2. Cache performance**

Modern CPUs load memory in **cache lines** (typically 64 bytes). When the program accesses the Widget, the CPU fetches the cache line containing it. When `shared_ptr` is copied or destroyed, the CPU accesses the control block's reference count.

With **dual allocation**, these two accesses likely hit **different cache lines** (possibly different pages):

```
Cache line A: [ Widget ... ]          ← touched when you access the Widget
Cache line B: [ Control Block ... ]   ← touched when you copy/destroy the shared_ptr
```

With **single allocation**, the control block and the Widget are **adjacent** — often on the same cache line or neighbouring lines:

```
Cache line A: [ Control Block | Widget ... ]   ← one cache miss loads both
```

This means:
- When you dereference the `shared_ptr` (access Widget), the control block may already be in L1/L2 cache — **fewer cache misses**.
- When you copy the `shared_ptr` (access control block), the Widget data may already be cached — beneficial if you immediately read the Widget.

In tight loops that repeatedly create, copy, and dereference `shared_ptr`s, the cache-locality benefit of `make_shared` is measurable.

**3. Memory fragmentation**

Heap fragmentation occurs when many small allocations of varying sizes intersperse with freed blocks, leaving gaps too small to satisfy future requests.

| Strategy | Effect on fragmentation |
|---|---|
| Dual allocation | **More fragmentation.** Two small blocks per `shared_ptr`. When the Widget is freed but the control block lingers (due to `weak_ptr`s), a hole appears where the Widget was. |
| Single allocation | **Less fragmentation.** One larger block per `shared_ptr`. Fewer total allocations → less interleaving → more contiguous free space. |

**4. Deallocation timing (the trade-off)**

As discussed in Q15 (Item 20), `make_shared`'s single-allocation strategy has a downside: the Widget's memory can't be freed independently of the control block. If `weak_ptr`s outlive all `shared_ptr`s, the Widget's bytes (destructor already run, data gone) remain allocated until the last `weak_ptr` is also gone.

| Strategy | Widget memory freed | Control block memory freed |
|---|---|---|
| Dual allocation | Immediately when strong count → 0 | When weak count → 0 |
| Single allocation (`make_shared`) | Deferred until weak count → 0 | Same — whole block freed together |

For **small to medium objects**, the cache/allocation benefits of `make_shared` far outweigh the deferred-deallocation cost. For **very large objects** (megabytes) with long-lived `weak_ptr`s, use `new` + `shared_ptr` constructor to allow prompt memory release.

#### Summary

| Metric | `new` + `shared_ptr` | `make_shared` | Winner |
|---|---|---|---|
| Heap allocations | 2 | 1 | `make_shared` |
| Cache locality | Poor (separate blocks) | Good (adjacent) | `make_shared` |
| Fragmentation | Higher | Lower | `make_shared` |
| Allocator lock acquisitions | 2 | 1 | `make_shared` |
| Memory freed promptly (with `weak_ptr`) | ✅ Yes | ❌ No (deferred) | `new` |
| Custom deleter support | ✅ Yes | ❌ No | `new` |

---

## Q18. When `make_shared` / `make_unique` Cannot Be Used

> *List all limitations of the make functions. For each, provide the correct alternative.*

### Answer

There are **four limitations** where the make functions (`make_shared`, `make_unique`, `allocate_shared`) cannot be used, plus one design consideration that makes them undesirable.

---

### Limitation 1: Custom Deleters

**Problem:** `make_shared` and `make_unique` provide **no parameter** for specifying a custom deleter. They always use `delete` (or `delete[]` for array forms).

```cpp
auto customDel = [](Widget* p) {
    auditLog("Destroying widget", p->id());
    delete p;
};

// ❌ No deleter parameter available:
// auto sp = std::make_shared<Widget>(customDel);   // Won't compile — passes customDel as Widget constructor arg

// ✅ Correct alternative:
std::shared_ptr<Widget> sp(new Widget(), customDel);
std::unique_ptr<Widget, decltype(customDel)> up(new Widget(), customDel);
```

**Safe pattern (pre-C++17)**: construct on a separate statement to avoid the exception-safety issue:

```cpp
auto sp = std::shared_ptr<Widget>(new Widget(), customDel);  // one statement
process(std::move(sp), computePriority());                    // safe
```

---

### Limitation 2: Brace Initialization (`{}`) — `initializer_list` Construction

**Problem:** Make functions use **perfect forwarding** with parentheses `()`. They forward their arguments via `std::forward<Args>(args)...` to the constructor using `(args...)`, never `{args...}`. This means you cannot invoke constructors that are only reachable via braces.

```cpp
// std::vector<int> has two relevant constructors:
//   vector(size_t count, const int& value)   ← reachable via ()
//   vector(std::initializer_list<int>)        ← reachable via {}

auto sp1 = std::make_shared<std::vector<int>>(10, 20);
// Calls vector(10, 20) → 10 elements, each with value 20 ✅

// auto sp2 = std::make_shared<std::vector<int>>({10, 20});
// ❌ Compile error: braced-init-list cannot be deduced by template argument deduction
```

**Why it fails:** Template argument deduction cannot deduce a type from a braced-init-list `{10, 20}`. The make function's parameter pack `Args...` cannot bind to `{10, 20}` because it has no type.

**Workaround 1 — Create an `initializer_list` variable:**

```cpp
auto initList = {10, 20};  // auto deduces std::initializer_list<int>
auto sp = std::make_shared<std::vector<int>>(initList);  // ✅ forwards the initializer_list
```

**Workaround 2 — Use `new`:**

```cpp
std::shared_ptr<std::vector<int>> sp(new std::vector<int>{10, 20});  // ✅
```

**Workaround 3 — Construct the object and move it in:**

```cpp
auto sp = std::make_shared<std::vector<int>>(std::vector<int>{10, 20});  // ✅
// Constructs a temporary vector with {}, then move-constructs inside make_shared
```

---

### Limitation 3: Classes with Custom `operator new` / `operator delete`

**Problem:** Some classes define class-specific `operator new` and `operator delete` to control their allocation (e.g., using a memory pool, a specific arena, or aligned allocation for SIMD). These operators are designed to allocate exactly `sizeof(T)` bytes.

`make_shared` allocates a **single block** holding both the object and the control block. It calls the **global** `::operator new` (or the allocator's `allocate`) for a block of size `sizeof(T) + sizeof(ControlBlock)`. This **bypasses** the class-specific `operator new` entirely.

```cpp
class AlignedWidget {
public:
    // Custom allocator: ensures 64-byte alignment for SIMD
    static void* operator new(std::size_t size) {
        return aligned_alloc(64, size);   // exactly sizeof(AlignedWidget)
    }
    static void operator delete(void* ptr) {
        std::free(ptr);
    }
    // ...
};

// ❌ make_shared bypasses AlignedWidget::operator new
// Allocates sizeof(AlignedWidget) + sizeof(ControlBlock) via global ::operator new
auto sp = std::make_shared<AlignedWidget>();

// ✅ Correct: new AlignedWidget() calls the class-specific operator new
std::shared_ptr<AlignedWidget> sp(new AlignedWidget());
```

**Note:** `make_unique` does NOT have this problem — it calls `new T(args...)` internally, which invokes the class-specific `operator new` if defined. This limitation is specific to `make_shared` / `allocate_shared`.

---

### Limitation 4: Large Objects with Long-Lived `weak_ptr`s

**Problem:** As discussed in Q15 and Q17, `make_shared` colocates the object and the control block in a single allocation. The entire block cannot be freed until both the strong count AND the weak count reach zero. If the object is large and `weak_ptr`s are long-lived, the object's memory stays allocated (though the destructor has run) long after all `shared_ptr`s are gone.

```cpp
// 200 MB image buffer
auto img = std::make_shared<ImageBuffer>(width, height);

// Weak-ptr-based cache stores a reference
imageCache[key] = img;   // stores weak_ptr

img.reset();  // Last shared_ptr gone. Destructor runs.
              // But 200 MB + control block are STILL allocated
              // because imageCache holds a weak_ptr → control block alive → entire block alive
```

**Alternative:** Use separate allocations so the object memory is freed immediately:

```cpp
std::shared_ptr<ImageBuffer> img(new ImageBuffer(width, height));
// Object and control block are in separate heap blocks.
// When strong count → 0: ImageBuffer memory (200 MB) is freed immediately.
// Control block (~64 bytes) persists until weak_ptr expires.
```

---

### Limitation 5 (design consideration): Private / Protected Constructors

**Problem:** `make_shared` and `make_unique` call `new T(args...)` internally, from within a library function / helper. If `T`'s constructor is `private` or `protected`, make functions cannot access it — even if the *calling code* is a friend of `T`.

```cpp
class Singleton {
private:
    Singleton() = default;
    friend class Factory;
};

// Inside Factory (which is a friend):
auto sp = std::make_shared<Singleton>();  // ❌ Compile error: Singleton() is private
                                          // make_shared is not a friend!

// ✅ Alternative:
std::shared_ptr<Singleton> sp(new Singleton());  // OK — Factory can call new Singleton()
```

**Workaround:** Use the "passkey" idiom — a public constructor taking a private token type:

```cpp
class Singleton {
    struct Token {};   // private nested type
    friend class Factory;
public:
    explicit Singleton(Token) {}   // public constructor, but only friends can create Token
};

// Inside Factory:
auto sp = std::make_shared<Singleton>(Singleton::Token{});  // ✅
```

---

### Quick reference

| Limitation | Applies to | Alternative |
|---|---|---|
| Custom deleters | `make_shared`, `make_unique` | `shared_ptr(new T, deleter)` |
| Brace-init (`{}`) construction | `make_shared`, `make_unique` | Create `initializer_list` variable, or use `new` |
| Custom `operator new`/`delete` | `make_shared` only | `shared_ptr(new T())` |
| Large objects + long-lived `weak_ptr` | `make_shared` only | `shared_ptr(new T())` (separate allocations) |
| Private constructors | `make_shared`, `make_unique` | `shared_ptr(new T())` from friend, or passkey idiom |

---

## Q19. Implementing `make_unique` for C++11

> *`std::make_unique` was omitted from C++11. Write a conforming implementation. What C++14 features, if any, does your implementation rely on?*

### Answer

#### Why it was omitted

`std::make_unique` was not included in C++11 due to an oversight — it was noted as missing during standardization but was too late to add. It was formally introduced in C++14 (N3656, by Stephan T. Lavavej). The omission is explicitly called out in Item 21.

#### Basic implementation — single objects (C++11)

```cpp
// For non-array types: make_unique<Widget>(args...)
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

This relies **only on C++11 features**:
- Variadic templates (`typename... Args`, `Args&&... args`)
- Perfect forwarding (`std::forward<Args>(args)...`)
- Rvalue references

No C++14 features are needed for this basic form.

**Usage:**

```cpp
auto up1 = make_unique<Widget>();              // default constructor
auto up2 = make_unique<Widget>(42, "hello");   // parameterised constructor
auto up3 = make_unique<std::string>(10, 'x');  // "xxxxxxxxxx"
```

#### Complete implementation — arrays and deleted overloads

The C++14 standard defines three overloads. A conforming C++11 backport looks like:

```cpp
#include <memory>
#include <type_traits>

// SFINAE helper: true if T is not an array
// e.g., Widget → enabled
template<typename T>
using _NonArray = typename std::enable_if<!std::is_array<T>::value>::type;

// SFINAE helper: true if T is an unbounded array (T[])
// e.g., int[] → enabled
template<typename T>
using _UnboundedArray = typename std::enable_if<
    std::is_array<T>::value && std::extent<T>::value == 0
>::type;

// SFINAE helper: true if T is a bounded array (T[N])
// e.g., int[5] → enabled
template<typename T>
using _BoundedArray = typename std::enable_if<
    std::extent<T>::value != 0
>::type;

// ─── Overload 1: Non-array types ───
// make_unique<Widget>(args...) → unique_ptr<Widget>
template<typename T, typename... Args>
_NonArray<T>*,                      // SFINAE: only for non-arrays
std::unique_ptr<T>
make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// ─── Overload 2: Unbounded array types ───
// make_unique<int[]>(5) → unique_ptr<int[]> with 5 value-initialised ints
template<typename T>
_UnboundedArray<T>*,                // SFINAE: only for T[]
std::unique_ptr<T>
make_unique(std::size_t size) {
    using Element = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>(new Element[size]());  // value-initialised (zeroed)
}

// ─── Overload 3: Bounded array types — DELETED ───
// make_unique<int[5]>() is ill-formed
template<typename T, typename... Args>
_BoundedArray<T>*,                  // SFINAE: only for T[N]
void
make_unique(Args&&...) = delete;
```

However, the SFINAE above is verbose. The cleaner, commonly-used C++11 implementation uses `enable_if` as a return type:

```cpp
#include <memory>
#include <type_traits>

namespace cpp11 {

// Overload 1: Non-array
template<typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Overload 2: Unbounded array T[]
template<typename T>
typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0,
                        std::unique_ptr<T>>::type
make_unique(std::size_t size) {
    using Element = typename std::remove_extent<T>::type;
    return std::unique_ptr<T>(new Element[size]());
}

// Overload 3: Bounded array T[N] — deleted
template<typename T, typename... Args>
typename std::enable_if<std::extent<T>::value != 0, void>::type
make_unique(Args&&...) = delete;

}  // namespace cpp11
```

**Usage:**

```cpp
auto up1 = cpp11::make_unique<Widget>(42);         // ✅ single object
auto up2 = cpp11::make_unique<int[]>(100);          // ✅ array of 100 zeroed ints
// auto up3 = cpp11::make_unique<int[5]>();          // ❌ compile error (deleted)
```

#### Why the bounded-array overload is deleted

`make_unique<int[5]>()` is ill-formed because it's unclear what it should do. The user likely wants a `unique_ptr<int[5]>`, but `unique_ptr` does not specialise for bounded arrays — it only has specialisations for `T` and `T[]`. There is no `unique_ptr<int[5]>`. Deleting this overload gives a clear compiler error rather than a confusing substitution failure.

#### What the C++14 standard adds beyond this

The C++14 standard version (§20.9.1.4 [unique.ptr.create]) is essentially identical to the implementation above. C++14 itself introduces `std::enable_if_t` and `std::remove_extent_t` alias templates for cleaner syntax, but these are **convenience aliases**, not new functionality. The implementation above is fully expressible in C++11.

#### Does `make_unique` provide the allocation-efficiency benefit of `make_shared`?

**No.** `make_unique` calls `new T(args...)` — a single allocation for the object. There is no control block to colocate (since `unique_ptr` has no reference counting). The benefits of `make_unique` are:

| Benefit | `make_unique` | `make_shared` |
|---|---|---|
| No type repetition | ✅ | ✅ |
| Exception safety | ✅ | ✅ |
| Single allocation (performance) | N/A (no control block) | ✅ |

`make_unique`'s value is **code clarity and exception safety**, not allocation optimization.
