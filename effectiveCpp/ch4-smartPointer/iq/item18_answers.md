# Item 18 — `std::unique_ptr` — Detailed Interview Answers

**Target:** Senior C++ Developer (10+ years C++, 14+ years overall)  
**Source:** Effective Modern C++ — Item 18

---

## Q1. Ownership Semantics and Internals

> *You have a factory function returning a `std::unique_ptr<Base>`. A colleague argues that returning by value is expensive because it copies the object. Explain why they are wrong, and what the compiler actually does. Also explain what happens to the `unique_ptr` itself during the return.*

### Answer

The colleague's concern is based on a pre-C++11 mental model. Returning a `std::unique_ptr` by value is **not expensive at all** — in fact it is the idiomatic and recommended way to return one. Here is why:

#### 1. `unique_ptr` cannot be copied — so the compiler never tries

`unique_ptr` deletes its copy constructor and copy assignment operator. The compiler knows it can only be moved. When you write:

```cpp
std::unique_ptr<Base> createWidget() {
    auto p = std::make_unique<Derived>();
    // ... configure ...
    return p;   // returning a local unique_ptr by value
}
```

the compiler applies one of two zero-cost mechanisms:

#### 2. NRVO (Named Return Value Optimisation)

Under NRVO, the compiler constructs `p` directly in the caller's return-value slot. No move constructor is ever called — the object is built in-place at the destination. The C++ standard explicitly permits this even though `unique_ptr` is move-only. In practice, every major compiler (GCC, Clang, MSVC) performs NRVO here.

With NRVO the generated code is equivalent to:

```cpp
// Pseudo-code of what the compiler does:
void createWidget(/*hidden*/ std::unique_ptr<Base>* __retSlot) {
    // construct directly in caller's memory
    new (__retSlot) std::unique_ptr<Base>(std::make_unique<Derived>());
}
```

Zero copies. Zero moves. The `unique_ptr` is born at the final destination.

#### 3. Implicit move when NRVO is not applied

If the compiler cannot apply NRVO (e.g., multiple return paths with different local variables), C++ mandates that a **return of a local variable is treated as an rvalue** (§15.8.3 / [class.copy.elision]). This means the move constructor of `unique_ptr` is called — not the copy constructor.

A `unique_ptr` move is trivially cheap:
- Copy the internal raw pointer (one pointer-sized integer).
- Set the source's internal pointer to `nullptr`.

That is one or two machine instructions. No heap allocation, no reference-count manipulation, no atomic operations.

#### 4. Zero overhead — same size as a raw pointer

With the default deleter (`std::default_delete<T>`), `sizeof(std::unique_ptr<T>)` equals `sizeof(T*)`. The default deleter is a stateless empty struct, and the compiler applies the **Empty Base Optimisation (EBO)** to compress it to zero bytes. So the "object being returned" is literally a single 8-byte pointer on a 64-bit platform.

#### 5. What happens to the managed object (the `Derived`)?

The managed `Derived` object is **heap-allocated once** (by `make_unique`) and **never moved or copied**. Only the pointer to it travels. The `unique_ptr` wrapper around that pointer is either elided (NRVO) or moved (one register copy).

#### Summary

| Step | Cost |
|---|---|
| Heap allocation of `Derived` | 1 allocation (unavoidable — it's dynamically allocated) |
| Return of `unique_ptr` | 0 cost (NRVO) or ~2 instructions (move) |
| Copy of `unique_ptr` | Impossible — deleted. Compiler enforces this. |
| Copy of `Derived` object | Never happens. Only the pointer travels. |

The colleague's intuition ("returning by value is expensive") applies to large value types before C++11. For move-only types like `unique_ptr`, returning by value is the cheapest and safest option.

---

## Q2. Custom Deleters and Type Impact

> *What is `sizeof(up1)` vs `sizeof(up2)` on a typical 64-bit platform? Explain why they differ, and how the Empty Base Optimisation (EBO) applies.*
>
> ```cpp
> auto d1 = [](Widget* p) { log(p); delete p; };
> auto d2 = [x = 42](Widget* p) { delete p; };
>
> std::unique_ptr<Widget, decltype(d1)> up1(new Widget, d1);
> std::unique_ptr<Widget, decltype(d2)> up2(new Widget, d2);
> ```

### Answer

#### Sizes

| Variable | `sizeof` | Explanation |
|---|---|---|
| `up1` | **8 bytes** | Same as a raw `Widget*` |
| `up2` | **16 bytes** | Pointer (8) + captured `int` (4) + padding (4) |

#### Why `up1` is 8 bytes — Empty Base Optimisation (EBO)

`d1` is a **stateless (non-capturing) lambda**. Every non-capturing lambda in C++ is translated by the compiler into an unnamed struct with:
- No data members
- An `operator()` member function

```cpp
// What the compiler generates for d1 (conceptual):
struct __lambda_d1 {
    // no data members — sizeof == 1 (empty struct rule), but EBO can compress to 0
    void operator()(Widget* p) const { log(p); delete p; }
};
```

`std::unique_ptr` typically stores the deleter using a **compressed pair** (similar to `boost::compressed_pair` or libstdc++'s `__compressed_pair`). When the deleter is an empty type, the compressed pair applies EBO: the empty deleter is stored as a base class of the pair, contributing **zero bytes** to the total size.

Result:

```
unique_ptr<Widget, __lambda_d1> layout:
┌──────────────┐
│ Widget* ptr  │  ← 8 bytes
│ (deleter)    │  ← 0 bytes (EBO — empty base, no storage)
└──────────────┘
Total: 8 bytes = sizeof(Widget*)
```

#### Why `up2` is 16 bytes — Capturing lambda has state

`d2` captures `x = 42` by value. The compiler generates a struct with a data member:

```cpp
// What the compiler generates for d2 (conceptual):
struct __lambda_d2 {
    int x;   // captured variable — 4 bytes
    void operator()(Widget* p) const { delete p; }
};
```

`sizeof(__lambda_d2)` is 4 bytes (the `int`). Now the compressed pair stores:
- The `Widget*` pointer: 8 bytes
- The deleter object (containing `int x`): 4 bytes
- Padding for alignment: 4 bytes (pointer alignment is 8)

```
unique_ptr<Widget, __lambda_d2> layout:
┌──────────────┐
│ Widget* ptr  │  ← 8 bytes
│ int x (= 42) │  ← 4 bytes
│ (padding)    │  ← 4 bytes (alignment to 8)
└──────────────┘
Total: 16 bytes
```

#### What about function-pointer deleters?

For completeness, if you used a function pointer instead of a lambda:

```cpp
void deleter(Widget* p) { delete p; }
std::unique_ptr<Widget, void(*)(Widget*)> up3(new Widget, deleter);
```

`sizeof(up3)` = **16 bytes** — always. A function pointer is a real 8-byte value; EBO cannot apply because it is not an empty type.

#### Summary table

| Deleter kind | Example | `sizeof(unique_ptr)` | EBO applies? |
|---|---|---|---|
| Default deleter | `unique_ptr<T>` | 8 | Yes (empty struct) |
| Stateless lambda | `[](T* p){ delete p; }` | 8 | Yes (empty struct) |
| Capturing lambda (small) | `[x](T* p){ delete p; }` | 16 | No (has data) |
| Function pointer | `void(*)(T*)` | 16 | No (8-byte value) |
| `std::function<void(T*)>` | — | 40+ | No (heavy type-erased wrapper) |

#### Practical implication

When you have millions of `unique_ptr` objects (e.g., nodes in a tree), the deleter choice affects total memory consumption. Prefer stateless lambdas or the default deleter to maintain the zero-overhead guarantee.

---

## Q3. `unique_ptr<T>` vs `unique_ptr<T[]>`

> *When would you use `unique_ptr<T[]>` instead of `unique_ptr<T>`? What changes in the API? In modern C++, when is `unique_ptr<T[]>` actually justified vs. `std::vector` or `std::array`?*

### Answer

#### API differences

`unique_ptr<T>` and `unique_ptr<T[]>` are separate **template specialisations** with different APIs:

| Feature | `unique_ptr<T>` | `unique_ptr<T[]>` |
|---|---|---|
| Dereference `*up` | ✅ Yes | ❌ Deleted |
| Arrow `up->member` | ✅ Yes | ❌ Deleted |
| Index `up[i]` | ❌ Not available | ✅ Yes |
| Destruction | Calls `delete ptr` | Calls `delete[] ptr` |
| Implicit conversion to `unique_ptr<Base>` | ✅ Yes | ❌ No (arrays don't support polymorphic base) |
| `make_unique` | `make_unique<T>(args...)` | `make_unique<T[]>(size)` (value-initialised) |

The critical distinction is **which `delete` is called**. Using `unique_ptr<T>` on an array allocated with `new[]` is undefined behaviour — `delete` is called instead of `delete[]`, which may not invoke destructors for all elements and corrupts the heap.

#### When `unique_ptr<T[]>` is justified

In modern C++, `unique_ptr<T[]>` is a **niche tool**. Prefer `std::vector` or `std::array` in almost all cases. The justified scenarios are:

**1. C-interop / FFI boundaries**

When a C API returns an owning pointer to an array:

```cpp
// C library returns malloc'd buffer
extern "C" float* get_sensor_data(size_t* out_count);

size_t count;
float* raw = get_sensor_data(&count);

// Wrap immediately — custom deleter for malloc'd memory
std::unique_ptr<float[], decltype(&std::free)> data(raw, std::free);

for (size_t i = 0; i < count; ++i)
    process(data[i]);
```

You cannot use `std::vector` here without copying because vector always manages its own allocation.

**2. Fixed-size runtime buffer where `vector`'s resizability is undesirable**

In safety-critical or deterministic-performance code, `vector`'s ability to reallocate on `push_back` may be a hazard. A `unique_ptr<T[]>` with a known size provides a non-resizable buffer:

```cpp
auto buffer = std::make_unique<char[]>(4096);  // exactly 4096 bytes, cannot grow
read(fd, buffer.get(), 4096);
```

**3. Interfacing with APIs that require `T*` + size**

Some C++ libraries or system APIs expect a raw contiguous array. While `vector::data()` also works, `unique_ptr<T[]>` signals "I own a fixed-size buffer" more explicitly.

**When NOT to use `unique_ptr<T[]>`:**

- When you need to know the size — `unique_ptr<T[]>` does not store the element count. You must track it separately.
- When you need iteration — no `begin()`/`end()`, no range-for support out of the box.
- When you need bounds checking — `operator[]` does not check bounds.
- When you need dynamic growth — use `vector`.
- When size is compile-time known — use `std::array`.

#### Rule of thumb

```
Need dynamic array?
├── Size known at compile time?    → std::array<T, N>
├── Resizable?                     → std::vector<T>
├── C-interop / no-copy ownership? → std::unique_ptr<T[]>
└── Otherwise                      → std::vector<T>  (default choice)
```

---

## Q4. Double Ownership from Raw Pointer

> *A legacy codebase passes `Widget*` around freely. During migration to smart pointers, a developer writes:*
>
> ```cpp
> Widget* raw = getWidget();
> std::unique_ptr<Widget> up1(raw);
> std::unique_ptr<Widget> up2(raw);
> ```
>
> *What happens at runtime? How would you structure the migration to prevent this class of bug entirely?*

### Answer

#### What happens at runtime

This is **undefined behaviour** — specifically a **double-free** bug:

1. `up1` and `up2` each think they exclusively own the `Widget` at address `raw`.
2. Each has its own independent deleter.
3. When `up2` goes out of scope (or at the end of the block), it calls `delete raw`. The `Widget` is destroyed and the memory is returned to the heap.
4. When `up1` goes out of scope, it also calls `delete raw`. This deletes **already-freed memory**.

Consequences (all undefined):
- Heap corruption — the allocator's internal bookkeeping is damaged.
- Crash (segfault / SIGABRT) — the allocator detects the double-free and aborts.
- Silent memory corruption — freed memory may have been reallocated for another object; `delete` destroys *that* object instead.
- Security vulnerability — in adversarial contexts, double-free is a well-known exploitation primitive.

On most modern platforms with debug allocators (ASan, `_GLIBCXX_DEBUG`), this will crash immediately with a diagnostic. In release builds, it may silently corrupt memory and crash later in an unrelated location.

#### Migration strategy to prevent this class of bug

**Phase 1: Ownership at the source — factories return `unique_ptr`**

The most impactful change is to make new/factory functions return `unique_ptr` instead of raw pointers:

```cpp
// BEFORE (legacy):
Widget* getWidget() { return new Widget(); }

// AFTER (migrated):
std::unique_ptr<Widget> getWidget() { return std::make_unique<Widget>(); }
```

Now there is no raw pointer to accidentally wrap twice. The `unique_ptr` is the one-and-only owner from birth.

**Phase 2: Audit and eliminate all raw `new` / `delete`**

Search the codebase for every `new` and `delete`:

```bash
grep -rn '\bnew\b' src/ | grep -v make_unique | grep -v make_shared
grep -rn '\bdelete\b' src/
```

Each site is a candidate for conversion. The goal: zero raw `new`/`delete` outside of allocator implementations.

**Phase 3: Use `release()` deliberately for ownership transfer to C APIs**

When you must hand a raw pointer to a C library that takes ownership:

```cpp
auto widget = std::make_unique<Widget>();
c_library_take_ownership(widget.release());  // explicit, intentional
```

`release()` makes it clear in code review that ownership is leaving the smart pointer.

**Phase 4: Static analysis tooling**

Integrate clang-tidy checks:
- `modernize-make-unique` — flags `unique_ptr(new T)` and suggests `make_unique`.
- `misc-use-after-move` — flags use of a moved-from `unique_ptr`.
- `clang-analyzer-cplusplus.NewDelete` — detects double-free patterns.
- `cppcoreguidelines-owning-memory` — flags raw owning pointers.

**Phase 5: Coding guidelines**

Establish rules:
- Never construct a `unique_ptr` from a raw pointer that you did not just `new` yourself or receive from `release()`.
- If a function receives `Widget*`, it is a **non-owning** observer. It must never wrap it in a smart pointer.
- Ownership transfer is signalled exclusively by passing `unique_ptr` by value.

```cpp
// Ownership transfer (sink):
void takeOwnership(std::unique_ptr<Widget> w);

// Non-owning access (observer):
void observe(const Widget& w);      // preferred
void observe(Widget* w);            // nullable observer
```

---

## Q5. `release()` vs `reset()` vs `get()`

> *Explain the semantic difference between `release()`, `reset()`, and `get()` on a `unique_ptr`. Give a concrete scenario where `release()` is the correct choice. Why is calling `get()` and then `delete` on the returned pointer a bug?*

### Answer

#### Semantic overview

| Method | Returns | Ownership after call | Object destroyed? |
|---|---|---|---|
| `get()` | Raw `T*` | `unique_ptr` **still owns** | No |
| `release()` | Raw `T*` | `unique_ptr` **gives up** ownership (becomes `nullptr`) | No — **caller must delete** |
| `reset()` | `void` | `unique_ptr` destroys old, optionally takes new | **Yes** (old object) |
| `reset(new_ptr)` | `void` | `unique_ptr` destroys old, takes `new_ptr` | **Yes** (old object) |

#### `get()` — "Let me peek at the address, but I still own it"

```cpp
auto up = std::make_unique<Widget>();
Widget* raw = up.get();    // raw points to the Widget
raw->doSomething();        // OK — object is alive
// up still owns the Widget
// raw is a non-owning observer
```

`get()` is used when you need to pass the raw pointer to an API that requires `T*` but does **not** take ownership (e.g., a C library function, a legacy interface).

#### `release()` — "I'm giving up ownership; someone else must clean up"

```cpp
auto up = std::make_unique<Widget>();
Widget* raw = up.release();   // up is now nullptr
// raw owns the Widget — you MUST delete it (or hand it to another owner)
delete raw;                   // your responsibility now
```

#### `reset()` — "Destroy what I have, optionally take something new"

```cpp
auto up = std::make_unique<Widget>(1);
up.reset();                   // Widget(1) is destroyed; up is nullptr
up.reset(new Widget(2));      // up now owns Widget(2)
up.reset(new Widget(3));      // Widget(2) is destroyed; up now owns Widget(3)
up = nullptr;                 // equivalent to up.reset(); Widget(3) is destroyed
```

#### Concrete scenario where `release()` is correct

**Handing ownership to a C API:**

Many C libraries follow the pattern "create → use → destroy" with raw pointers:

```cpp
// C library API:
// sqlite3_blob* sqlite3_blob_open(...);   — creates resource
// void sqlite3_blob_close(sqlite3_blob*); — destroys resource

// C++ wrapper creates the resource as unique_ptr:
auto blob = std::unique_ptr<sqlite3_blob, decltype(&sqlite3_blob_close)>(
    sqlite3_blob_open(db, ...),
    sqlite3_blob_close
);

// Later, another C function wants to take ownership of the blob:
c_library_adopt_blob(blob.release());
// blob is now nullptr; C library is responsible for cleanup
```

Another common case: building a node for a C-style linked list or tree:

```cpp
auto node = std::make_unique<Node>();
node->data = compute();
// Hand to the C tree library that takes ownership:
tree_insert(tree, node.release());
```

#### Why `get()` + `delete` is a bug

```cpp
auto up = std::make_unique<Widget>();
Widget* raw = up.get();
delete raw;        // ❌ BUG!
// raw is deleted, but up still thinks it owns the Widget.
// When up goes out of scope, it calls delete again → DOUBLE FREE.
```

After `delete raw`, the memory is freed. But `up` still holds the same pointer value. When `up`'s destructor runs, it calls `delete` on dangling memory. This is the same double-free UB as the previous question.

The correct alternative depends on intent:
- If you want to destroy the object early: `up.reset();`
- If you want to transfer ownership out: `Widget* raw = up.release();`

**Never call `delete` on a pointer obtained via `get()`.**

#### Decision guide

```
I want to…
├── Pass raw pointer to non-owning API → use get()
├── Hand ownership to a C API          → use release()  (caller deletes)
├── Destroy the current object         → use reset()
├── Replace with a different object    → use reset(new_ptr)
└── Delete the raw pointer manually    → NEVER do this
```

---

## Q6. `unique_ptr` Conversion to `shared_ptr`

> ```cpp
> std::unique_ptr<Widget> up = std::make_unique<Widget>();
> std::shared_ptr<Widget> sp = std::move(up);
> ```
>
> *Why does this compile? Can you go the other direction (`shared_ptr` → `unique_ptr`)? What design pattern in Effective Modern C++ recommends factories returning `unique_ptr` instead of `shared_ptr`?*

### Answer

#### Why `unique_ptr` → `shared_ptr` compiles

`std::shared_ptr<T>` has a **converting move constructor** that accepts `std::unique_ptr<T, Deleter>&&`:

```cpp
// In <memory> (simplified):
template<typename T>
class shared_ptr {
public:
    template<typename U, typename Deleter>
    shared_ptr(std::unique_ptr<U, Deleter>&& uptr);
    // ...
};
```

This constructor:
1. Takes the raw pointer from the `unique_ptr` (effectively calling `uptr.release()` internally).
2. Allocates a **control block** containing:
   - Strong reference count (initialised to 1)
   - Weak reference count (initialised to 0)
   - The deleter (moved from the `unique_ptr`'s deleter)
3. Sets the `unique_ptr` to `nullptr`.

The conversion is **safe** because exclusive ownership (one owner) can always be widened to shared ownership (one-or-more owners). The process is:

```
Before:  up ──owns──▶ Widget
         sp = (empty)

After:   up = nullptr
         sp ──owns──▶ Widget (+ new control block, ref count = 1)
```

If the `unique_ptr` had a custom deleter, that deleter is **preserved** inside the `shared_ptr`'s control block and will be used when the last `shared_ptr` releases the object.

#### Why `shared_ptr` → `unique_ptr` does NOT compile

```cpp
std::shared_ptr<Widget> sp = std::make_shared<Widget>();
std::unique_ptr<Widget> up = std::move(sp);   // ❌ Compile error
```

This is **logically impossible**, not just a library restriction:

- `shared_ptr` allows **multiple owners**. When you try to convert to `unique_ptr`, you're claiming exclusive ownership — but other `shared_ptr` copies may still exist and share the same object.
- There is no way to atomically "steal" the object from all shared owners.
- Even if `sp.use_count() == 1` at this moment, the compiler cannot guarantee that another thread won't copy `sp` between the check and the conversion.

The type system correctly prevents this at compile time. There is no standard library mechanism to go from shared → exclusive ownership.

#### The factory-return pattern from Effective Modern C++

Item 18 recommends that **factory functions return `std::unique_ptr<T>`** rather than `std::shared_ptr<T>`:

```cpp
// ✅ Recommended: return unique_ptr
std::unique_ptr<Investment> makeInvestment(/* params */) {
    // ... create the right derived type ...
    return std::make_unique<Stock>(/* ... */);
}
```

**Why this is superior to returning `shared_ptr`:**

**1. Maximum flexibility for the caller:**

```cpp
// Caller who wants exclusive ownership — zero overhead:
auto exclusive = makeInvestment(params);

// Caller who wants shared ownership — just convert:
std::shared_ptr<Investment> shared = makeInvestment(params);

// Caller who wants to store in a container of shared_ptr:
std::vector<std::shared_ptr<Investment>> portfolio;
portfolio.push_back(makeInvestment(params));   // implicit conversion
```

If the factory returned `shared_ptr`, callers who only need exclusive ownership would pay for the control block allocation and atomic reference counting for nothing.

**2. Efficiency — avoid unnecessary overhead:**

| Factory returns | Caller needs `unique_ptr` | Caller needs `shared_ptr` |
|---|---|---|
| `unique_ptr` | ✅ Direct use — zero overhead | ✅ Move-convert — one extra alloc (control block) |
| `shared_ptr` | ❌ Cannot convert back — stuck with overhead | ✅ Direct use |

Returning `unique_ptr` is **strictly more general**. It serves both audiences. Returning `shared_ptr` locks out the exclusive-ownership audience.

**3. Self-documenting ownership:**

A factory returning `unique_ptr` communicates: *"The factory creates an object and gives you sole ownership. What you do with it is your choice."*

A factory returning `shared_ptr` communicates: *"This object is meant to be shared"* — which may not be true. It bakes a premature design decision into the API.

#### Complete example

```cpp
#include <memory>
#include <iostream>

class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() const = 0;
};

class Dog : public Animal {
public:
    void speak() const override { std::cout << "Woof!\n"; }
};

// Factory returns unique_ptr — maximally flexible
std::unique_ptr<Animal> makeAnimal(const std::string& type) {
    if (type == "dog") return std::make_unique<Dog>();
    return nullptr;
}

int main() {
    // Use case 1: exclusive ownership — zero overhead
    auto pet = makeAnimal("dog");
    pet->speak();

    // Use case 2: shared ownership — seamless conversion
    std::shared_ptr<Animal> sharedPet = makeAnimal("dog");
    auto anotherRef = sharedPet;  // now two shared owners
    anotherRef->speak();

    // Use case 3: store in vector of shared_ptr
    std::vector<std::shared_ptr<Animal>> zoo;
    zoo.push_back(makeAnimal("dog"));  // unique_ptr → shared_ptr conversion

    return 0;
}
```

#### Key takeaway

> Return `unique_ptr` from factories. It gives callers the freedom to choose exclusive or shared ownership at the call site, without paying for features they don't need. This is the **"start narrow, widen if needed"** principle applied to ownership semantics.
