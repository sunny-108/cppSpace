# Item 22 — Pimpl Idiom with `unique_ptr` — Detailed Interview Answers

**Target:** Senior C++ Developer (10+ years C++, 14+ years overall)  
**Source:** Effective Modern C++ — Item 22

---

## Q20. Why Must the Destructor Be Defined in the `.cpp` File?

> ```cpp
> // widget.h
> class Widget {
> public:
>     Widget();
>     ~Widget() = default; // compiles? why or why not?
> private:
>     struct Impl;
>     std::unique_ptr<Impl> pImpl_;
> };
> ```
>
> *Explain the compilation error. What specific `static_assert` inside `unique_ptr`'s deleter triggers it? Why does the same code work fine with `shared_ptr`?*

### Answer

#### The compilation error

This code does **not** compile. The error looks like:

```
error: invalid application of 'sizeof' to an incomplete type 'Widget::Impl'
note: required from 'void std::default_delete<Widget::Impl>::operator()(Widget::Impl*) const'
```

#### Why it fails — step by step

**Step 1:** `~Widget() = default` in the header tells the compiler to generate the destructor **inline** — right here in the header, at the point of declaration.

**Step 2:** The generated destructor must destroy all data members, including `pImpl_` (a `unique_ptr<Impl>`).

**Step 3:** Destroying a `unique_ptr<Impl>` invokes its deleter. The default deleter is `std::default_delete<Impl>`, which calls `delete` on the `Impl*`.

**Step 4:** Inside `std::default_delete<T>::operator()`, the standard library implementation contains a `static_assert`:

```cpp
// Simplified from libstdc++ / libc++:
template<typename T>
struct default_delete {
    void operator()(T* ptr) const {
        static_assert(sizeof(T) > 0,
            "can't delete pointer to incomplete type");
        static_assert(!std::is_void<T>::value,
            "can't delete pointer to void");
        delete ptr;
    }
};
```

**Step 5:** At this point in the header, `Widget::Impl` is only **forward-declared** — it is an *incomplete type*. `sizeof(Impl)` is unknown. The `static_assert(sizeof(T) > 0)` fails.

**Why does the `static_assert` exist?** Calling `delete` on an incomplete type is technically allowed by C++ if the type has a trivial destructor, but it is undefined behaviour if the destructor is non-trivial. Since the library cannot know at compile time whether `Impl` has a trivial destructor, it conservatively requires the type to be complete.

#### The fix — declare in header, define in `.cpp`

```cpp
// widget.h
class Widget {
public:
    Widget();
    ~Widget();           // DECLARED only — no = default here
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// widget.cpp
#include "widget.h"

struct Widget::Impl {    // NOW Impl is fully defined (complete type)
    std::string name;
    std::vector<double> data;
};

Widget::Widget() : pImpl_(std::make_unique<Impl>()) {}
Widget::~Widget() = default;   // ✅ = default HERE is fine — Impl is complete
```

When the compiler processes `Widget::~Widget() = default` in the `.cpp` file, it can see the full definition of `Impl` (including `sizeof(Impl)`). The `static_assert` passes, and `delete` is called safely.

#### Why `shared_ptr` does NOT have this problem

```cpp
// widget.h — compiles fine with shared_ptr!
class Widget {
public:
    Widget();
    ~Widget() = default;   // ✅ OK with shared_ptr
private:
    struct Impl;
    std::shared_ptr<Impl> pImpl_;   // note: shared_ptr, not unique_ptr
};
```

The difference is **where the deleter is resolved**:

| Smart pointer | Deleter mechanism | When deleter needs complete type |
|---|---|---|
| `unique_ptr<T>` | Deleter is a **template parameter** (`default_delete<T>`). The deleter is invoked wherever `unique_ptr` is destroyed. | At every destruction site — including the **header** (for inline/defaulted destructors). |
| `shared_ptr<T>` | Deleter is **type-erased** into the control block. The deleter is captured at **construction** time, then stored as a function pointer / virtual call. | Only at **construction** time — which happens in the `.cpp` file. |

For `shared_ptr`:

1. **Construction** (`make_shared<Impl>()` in `widget.cpp`) — `Impl` is complete here. The deleter (which calls `delete`) is captured and stored in the control block. Type completeness is checked and satisfied.

2. **Destruction** (in the header, when `shared_ptr` is destroyed) — the `shared_ptr` merely decrements the reference count. If it drops to zero, it calls the **already-stored** deleter via an erased-type mechanism (virtual call or function pointer). The compiler does not need to instantiate `delete Impl*` here — it was already compiled into the control block's deleter at construction. No completeness check is needed.

```
unique_ptr destruction in header:
  unique_ptr<Impl>::~unique_ptr() → default_delete<Impl>::operator()(Impl*)
                                     → static_assert(sizeof(Impl) > 0)  ← FAILS

shared_ptr destruction in header:
  shared_ptr<Impl>::~shared_ptr() → decrement ref count
                                   → if count == 0: call stored_deleter(ptr)
                                     (stored_deleter was compiled in .cpp where Impl was complete)
                                     ← no sizeof check here — OK
```

---

## Q21. Complete Special Member Function List for Pimpl

> *When using `unique_ptr` Pimpl, which special member functions must be **declared** in the header and **defined** in the `.cpp`? Why does forgetting the move constructor cause a problem even if you declared the destructor?*

### Answer

#### Required special members declared in header, defined in `.cpp`

| Special member | Must declare in header? | Must define in `.cpp`? | Why |
|---|---|---|---|
| **Destructor** | ✅ Yes | ✅ Yes (`= default` in `.cpp`) | `unique_ptr<Impl>` needs complete type to call `delete` |
| **Move constructor** | ✅ Yes | ✅ Yes (`= default` in `.cpp`) | Moves the `unique_ptr`, which requires complete type (destructor of moved-from state) |
| **Move assignment** | ✅ Yes | ✅ Yes (`= default` in `.cpp`) | Destroys old `Impl` before taking new one — needs complete type |
| **Copy constructor** | ✅ If copyable | ✅ Yes (manual deep-copy) | Must create new `Impl` via `make_unique<Impl>(*rhs.pImpl_)` |
| **Copy assignment** | ✅ If copyable | ✅ Yes (manual deep-copy) | Must copy `Impl` contents: `*pImpl_ = *rhs.pImpl_` |

The minimal correct header (non-copyable widget):

```cpp
class Widget {
public:
    Widget();
    ~Widget();
    Widget(Widget&& rhs) noexcept;
    Widget& operator=(Widget&& rhs) noexcept;
    // ... public interface ...
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
```

And the `.cpp`:

```cpp
struct Widget::Impl { /* ... */ };

Widget::Widget() : pImpl_(std::make_unique<Impl>()) {}
Widget::~Widget() = default;
Widget::Widget(Widget&&) noexcept = default;
Widget& Widget::operator=(Widget&&) noexcept = default;
```

#### Why forgetting the move constructor is a problem

C++ has a rule (§15.8.1 / [class.copy.ctor]):

> *If the class definition declares a user-defined destructor, the compiler does NOT implicitly generate move constructor or move assignment operator.*

When you declare `~Widget();` (user-declared destructor), the compiler **suppresses** the implicit move operations. Without explicit move declarations, the class has:

- No move constructor (suppressed)
- No move assignment (suppressed)
- Copy constructor: compiler **tries** to generate it → fails because `unique_ptr` is non-copyable → **implicitly deleted**
- Copy assignment: same → **implicitly deleted**

Result: `Widget` becomes **non-movable and non-copyable**:

```cpp
Widget w1;
Widget w2 = std::move(w1);   // ❌ Compile error: no move constructor, copy is deleted
```

This is a particularly insidious bug because:
1. The class compiles fine — no error at the definition.
2. The error only appears at the **usage site**, potentially in a completely different file.
3. The error message says "use of deleted function `Widget(const Widget&)`" — confusing, because you never intended to copy, and the real issue is that move was never generated.

#### The cascade of suppression rules

```
You declare ~Widget()
    │
    ├──▶ Move constructor: SUPPRESSED (not generated)
    ├──▶ Move assignment:  SUPPRESSED (not generated)
    │
    ├──▶ Copy constructor: compiler tries implicit generation
    │       └──▶ tries to copy unique_ptr → DELETED
    │
    └──▶ Copy assignment: compiler tries implicit generation
            └──▶ tries to copy unique_ptr → DELETED

Result: Widget is non-movable, non-copyable → nearly useless
Solution: Explicitly declare + define move operations
```

#### Declaring all five in the header

For a fully functional Pimpl class (copyable + movable):

```cpp
class Widget {
public:
    Widget();
    ~Widget();

    Widget(Widget&& rhs) noexcept;
    Widget& operator=(Widget&& rhs) noexcept;

    Widget(const Widget& rhs);
    Widget& operator=(const Widget& rhs);

    // ... public interface ...
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
```

---

## Q22. Deep Copy for Pimpl

> *Write a correct copy constructor and copy assignment operator for a Pimpl class. What happens if you write `= default` for the copy in the header?*

### Answer

#### What happens with `= default` in the header

```cpp
// widget.h
class Widget {
public:
    Widget();
    ~Widget();
    Widget(const Widget&) = default;             // ❌ COMPILE ERROR
    Widget& operator=(const Widget&) = default;  // ❌ COMPILE ERROR
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
```

The compiler-generated copy constructor tries to copy each member. For `pImpl_` (a `unique_ptr`), it attempts to call `unique_ptr`'s copy constructor — which is **explicitly deleted**:

```
error: use of deleted function 'std::unique_ptr<Widget::Impl>::unique_ptr(const std::unique_ptr<Widget::Impl>&)'
note: declared here: unique_ptr(const unique_ptr&) = delete;
```

This fails at two levels:
1. `unique_ptr` is non-copyable (copy is deleted).
2. Even if it were copyable, `Impl` is incomplete in the header.

#### What happens with `= default` in the `.cpp`

```cpp
// widget.cpp
Widget::Widget(const Widget&) = default;   // Still ❌ — unique_ptr copy is deleted
```

Even in the `.cpp` where `Impl` is complete, the copy constructor is still deleted because `unique_ptr`'s copy is deleted. `= default` for copy **never works** with `unique_ptr` members.

#### Correct implementation — manual deep copy

```cpp
// widget.h
class Widget {
public:
    Widget();
    ~Widget();
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;
    Widget(const Widget& rhs);               // declared
    Widget& operator=(const Widget& rhs);    // declared
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
```

```cpp
// widget.cpp
struct Widget::Impl {
    std::string name;
    std::vector<double> data;
    int version = 0;
};

// Copy constructor — creates a NEW Impl that is a copy of rhs's Impl
Widget::Widget(const Widget& rhs)
    : pImpl_(std::make_unique<Impl>(*rhs.pImpl_))   // deep copy via Impl's copy ctor
{}

// Copy assignment — copies the CONTENTS of rhs's Impl into this->pImpl_
Widget& Widget::operator=(const Widget& rhs) {
    if (this != &rhs) {
        *pImpl_ = *rhs.pImpl_;   // deep copy via Impl's copy assignment
    }
    return *this;
}
```

#### How the deep copy works

**Copy constructor:**
- `*rhs.pImpl_` dereferences the `unique_ptr` → yields `Impl&` (the source object).
- `std::make_unique<Impl>(...)` calls `new Impl(...)` → invokes `Impl`'s **copy constructor**.
- A brand-new `Impl` is created on the heap with the same data as `rhs.pImpl_`.
- The new `unique_ptr` owns this new `Impl` — each `Widget` has its own independent `Impl`.

**Copy assignment:**
- `*pImpl_ = *rhs.pImpl_` dereferences both pointers and calls `Impl`'s **copy assignment operator**.
- The existing `Impl`'s members are overwritten with the values from `rhs`.
- No allocation / deallocation — the same `Impl` object is reused.

#### Self-assignment and null-safety

The self-assignment check (`if (this != &rhs)`) is a safeguard. For the copy assignment, it's not strictly required if `Impl`'s assignment is self-assignment-safe (which compiler-generated assignments are). But it's good practice and avoids unnecessary work.

For robustness against a null `pImpl_` (e.g., after a move), you might add:

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    if (this != &rhs) {
        if (rhs.pImpl_) {
            if (pImpl_) {
                *pImpl_ = *rhs.pImpl_;
            } else {
                pImpl_ = std::make_unique<Impl>(*rhs.pImpl_);
            }
        } else {
            pImpl_.reset();
        }
    }
    return *this;
}
```

However, in most Pimpl designs, `pImpl_` is never null (always initialised in the constructor). The simple version suffices.

#### What `Impl` needs for deep copy to work

`Impl` must be copyable — its copy constructor and copy assignment must work. Since `Impl` is a plain struct with standard library members (`string`, `vector`, etc.), the compiler-generated copy operations are correct. If `Impl` contained a `unique_ptr` member, you'd need to write `Impl`'s copy operations too.

---

## Q23. ABI Stability and Pimpl

> *A shared library (.so / .dylib) exposes a Pimpl-based class in its public header. The library maintainer adds three new data members to `Impl` and ships a new version. Will client code compiled against the old header still work without recompilation?*

### Answer

#### Yes — client code works without recompilation

This is one of the primary motivations for the Pimpl idiom in library design.

#### Why it works — ABI analysis

ABI (Application Binary Interface) compatibility requires that the following remain unchanged between library versions:

| ABI property | Changed? | Why |
|---|---|---|
| `sizeof(Widget)` | ❌ No | Widget contains only `unique_ptr<Impl>` — one pointer. Adding members to `Impl` does not change `sizeof(Widget)`. |
| Layout of `Widget` | ❌ No | `Widget`'s only data member is `pImpl_` — its offset is 0 (or after vptr if virtual). Does not change. |
| Vtable layout | ❌ No | No virtual functions were added or removed. If `Widget` has virtuals, their order is unchanged. |
| Name mangling of `Widget`'s methods | ❌ No | Public method signatures are unchanged. |
| `sizeof(Impl)` | ✅ Yes | Three new data members increase Impl's size. But this is **invisible** to client code. |
| Layout of `Impl` | ✅ Yes | Member offsets change. But client code never sees `Impl`'s layout — it's defined only in the `.cpp`. |

The key insight:

```
Client code sees:                    Library internals (not visible):
┌─────────────────────────┐          ┌─────────────────────────────┐
│  class Widget {         │          │  struct Widget::Impl {      │
│      struct Impl;       │  ◄───── │      string name;            │
│      unique_ptr<Impl>;  │  ptr    │      vector<double> data;    │
│      // public methods  │          │      int version;            │
│  };                     │          │      // NEW: 3 more members  │
│  sizeof(Widget) = 8     │          │  };                         │
└─────────────────────────┘          │  sizeof(Impl) = changed     │
                                     └─────────────────────────────┘
```

Client code only knows:
- `Widget` exists.
- `Widget` has a pointer-sized member.
- `Widget` has certain public methods with certain signatures.

None of these changed. The binary interface is preserved.

#### What clients must do

| Action | Required? |
|---|---|
| Recompile client code | ❌ No |
| Relink against new `.so` / `.dylib` | ✅ Yes (new function implementations) |
| Change any source code | ❌ No |

In practice: the library ships a new `.so` file. The application loads it (or the linker resolves it at start-up). The application's compiled code works identically — the pointer-sized `pImpl_` is initialised by the library's constructor (inside the `.so`), which now allocates a larger `Impl`. Everything functions correctly.

#### Contrast: without Pimpl

If `Widget` held data members directly in the header:

```cpp
class Widget {
    std::string name_;
    std::vector<double> data_;
    int version_;
    // Library adds: double newMember1_; string newMember2_; ...
};
```

Adding members changes `sizeof(Widget)`, member offsets, and potentially alignment. All client code must be **recompiled** — an ABI break.

#### Real-world significance

This is why Pimpl is ubiquitous in:
- **Qt** — `QObject` and derivatives use a `d_ptr` (Pimpl pointer) for ABI stability across minor releases.
- **KDE libraries** — strict ABI compatibility policies mandate Pimpl.
- **System libraries** — `libcurl`, `OpenSSL`, etc. use opaque pointers (C-style Pimpl) to maintain ABI across versions.
- **COM / CORBA** — interface-based designs are essentially Pimpl at the ABI level.

#### When ABI stability breaks even with Pimpl

Pimpl protects against **Impl-internal changes** but not against changes to Widget's **public interface**:

| Change | ABI break? |
|---|---|
| Add/remove members to `Impl` | ❌ Safe |
| Change types of `Impl` members | ❌ Safe |
| Add a new **public** method to `Widget` | ❌ Safe (new symbol, old symbols unchanged) |
| Remove or rename a public method | ✅ **Break** (client calls missing symbol) |
| Change a public method's signature | ✅ **Break** (name mangling changes) |
| Add a **virtual** method | ✅ **Break** (vtable layout changes) |
| Change `Widget` from non-virtual to virtual (add first virtual) | ✅ **Break** (adds vptr, changes `sizeof(Widget)`) |

---

## Q24. `unique_ptr` Pimpl vs `shared_ptr` Pimpl — Trade-offs

> *A colleague suggests using `shared_ptr<Impl>` for Pimpl to avoid declaring the destructor and move operations in the header. Argue for or against this approach. What semantic issue does `shared_ptr` Pimpl introduce?*

### Answer

#### What the colleague's approach looks like

```cpp
// widget.h — shared_ptr Pimpl
#include <memory>

class Widget {
public:
    Widget();
    // No ~Widget() needed
    // No move constructor needed
    // No move assignment needed
    // Default copy works (but does it do what you want?)
    void doSomething();
private:
    struct Impl;
    std::shared_ptr<Impl> pImpl_;   // shared_ptr instead of unique_ptr
};
```

This compiles without special member declarations. The colleague is correct that it's **more convenient**. But it introduces a **serious semantic defect**.

#### The semantic defect — shared state on copy

With `shared_ptr<Impl>`, the **default copy constructor** copies the `shared_ptr`, which means two `Widget` instances now **share the same `Impl`**:

```cpp
Widget w1;
w1.setName("Alice");

Widget w2 = w1;          // copies the shared_ptr — SHARES the same Impl
w2.setName("Bob");

std::cout << w1.getName();  // prints "Bob" — SURPRISE!
```

Both `w1` and `w2` point to the **same** `Impl` object:

```
w1.pImpl_ ──▶ ┌──────────────┐ ◀── w2.pImpl_
               │    Impl      │
               │  name="Bob"  │     ← mutation through w2 visible through w1
               └──────────────┘
               Control block: strong=2
```

This is **reference semantics** (like Java/Python objects) — not **value semantics** (what C++ programmers expect from classes without explicit sharing). Mutating one copy silently mutates the other.

With `unique_ptr<Impl>` and a proper deep-copy constructor, each `Widget` has its own `Impl`:

```
w1.pImpl_ ──▶ ┌──────────────┐
               │    Impl      │
               │ name="Alice" │     ← independent
               └──────────────┘

w2.pImpl_ ──▶ ┌──────────────┐
               │    Impl      │
               │  name="Bob"  │     ← independent
               └──────────────┘
```

#### Comprehensive trade-off table

| Aspect | `unique_ptr<Impl>` Pimpl | `shared_ptr<Impl>` Pimpl |
|---|---|---|
| **Header ceremony** | Must declare destructor + move ops | None — all compiler-generated |
| **Copy semantics** | Deep copy (correct value semantics) | Shallow copy (shared state — usually wrong) |
| **sizeof(Widget)** | 8 bytes (one pointer) | 16 bytes (two pointers: object + control block) |
| **Move performance** | One pointer swap | One pointer swap + atomic ref-count operations |
| **Destruction performance** | Direct `delete` | Atomic decrement + check; `delete` only when count → 0 |
| **Thread-safety implication** | No shared state — no synchronization needed | Shared `Impl` across copies requires synchronization |
| **Ownership clarity** | Exclusive — Widget owns its Impl | Ambiguous — who owns Impl? All copies? |
| **ABI overhead** | `sizeof(Widget)` = 8 bytes | `sizeof(Widget)` = 16 bytes |
| **Risk of subtle bugs** | Low — copy is explicit and correct | **High** — shared mutation is silent and unexpected |

#### The argument against `shared_ptr` Pimpl

1. **Value semantics violation:** A `Widget` should behave like a value. When you copy a `Widget`, the copy should be independent. `shared_ptr` Pimpl violates this fundamental expectation. This is not a theoretical concern — it produces real bugs that are extremely hard to diagnose (mutation through one object silently affects another).

2. **Performance:** `shared_ptr` adds an atomic reference-count increment on every copy and an atomic decrement on every destruction. For a Pimpl class that may be copied/moved frequently (stored in containers, passed by value), this overhead is unnecessary.

3. **Double the pointer size:** `shared_ptr` is 16 bytes vs `unique_ptr`'s 8 bytes. In arrays or tight structures, this doubles memory usage for the pointer.

4. **Unnecessary thread-safety overhead:** `shared_ptr`'s atomic ref counting adds overhead even in single-threaded code. `unique_ptr` has zero overhead.

5. **Hides a design decision:** Using `shared_ptr` for Pimpl communicates "the implementation is shared" — which is almost never the intent. It confuses future maintainers who may wonder if the sharing is intentional.

#### When `shared_ptr` Pimpl IS appropriate

There is **one** legitimate use case: when `Widget` objects are specifically designed to have **reference semantics** — i.e., copying a `Widget` should create a shallow copy that shares state. This is rare in C++ but occurs in:

- Handle types (e.g., `std::thread`, window handles) where copies refer to the same underlying resource.
- Copy-on-write (COW) implementations where the `shared_ptr` enables sharing until mutation occurs.

For COW, you'd implement conditional deep-copy:

```cpp
void Widget::mutate() {
    if (pImpl_.use_count() > 1) {
        pImpl_ = std::make_shared<Impl>(*pImpl_);  // deep copy before mutation
    }
    pImpl_->doMutation();
}
```

But this is an advanced pattern with its own pitfalls (thread-safety of `use_count()`, etc.) and should not be the default.

#### Final recommendation

> **Use `unique_ptr` for Pimpl.** The five extra lines in the header (destructor + two move declarations) are a trivially small cost for correct value semantics, smaller memory footprint, and zero reference-counting overhead. Use `shared_ptr` Pimpl only when you genuinely want reference semantics — and document that intent explicitly.
