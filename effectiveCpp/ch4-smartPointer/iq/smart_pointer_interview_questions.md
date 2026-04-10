# Smart Pointers — Senior C++ Interview Questions

**Target:** C++ Developer with 10+ years C++ experience, 14+ years overall  
**Source:** Effective Modern C++ — Chapter 4 (Items 18–22)

---

## Section 1: `std::unique_ptr` (Item 18)

### Q1. Ownership semantics and internals

You have a factory function returning a `std::unique_ptr<Base>`. A colleague argues that returning by value is expensive because it copies the object. Explain why they are wrong, and what the compiler actually does. Also explain what happens to the `unique_ptr` itself during the return.

**Expected depth:** RVO / NRVO, implicit move on return, zero-overhead abstraction, `unique_ptr` being the same size as a raw pointer (default deleter).

---

### Q2. Custom deleters and type impact

```cpp
auto d1 = [](Widget* p) { log(p); delete p; };
auto d2 = [x = 42](Widget* p) { delete p; };

std::unique_ptr<Widget, decltype(d1)> up1(new Widget, d1);
std::unique_ptr<Widget, decltype(d2)> up2(new Widget, d2);
```

What is `sizeof(up1)` vs `sizeof(up2)` on a typical 64-bit platform? Explain why they differ, and how the empty base optimisation (EBO) applies.

**Expected depth:** stateless lambda → EBO collapses deleter to zero bytes → same size as raw pointer. Capturing lambda has non-zero size → `unique_ptr` grows. Function-pointer deleter is always one pointer extra.

---

### Q3. `unique_ptr<T>` vs `unique_ptr<T[]>`

When would you use `unique_ptr<T[]>` instead of `unique_ptr<T>`? What changes in the API? In modern C++, when is `unique_ptr<T[]>` actually justified vs. `std::vector` or `std::array`?

**Expected depth:** `operator[]` available on array form, `operator*` / `operator->` deleted. `delete[]` called instead of `delete`. Justified for C-interop or when the array size is runtime-determined but should not be resizable.

---

### Q4. Double ownership from raw pointer

A legacy codebase passes `Widget*` around freely. During migration to smart pointers, a developer writes:

```cpp
Widget* raw = getWidget();
std::unique_ptr<Widget> up1(raw);
std::unique_ptr<Widget> up2(raw);
```

What happens at runtime? How would you structure the migration to prevent this class of bug entirely?

**Expected depth:** double-free / undefined behaviour. Strategies: have factory functions return `unique_ptr` directly, audit all `new` call sites, use `release()` deliberately, compile-time ownership annotations, static analysis tooling (clang-tidy).

---

### Q5. `release()` vs `reset()` vs `get()`

Explain the semantic difference between `release()`, `reset()`, and `get()` on a `unique_ptr`. Give a concrete scenario where `release()` is the correct choice. Why is calling `get()` and then `delete` on the returned pointer a bug?

**Expected depth:** `release()` surrenders ownership and returns raw pointer (caller must delete). `reset()` destroys the held object (or replaces it). `get()` returns raw pointer but `unique_ptr` still owns it. Legitimate `release()` use: handing off to a C API that takes ownership.

---

### Q6. `unique_ptr` conversion to `shared_ptr`

```cpp
std::unique_ptr<Widget> up = std::make_unique<Widget>();
std::shared_ptr<Widget> sp = std::move(up);
```

Why does this compile? Can you go the other direction (`shared_ptr` → `unique_ptr`)? What design pattern in Effective Modern C++ recommends factories returning `unique_ptr` instead of `shared_ptr`?

**Expected depth:** implicit conversion via move constructor of `shared_ptr`. Reverse is impossible — can't extract exclusive ownership from shared ownership. Returning `unique_ptr` from factories is more flexible: caller can convert to `shared_ptr` if needed, or keep exclusive ownership.

---

## Section 2: `std::shared_ptr` (Item 19)

### Q7. Control block layout and creation rules

Draw (or describe) the memory layout of a `shared_ptr` and its control block. List *all* the scenarios that trigger creation of a new control block.

**Expected depth:** `shared_ptr` = { object pointer, control-block pointer }. Control block = { strong count, weak count, deleter, allocator }. New control block created by: (1) `make_shared`, (2) constructing from raw pointer, (3) constructing from `unique_ptr`. Copying a `shared_ptr` does NOT create a new control block.

---

### Q8. Atomic reference counting — costs and guarantees

`shared_ptr`'s reference count is atomically incremented/decremented. Does that make the *managed object* itself thread-safe? Explain the performance implications of passing `shared_ptr` by value in a hot loop vs by `const&`.

**Expected depth:** Atomic ref-count ops ≠ thread-safe object access. Passing by value = atomic increment + decrement per call. In tight loops this can be a measurable bottleneck. Prefer `const shared_ptr<T>&` or raw `T*`/`T&` when ownership transfer is not needed. Benchmark proof: atomic operations generate `lock` prefix instructions on x86.

---

### Q9. Creating `shared_ptr` from `this`

```cpp
class Node {
public:
    std::shared_ptr<Node> getShared() {
        return std::shared_ptr<Node>(this);
    }
};
```

What is catastrophically wrong here? How does `std::enable_shared_from_this` fix it? What happens if `shared_from_this()` is called before any `shared_ptr` to the object exists?

**Expected depth:** Creates a second control block → double-free. `enable_shared_from_this` stores a `weak_ptr` to `this` that is set when the first `shared_ptr` is created. `shared_from_this()` then calls `lock()` on that internal `weak_ptr`. If no `shared_ptr` exists yet, throws `std::bad_weak_ptr` (C++17) or is undefined (C++11/14).

---

### Q10. Custom deleters — type erasure advantage

With `unique_ptr`, the deleter is part of the type. With `shared_ptr`, it is not. What practical benefit does this give you? Show a code example where this matters.

**Expected depth:** All `shared_ptr<Widget>` are the same type regardless of deleter, so they can be stored in the same container, passed to the same function, or assigned to each other. Example: a heterogeneous vector of `shared_ptr<Widget>` where some use logging deleters and others use pool-return deleters.

---

### Q11. Aliasing constructor

What is the aliasing constructor of `shared_ptr`? Give a real-world example where it is useful. What does the aliasing form do to the strong reference count?

**Expected depth:** `shared_ptr<T> alias(shared_ptr<U> owner, T* subobject)` — shares ownership of the owner but points to a subobject. Strong count is shared (incremented). Example: exposing a member of a class while keeping the parent alive.

---

## Section 3: `std::weak_ptr` (Item 20)

### Q12. Dangling detection and the `lock()` pattern

Why is the following pattern unsafe in a multithreaded environment?

```cpp
if (!wp.expired()) {
    auto sp = wp.lock();
    sp->doSomething();
}
```

Rewrite it correctly and explain the atomicity guarantee of `lock()`.

**Expected depth:** Between `expired()` and `lock()`, another thread may destroy the last `shared_ptr`, making the object go away. `lock()` atomically checks liveness and increments the strong count in a single operation. Correct pattern: `if (auto sp = wp.lock()) { sp->doSomething(); }`.

---

### Q13. Circular reference detection

A doubly-linked list uses `shared_ptr` for both `next` and `prev`. Nobody can see the leak in testing — all unit tests pass, Valgrind shows no *invalid reads* — yet memory usage grows over time. Explain what is happening and how to fix it.

**Expected depth:** Circular `shared_ptr` references keep each other alive: strong count never reaches 0. Valgrind may not flag this because the memory is still "reachable." Fix: use `weak_ptr` for the back-link (`prev`). This breaks the cycle so the strong count can reach 0.

---

### Q14. `weak_ptr` for caching

Design a thread-safe `weak_ptr`-based cache for expensive-to-create objects. What happens when a cache lookup finds an expired `weak_ptr`? What is the trade-off vs a `shared_ptr`-based cache?

**Expected depth:** Cache stores `unordered_map<Key, weak_ptr<Value>>`. On lookup: `lock()` the `weak_ptr`. If empty, re-create and store a new `weak_ptr`. Trade-off: a `shared_ptr`-based cache keeps objects alive indefinitely (possible memory bloat); a `weak_ptr` cache allows objects to be freed when no external owner exists. Thread-safety: protect the map with a mutex; the `lock()` call itself is already atomic.

---

### Q15. Control block lifetime with `weak_ptr`

After all `shared_ptr` instances are gone but `weak_ptr`s still exist, is the managed object destroyed? Is the control block destroyed? When is each freed?

**Expected depth:** Managed object is destroyed when strong count → 0. Control block is destroyed when *both* strong and weak counts → 0. If `make_shared` was used (single allocation), the memory for the object + control block cannot be returned to the allocator until the control block is also freed — even though the object's destructor ran. This can delay memory reclamation if many `weak_ptr`s are long-lived.

---

## Section 4: `make_unique` / `make_shared` vs `new` (Item 21)

### Q16. Exception safety in function arguments

```cpp
processWidget(std::shared_ptr<Widget>(new Widget), computePriority());
```

Explain how this line can leak memory. How does `std::make_shared` prevent the leak? Is this still a problem in C++17? Why or why not?

**Expected depth:** Pre-C++17, argument evaluation order is unspecified. Compiler may: (1) `new Widget`, (2) `computePriority()` (throws), (3) never construct `shared_ptr` → leak. `make_shared` is atomic — allocation and `shared_ptr` construction are a single step. In C++17, the evaluation of each function argument is *indeterminately sequenced* but each is a full-expression, so the leak can no longer happen. Still, `make_shared` is preferred for the single-allocation benefit.

---

### Q17. Single vs dual allocation

Describe the two memory allocation strategies for constructing a `shared_ptr` and their performance implications:

1. `std::shared_ptr<Widget>(new Widget());`
2. `auto sp = std::make_shared<Widget>();`

What effect does each have on cache performance? On memory fragmentation?

**Expected depth:** (1) Two allocations: Widget + control block (separate heap blocks). (2) One allocation: Widget and control block are colocated. Single allocation → better cache locality, less allocator overhead, reduced fragmentation. But: with (2), the entire block is kept alive until weak count also reaches 0 (relevant for large objects with long-lived `weak_ptr`s).

---

### Q18. When `make_shared` / `make_unique` cannot be used

List all limitations of the make functions. For each, provide the correct alternative.

**Expected depth:** (1) Custom deleters — use `shared_ptr<T>(new T, deleter)`. (2) Brace-initialised construction — make functions use `()` forwarding, not `{}`; workaround: create an `initializer_list` variable first. (3) Classes with custom `operator new`/`operator delete` — `make_shared` bypasses them. (4) Huge objects with long-lived `weak_ptr`s — prefer separate allocations so object memory is freed promptly.

---

### Q19. Implementing `make_unique` for C++11

`std::make_unique` was omitted from C++11. Write a conforming implementation. What C++14 features, if any, does your implementation rely on?

**Expected depth:**
```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```
Relies only on variadic templates and perfect forwarding (both C++11). Array form (`make_unique<T[]>(size)`) requires an additional overload. The C++14 standard version also includes a `= delete` overload for `make_unique<T[N]>`.

---

## Section 5: Pimpl Idiom with `unique_ptr` (Item 22)

### Q20. Why must the destructor be defined in the `.cpp` file?

```cpp
// widget.h
class Widget {
public:
    Widget();
    ~Widget() = default; // compiles? why or why not?
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
```

Explain the compilation error. What specific `static_assert` inside `unique_ptr`'s deleter triggers it? Why does the same code work fine with `shared_ptr`?

**Expected depth:** `unique_ptr`'s default deleter calls `delete ptr` with a `static_assert(sizeof(T) > 0)` to ensure the type is complete. The compiler generates the inline destructor in the header where `Impl` is only forward-declared (incomplete). `shared_ptr` type-erases the deleter into the control block at construction time (in the `.cpp` file where `Impl` *is* complete), so no completeness check is needed when the `shared_ptr` is destroyed.

---

### Q21. Complete special member function list for Pimpl

When using `unique_ptr` Pimpl, which special member functions must be *declared* in the header and *defined* in the `.cpp`? Why does forgetting the move constructor cause a problem even if you declared the destructor?

**Expected depth:** Must declare in header and define in `.cpp`: destructor, move constructor, move assignment operator. Optionally: copy constructor and copy assignment (with deep-copy implementation). Declaring a user-defined destructor suppresses compiler-generated move operations. If you never declare moves, the class becomes non-movable (or tries to copy, which fails because `unique_ptr` is non-copyable).

---

### Q22. Deep copy for Pimpl

Write a correct copy constructor and copy assignment operator for a Pimpl class. What happens if you write `= default` for the copy in the header?

**Expected depth:** `= default` for copy in the header attempts to copy the `unique_ptr` → compile error (copy is deleted). Correct implementation:
```cpp
Widget::Widget(const Widget& rhs)
    : pImpl_(std::make_unique<Impl>(*rhs.pImpl_)) {}
Widget& Widget::operator=(const Widget& rhs) {
    *pImpl_ = *rhs.pImpl_;
    return *this;
}
```
Both must be defined in the `.cpp` where `Impl` is complete.

---

### Q23. ABI stability and Pimpl

A shared library (.so / .dylib) exposes a Pimpl-based class in its public header. The library maintainer adds three new data members to `Impl` and ships a new version. Will client code compiled against the old header still work without recompilation?

**Expected depth:** Yes. `sizeof(Widget)` does not change (it just holds a pointer). The vtable (if any) doesn't change. The new `Impl` layout is entirely inside the `.so`. Client code only needs to relink — no recompilation required. This is the ABI-stability benefit.

---

### Q24. `unique_ptr` Pimpl vs `shared_ptr` Pimpl — trade-offs

A colleague suggests using `shared_ptr<Impl>` for Pimpl to avoid declaring the destructor and move operations in the header. Argue for or against this approach. What semantic issue does `shared_ptr` Pimpl introduce?

**Expected depth:** `shared_ptr` Pimpl eliminates the need for special member declarations (no incomplete-type problem). However, the default copy constructor now *shares* the `Impl` between two `Widget` instances — mutating one silently mutates the other. This is almost never correct. `unique_ptr` forces you to implement explicit deep copy, which is the correct semantic. The minor convenience is not worth the subtle shared-state bug.

---

## Section 6: Cross-Cutting / Scenario-Based

### Q25. Smart pointer choice in a real system

You are designing an event-driven system with:
- A central `EventBus`
- Multiple `Subscriber` objects that can be created and destroyed dynamically
- Events carry payloads that may be processed asynchronously after the publisher is gone

For each of the following, state which smart pointer you would use and why:
1. `EventBus` storing references to subscribers
2. An asynchronous event handler capturing the payload
3. A subscriber holding a back-reference to the `EventBus`
4. A factory function creating subscribers

**Expected depth:** (1) `weak_ptr<Subscriber>` — subscribers may die at any time; bus should not keep them alive. (2) `shared_ptr<Payload>` — async handler must keep the payload alive until processing completes. (3) Raw pointer or `weak_ptr<EventBus>` — avoid circular ownership; if bus lifetime is guaranteed, raw pointer suffices. (4) `unique_ptr<Subscriber>` — factory transfers exclusive ownership to caller.

---

### Q26. Performance-sensitive code path

In a trading system, a hot path processes 10 million messages per second. A colleague passes `shared_ptr<Message>` by value through six function calls in the pipeline. The profiler shows 40% time spent on atomic reference count operations. What is your recommendation?

**Expected depth:** Pass `const shared_ptr<Message>&` or, better, pass `const Message&` / `Message*` through the pipeline. The top-level frame holds the `shared_ptr` keeping the object alive; inner frames just need access. If the object must outlive the pipeline (e.g., async handoff), construct a `shared_ptr` copy only at the async boundary. Also consider whether `unique_ptr` is sufficient if messages have a single owner.

---

### Q27. Migrating legacy code to smart pointers

A 500 KLOC legacy codebase uses raw `new`/`delete` everywhere. Outline a migration strategy that minimises risk. What role does `unique_ptr` play as the "default" smart pointer? How do you handle cases where ownership is genuinely shared?

**Expected depth:** Phased approach: (1) Identify ownership semantics per subsystem. (2) Replace factory functions to return `unique_ptr` first — lowest risk, clearest ownership. (3) Convert clearly single-owner `new`/`delete` pairs to `unique_ptr`. (4) Introduce `shared_ptr` only where multiple owners are proven. (5) Audit for raw `delete` — should vanish. (6) Use static analysis (clang-tidy `modernize-use-unique-ptr`, `misc-use-after-move`) to catch stragglers. Key insight: `unique_ptr` is the default; `shared_ptr` is the exception.

---

### Q28. `enable_shared_from_this` pitfall

```cpp
class Session : public std::enable_shared_from_this<Session> {
public:
    void start() {
        auto self = shared_from_this();
        asyncRead([self](auto data) { self->process(data); });
    }
};

// In main():
Session s;
s.start(); // crashes
```

Why does this crash? What is the fundamental rule about `enable_shared_from_this` that was violated?

**Expected depth:** `shared_from_this()` requires that at least one `shared_ptr` to the object already exists. `Session s` is stack-allocated — no `shared_ptr` manages it. On C++17 this throws `std::bad_weak_ptr`; on C++11/14 it is undefined behaviour. Fix: always create `Session` through `std::make_shared<Session>()`.

---

### Q29. Memory pool with `unique_ptr` custom deleter

Design a memory pool that vends objects via `unique_ptr` with a custom deleter that returns the object to the pool instead of calling `delete`. What happens to the `unique_ptr`'s size? How does this affect storage in `std::vector`?

**Expected depth:** Custom deleter (e.g., lambda capturing pool pointer) makes `unique_ptr` larger than a raw pointer. If using a stateless functor with `operator()`, EBO applies and size remains unchanged. A lambda with captures increases size. For vector storage, the size increase means more memory per element and potentially worse cache performance. Alternative: use a function pointer deleter (one pointer overhead) or a stateless custom deleter class.

---

### Q30. Compile-time ownership documentation

How do smart pointer types serve as documentation of ownership intent in a codebase? For each function signature below, describe the ownership contract:

```cpp
void observe(const Widget& w);
void observe(Widget* w);
void sink(std::unique_ptr<Widget> w);
void share(std::shared_ptr<Widget> w);
void maybeShare(const std::shared_ptr<Widget>& w);
void reseat(std::shared_ptr<Widget>& w);
```

**Expected depth:**
- `const Widget&` — no ownership, just reading
- `Widget*` — no ownership, optional / nullable reference
- `unique_ptr<Widget>` by value — ownership *transfer* (caller gives up ownership)
- `shared_ptr<Widget>` by value — caller *shares* ownership (ref count incremented)
- `const shared_ptr<Widget>&` — function may extend lifetime by copying, but doesn't necessarily; typically used for efficiency to avoid ref-count bump
- `shared_ptr<Widget>&` — function may reseat the caller's `shared_ptr` to point to a different object

---

## Bonus: Quick-Fire Questions (60 seconds each)

**B1.** What is the size of `shared_ptr<T>` on a 64-bit platform?  
→ 16 bytes (two pointers: object + control block)

**B2.** Can you `std::move` a `shared_ptr` into a `unique_ptr`?  
→ No. `shared_ptr` → `unique_ptr` conversion is not allowed. Only `unique_ptr` → `shared_ptr` works.

**B3.** What does `shared_ptr::use_count()` return when called from a `weak_ptr`?  
→ The strong (owning) reference count. `weak_ptr` instances are not counted.

**B4.** Is `make_unique` exception-safe with respect to the allocation?  
→ Yes. If the constructor throws, the `new`-allocated memory is freed. But it does not help with the dual-allocation issue (only `make_shared` does).

**B5.** What happens when you assign `nullptr` to a `unique_ptr`?  
→ The currently owned object is destroyed (deleter is called), and the `unique_ptr` becomes empty.

**B6.** Can a `unique_ptr` be stored in a `std::set`?  
→ Yes, if move-only insertion is used (`emplace`, `insert(std::move(...))`). Comparison requires a custom comparator or using `owner_less`.

**B7.** What is the difference between `unique_ptr::reset()` and `unique_ptr::release()`?  
→ `reset()` destroys the managed object. `release()` gives up ownership without destroying — caller is responsible for deletion.

**B8.** Can `make_shared` be used with abstract classes?  
→ No. `make_shared<T>()` calls `new T(...)` internally, which requires `T` to be instantiable. Use `shared_ptr<Base>(new Derived())` instead.

**B9.** Why is `unique_ptr<T>` preferred over `auto_ptr<T>`?  
→ `auto_ptr` has copy semantics that actually move (confusing, unsafe). `auto_ptr` was deprecated in C++11 and removed in C++17. `unique_ptr` uses proper move semantics and is well-behaved in containers.

**B10.** In a Pimpl class, if `Impl` has a `std::vector<std::string>` member, do you need to define the copy constructor in the `.cpp` file?  
→ Yes, because the compiler-generated copy tries to copy the `unique_ptr` (deleted operation). You must write a manual deep-copy that creates a new `Impl` via `make_unique<Impl>(*rhs.pImpl_)`.
