# Cross-Cutting & Scenario-Based — Detailed Interview Answers (Q25–Q30)

**Target:** Senior C++ Developer (10+ years C++, 14+ years overall)  
**Source:** Effective Modern C++ — Items 18–22 (combined)

---

## Q25. Smart Pointer Choice in an Event-Driven System

> *You're designing an event-driven system with publishers and subscribers. A publisher holds a list of subscribers. Subscribers can be destroyed at any time. Which smart pointer(s) do you use for the subscriber list? What about the event payload? What about back-references from events to the publisher?*

### Answer

#### Architecture overview

```
┌──────────────┐    fires    ┌───────────────┐    delivered to    ┌──────────────┐
│  Publisher   │ ──────────▶ │    Event       │ ──────────────▶   │  Subscriber  │
│              │             │  (payload)     │                   │              │
│ subscribers_ │             │  source_       │                   │              │
│  [weak_ptr]  │             │  [weak/raw]    │                   │  [shared_ptr │
│              │             │                │                   │   owned by   │
└──────────────┘             └───────────────┘                   │  app code]   │
```

#### Subscriber list → `std::vector<std::weak_ptr<Subscriber>>`

**Why `weak_ptr`:** Subscribers can be destroyed at any time (the user closes a window, a module unloads, etc.). The publisher must **not** extend the lifetime of subscribers — otherwise, "destroyed" subscribers live on as zombies, consuming resources and potentially processing events they shouldn't.

`weak_ptr` provides exactly this:
- **Non-owning:** does not extend lifetime.
- **Dangling detection:** `lock()` returns `nullptr` if the subscriber was destroyed.
- **Thread-safe:** `lock()` is atomic.

**Implementation:**

```cpp
class Publisher {
    std::vector<std::weak_ptr<Subscriber>> subscribers_;

public:
    void subscribe(std::weak_ptr<Subscriber> sub) {
        subscribers_.push_back(std::move(sub));
    }

    void publish(const Event& event) {
        // Use erase-remove idiom to clean up expired subscribers
        subscribers_.erase(
            std::remove_if(subscribers_.begin(), subscribers_.end(),
                [](const std::weak_ptr<Subscriber>& wp) { return wp.expired(); }),
            subscribers_.end()
        );

        for (auto& wp : subscribers_) {
            if (auto sp = wp.lock()) {  // atomically check + acquire
                sp->onEvent(event);     // subscriber is alive for the duration of this call
            }
        }
    }
};
```

**Why not alternatives:**

| Alternative | Problem |
|---|---|
| `shared_ptr<Subscriber>` | Publisher keeps subscribers alive forever → memory leak, zombie processing |
| `raw pointer` | Dangling pointer if subscriber is destroyed → UB, crash |
| `std::function` callback | Cannot detect if the owner is dead → may invoke callback on destroyed object |

#### Event payload → `std::shared_ptr<const Payload>`

**Why `shared_ptr`:** An event may be delivered to multiple subscribers, potentially asynchronously (queued, across threads). The payload must outlive the publisher and remain alive until the last subscriber has processed it.

**Why `const`:** Payloads should be immutable — multiple subscribers must not race on mutation. `shared_ptr<const Payload>` enforces this at compile time.

```cpp
struct Event {
    std::shared_ptr<const Payload> payload;  // shared, immutable
    EventType type;
    Timestamp timestamp;
};

// Creation — in the publisher:
auto payload = std::make_shared<const Payload>(/* ... */);
Event event{payload, EventType::DataUpdate, Clock::now()};
publish(event);
```

**Alternative for simple payloads:** If the payload is small and trivially copyable (e.g., an enum + int), simply copy it by value — no smart pointer needed.

#### Back-references (event → publisher) → `weak_ptr<Publisher>` or raw pointer

**Option 1: `weak_ptr<Publisher>`** — Use when the publisher is managed by `shared_ptr` and subscribers might outlive the publisher (asynchronous event processing):

```cpp
struct Event {
    std::weak_ptr<Publisher> source;   // non-owning back-reference
    std::shared_ptr<const Payload> payload;
};

// Subscriber processing:
void Subscriber::onEvent(const Event& event) {
    if (auto pub = event.source.lock()) {
        pub->acknowledge(/* ... */);   // publisher still alive
    }
    // else: publisher was destroyed — skip acknowledgment
}
```

**Option 2: Raw pointer / reference** — Use when the publisher's lifetime is guaranteed to exceed all event processing (synchronous delivery, publisher is a long-lived singleton):

```cpp
struct Event {
    Publisher* source;   // raw — lifetime guaranteed by design
    // ...
};
```

Raw is appropriate here because:
- The publisher is typically a long-lived object (application lifetime).
- Synchronous delivery means events are processed while the publisher is on the call stack.
- No overhead of `weak_ptr` (no control block, no atomic operations).

#### Factory-created subscribers → `unique_ptr` from factory, then move into `shared_ptr`

If the publisher owns a factory that creates subscribers:

```cpp
std::unique_ptr<Subscriber> sub = factory.create();    // factory returns unique_ptr
std::shared_ptr<Subscriber> shared = std::move(sub);   // convert to shared for multi-reference
publisher.subscribe(shared);                            // publisher holds weak_ptr
appState.addWidget(std::move(shared));                  // app code holds shared_ptr
```

This follows Item 18's principle: **factory functions return `unique_ptr`** (most restrictive ownership), and callers convert to `shared_ptr` when needed.

---

## Q26. Performance on Hot Paths

> *In a performance-sensitive inner loop, you call a function that takes a `shared_ptr<Config>`. The config never changes during the loop. How should you pass it? What if it's an async boundary?*

### Answer

#### Rule: Pass raw reference in hot paths, copy `shared_ptr` only at async boundaries

#### Case 1: Synchronous hot path — pass by raw reference

```cpp
// Config is stable for the lifetime of the loop
void processFrame(const Config& config) {   // raw reference — zero overhead
    for (int i = 0; i < 1'000'000; ++i) {
        applyFilter(config);   // hot inner loop
    }
}

// Caller holds the shared_ptr
std::shared_ptr<Config> configPtr = loadConfig();
processFrame(*configPtr);   // dereference once — pass raw reference
```

**Why not `shared_ptr<Config>`?** Every copy of a `shared_ptr` performs an **atomic increment** (on construction) and **atomic decrement** (on destruction). In a tight loop called millions of times:

```cpp
// BAD — atomic inc/dec on every call
void applyFilter(std::shared_ptr<Config> config) { /* ... */ }

// Atomic operations per call: 2 (increment on copy-in + decrement on destruction)
// In 1M iterations: 2M atomic operations → significant cache-line bouncing
```

Atomic operations on x86-64 are ~10-40ns each. On ARM, even more expensive (memory barriers). In a loop of 1M iterations:
- Reference passing: ~0 overhead for ownership
- `shared_ptr` by value: ~20-80ms of pure atomic overhead

**Pass by `const shared_ptr<Config>&`?** Better — no atomic operations. But semantically confusing: `const shared_ptr<Config>&` signals "I might share ownership" but never does. A raw `const Config&` is clearer and equally efficient.

#### Decision tree

```
Is the function synchronous and the caller guaranteed to outlive the call?
├── YES → pass raw pointer (T*) or reference (const T&)
│         Zero overhead. Caller's shared_ptr keeps object alive.
│
└── NO → Does the function need to extend the object's lifetime?
    ├── YES → pass shared_ptr<T> by value (copy = atomic inc)
    │         This happens at async boundaries: thread dispatch, callback storage
    │
    └── NO → pass const shared_ptr<T>& (no atomic ops, no ownership transfer)
             Rare — usually raw ref is better
```

#### Case 2: Async boundary — copy the `shared_ptr`

When you dispatch work to another thread or store a callback, the config must outlive the current scope. Here, copying the `shared_ptr` is necessary and correct:

```cpp
void scheduleAsyncWork(std::shared_ptr<Config> config) {
    // Capture config BY VALUE in the lambda
    threadPool.enqueue([config = std::move(config)]() {
        // config's shared_ptr keeps Config alive even if the original is gone
        processFrame(*config);
    });
}
```

Note: `std::move(config)` into the lambda avoids one extra atomic increment (moved-from `shared_ptr` doesn't need decrement).

#### Summary of guidelines from Herb Sutter (GotW #91)

| Parameter type | When to use | Overhead |
|---|---|---|
| `const T&` or `T*` | Function uses object, doesn't affect lifetime | Zero |
| `shared_ptr<T>` by value | Function stores/shares ownership (async, caching) | 1 atomic inc + 1 atomic dec |
| `const shared_ptr<T>&` | Function **might** share ownership (conditional) | Zero (but semantically unclear) |
| `shared_ptr<T>&&` | Function unconditionally takes ownership | 1 move (no atomics) |

**The rule is simple:** Only pay for `shared_ptr` overhead at the points where ownership actually changes hands.

---

## Q27. Legacy Migration Strategy

> *You inherit a 500KLOC C++ codebase that uses raw `new`/`delete` everywhere. Outline a phased migration strategy to smart pointers. Which smart pointer should be your default choice and why?*

### Answer

#### Default choice: `std::unique_ptr`

**Why `unique_ptr` first:**
1. **1:1 mapping** — Every raw `new` that has a corresponding `delete` in the same scope/class is a direct candidate for `unique_ptr`. No architectural changes needed.
2. **Zero overhead** — Same performance as raw pointers. No risk of regressions in performance-critical paths.
3. **Strictest ownership** — Forces single-owner semantics. If code fails to compile because it tries to copy a `unique_ptr`, that's a signal of unclear ownership — which was a latent bug with raw pointers.
4. **Easy upgrade path** — `unique_ptr` converts to `shared_ptr` if shared ownership is later needed.

#### Phased migration strategy

##### Phase 0: Preparation (no code changes)

1. **Enable AddressSanitizer (ASan)** and **LeakSanitizer (LSan)** in debug builds:
   ```
   -fsanitize=address,leak -fno-omit-frame-pointer
   ```
2. **Run existing tests under sanitizers** to establish a baseline of known leaks and use-after-free bugs.
3. **Add `-Wall -Wextra -Werror`** to catch all warnings.
4. **Identify ownership patterns** — Use static analysis tools (clang-tidy's `modernize-use-unique-ptr`, `cppcheck`) to flag `new`/`delete` pairs.
5. **Categorize ownership:**
   - **Scope-local:** `new` in function, `delete` in same function → `unique_ptr` or stack allocation.
   - **Member-owned:** `new` in constructor, `delete` in destructor → `unique_ptr` member.
   - **Transferred:** `new` in one place, `delete` in another (factory pattern) → `unique_ptr` return value.
   - **Shared:** multiple owners, unclear who deletes → `shared_ptr` (rare — most "shared" ownership is actually unclear ownership).

##### Phase 1: Low-hanging fruit (lowest risk)

**Target:** Scope-local allocations.

```cpp
// Before:
void process() {
    Config* cfg = new Config(loadFile());
    doWork(cfg);
    delete cfg;        // easy to miss if doWork throws
}

// After:
void process() {
    auto cfg = std::make_unique<Config>(loadFile());
    doWork(cfg.get());   // pass raw pointer to non-owning functions
}   // automatic cleanup, exception-safe
```

**Mechanically safe** — the ownership boundary doesn't cross function boundaries. Low risk of regression.

##### Phase 2: Member ownership

**Target:** Classes that `new` in constructor and `delete` in destructor.

```cpp
// Before:
class Engine {
    Renderer* renderer_;
public:
    Engine() : renderer_(new Renderer()) {}
    ~Engine() { delete renderer_; }
    // Bug: no copy/move operations defined → double-delete on copy
};

// After:
class Engine {
    std::unique_ptr<Renderer> renderer_;
public:
    Engine() : renderer_(std::make_unique<Renderer>()) {}
    // Destructor, move ops: automatically correct
    // Copy: automatically deleted (safe — was buggy before)
};
```

This phase often **reveals existing bugs** — classes that were missing copy constructors (Rule of Three violations) become non-copyable, which is usually the correct behavior.

##### Phase 3: Factory functions and ownership transfer

**Target:** Functions that return `new`-ed pointers.

```cpp
// Before:
Widget* createWidget(Config* cfg) {
    return new Widget(cfg);   // caller must remember to delete
}

// After:
std::unique_ptr<Widget> createWidget(const Config& cfg) {
    return std::make_unique<Widget>(cfg);   // ownership is explicit
}
```

This is the biggest win for API clarity — the return type **documents** ownership transfer.

##### Phase 4: Shared ownership (where genuinely needed)

**Target:** Objects with multiple owners (caches, shared resources, observer patterns).

After Phases 1-3, the remaining raw `new`/`delete` patterns are likely the genuinely shared-ownership cases. These require `shared_ptr`:

```cpp
// Cache that shares objects with callers:
class TextureCache {
    std::unordered_map<std::string, std::shared_ptr<Texture>> cache_;
public:
    std::shared_ptr<Texture> get(const std::string& name) {
        auto it = cache_.find(name);
        if (it != cache_.end()) return it->second;
        auto tex = std::make_shared<Texture>(loadFromDisk(name));
        cache_[name] = tex;
        return tex;
    }
};
```

#### Phase 5: Ongoing enforcement

1. **clang-tidy rules** — Enable `cppcoreguidelines-owning-memory`, `modernize-make-unique`, `modernize-make-shared`.
2. **Code review policy** — No raw `new`/`delete` in new code without a justification comment.
3. **CI checks** — Run sanitizers on every PR.
4. **Gradual** — Do not attempt to migrate 500KLOC at once. Migrate module by module, with tests passing at each step.

#### Common pitfalls during migration

| Pitfall | Description | Mitigation |
|---|---|---|
| Double-free | Wrapping a raw pointer in `unique_ptr` while old code still calls `delete` on it | Search for ALL `delete` sites of each migrated pointer |
| Aliased raw pointers | Multiple raw pointers to the same object — wrapping one in `unique_ptr` while others remain raw → dangling | Map all aliases before migrating |
| Custom allocators | `new`/`delete` mismatch with custom allocators (e.g., `malloc`/`free`, pool allocators) | Use custom deleters: `unique_ptr<T, decltype(&free)>` |
| Non-owning parameters | Converting function parameters from `T*` to `unique_ptr<T>` when the function is non-owning | Non-owning parameters should remain raw `T*` or `T&` |

---

## Q28. `enable_shared_from_this` Pitfall

> ```cpp
> class Session : public std::enable_shared_from_this<Session> {
> public:
>     std::shared_ptr<Session> getPtr() {
>         return shared_from_this();
>     }
> };
>
> int main() {
>     Session s;                        // stack-allocated
>     auto ptr = s.getPtr();            // what happens?
> }
> ```
>
> *Explain the probable result. What is the precondition for `shared_from_this()`?*

### Answer

#### What happens: `std::bad_weak_ptr` exception (or undefined behaviour)

```
terminate called after throwing an instance of 'std::bad_weak_ptr'
  what(): bad_weak_ptr
```

On some implementations (pre-C++17), calling `shared_from_this()` on an object not managed by a `shared_ptr` is **undefined behaviour**. C++17 mandates that it throws `std::bad_weak_ptr`.

#### Why it fails — the mechanism inside `enable_shared_from_this`

`enable_shared_from_this<T>` works by storing a **hidden `weak_ptr<T>`** inside the base class:

```cpp
// Simplified implementation:
template<typename T>
class enable_shared_from_this {
    mutable weak_ptr<T> weak_this_;   // the hidden member
protected:
    enable_shared_from_this() noexcept = default;
public:
    shared_ptr<T> shared_from_this() {
        return shared_ptr<T>(weak_this_);   // constructs from weak_ptr
        // If weak_this_ is empty → throws bad_weak_ptr
    }
};
```

The `weak_this_` is **initialised by `shared_ptr`'s constructor**. When you create a `shared_ptr<Session>`, the `shared_ptr` constructor detects that `Session` inherits from `enable_shared_from_this<Session>` and initialises `weak_this_`:

```cpp
// Inside shared_ptr<T> constructor (simplified):
template<typename T>
shared_ptr<T>::shared_ptr(T* ptr) {
    // ... create control block ...
    if constexpr (has_enable_shared_from_this<T>) {
        ptr->weak_this_ = *this;   // ← initializes the hidden weak_ptr
    }
}
```

For a **stack-allocated** `Session s`:
- No `shared_ptr` was ever created.
- `weak_this_` was never initialised — it's still an empty `weak_ptr`.
- `shared_from_this()` tries to construct a `shared_ptr` from an empty `weak_ptr` → **throws `bad_weak_ptr`**.

#### The precondition

> **`shared_from_this()` may only be called on an object that is currently managed by at least one `shared_ptr`.**

The object must have been created via:
```cpp
auto session = std::make_shared<Session>();          // ✅ OK
auto session = std::shared_ptr<Session>(new Session()); // ✅ OK
```

It must **not** be:
```cpp
Session s;                                           // ❌ Stack-allocated
Session* s = new Session();                          // ❌ Raw pointer, no shared_ptr yet
auto s = std::make_unique<Session>();                // ❌ unique_ptr, not shared_ptr
```

#### Why this design exists — the problem it solves

Without `enable_shared_from_this`, creating a `shared_ptr` from `this` is dangerous:

```cpp
class Session {
public:
    std::shared_ptr<Session> getPtr() {
        return std::shared_ptr<Session>(this);   // ❌ DOUBLE-DELETE!
    }
};

auto sp1 = std::make_shared<Session>();
auto sp2 = sp1->getPtr();   // creates a SECOND control block for the same object!
// sp1 and sp2 have independent ref counts
// When both go out of scope → double delete
```

`enable_shared_from_this` solves this by reusing the **existing control block** — `shared_from_this()` returns a `shared_ptr` that shares ownership with the original.

#### Correct usage pattern

```cpp
class Session : public std::enable_shared_from_this<Session> {
    // Private constructor to prevent stack/raw allocation
    Session() = default;
public:
    // Factory function ensures shared_ptr creation
    static std::shared_ptr<Session> create() {
        return std::shared_ptr<Session>(new Session());
        // Note: can't use make_shared with private constructor
        // unless you use the passkey idiom
    }

    void asyncOperation() {
        // Capture shared_from_this() to extend lifetime through async work
        auto self = shared_from_this();
        threadPool.enqueue([self]() {
            self->processResult();   // 'self' keeps Session alive
        });
    }
};
```

#### Make-shared with private constructor (passkey idiom)

```cpp
class Session : public std::enable_shared_from_this<Session> {
    struct PrivateKey {};   // passkey — only Session can create it
public:
    explicit Session(PrivateKey) {}   // "public" but effectively private

    static std::shared_ptr<Session> create() {
        return std::make_shared<Session>(PrivateKey{});  // ✅ make_shared works
    }
};
```

---

## Q29. Memory Pool with `unique_ptr` Custom Deleter

> *Design a `unique_ptr`-based allocation scheme where objects come from a fixed-size memory pool. Show the custom deleter. Discuss whether you use a functor or a lambda for the deleter and why. What is the size impact?*

### Answer

#### Memory pool design

A fixed-size pool pre-allocates N objects and hands them out / reclaims them without `malloc`/`free`:

```cpp
#include <memory>
#include <array>
#include <stack>
#include <cassert>
#include <new>

template<typename T, std::size_t PoolSize = 256>
class ObjectPool {
    // Aligned raw storage for T objects
    alignas(T) std::array<std::byte, sizeof(T) * PoolSize> storage_;
    std::stack<T*> freeList_;

public:
    ObjectPool() {
        // Pre-populate free list with addresses of all slots
        auto* base = reinterpret_cast<T*>(storage_.data());
        for (std::size_t i = 0; i < PoolSize; ++i) {
            freeList_.push(base + i);
        }
    }

    ~ObjectPool() {
        // All objects must be returned before pool destruction
        assert(freeList_.size() == PoolSize && "Leaked pool objects!");
    }

    // Non-copyable, non-movable (addresses of storage must be stable)
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // The custom deleter — a stateful functor that holds a pointer to the pool
    struct Deleter {
        ObjectPool* pool;

        void operator()(T* ptr) const noexcept {
            ptr->~T();               // call destructor
            pool->freeList_.push(ptr); // return slot to pool
        }
    };

    using Ptr = std::unique_ptr<T, Deleter>;

    template<typename... Args>
    Ptr acquire(Args&&... args) {
        if (freeList_.empty()) {
            throw std::bad_alloc();
        }
        T* slot = freeList_.top();
        freeList_.pop();
        ::new (slot) T(std::forward<Args>(args)...);  // placement new
        return Ptr(slot, Deleter{this});
    }
};
```

#### Usage

```cpp
ObjectPool<Particle, 1024> pool;

auto p1 = pool.acquire(1.0f, 2.0f, 3.0f);   // construct in pool
auto p2 = pool.acquire(4.0f, 5.0f, 6.0f);
p1->update(dt);
// p1 goes out of scope → destructor called, slot returned to pool
// No malloc/free — just placement new + destructor
```

#### Functor vs Lambda vs Function pointer for the deleter

| Deleter type | `sizeof(unique_ptr<T, D>)` | Reason | Recommended? |
|---|---|---|---|
| **Stateless functor** (empty struct) | `sizeof(T*)` = 8 bytes | Empty Base Optimization (EBO) — empty deleter stored in zero bytes | ✅ (when no state needed) |
| **Stateful functor** (`ObjectPool* pool`) | `sizeof(T*) + sizeof(ObjectPool*)` = 16 bytes | Functor stores a pointer to the pool — one extra pointer | ✅ (our case — needs pool reference) |
| **Lambda (stateless)** | `sizeof(T*)` = 8 bytes | Stateless lambda = empty type → EBO applies | ✅ (when no captures) |
| **Lambda (capturing `pool`)** | `sizeof(T*) + sizeof(ObjectPool*)` = 16 bytes | Capturing lambda has size of its captures | ✅ (equivalent to stateful functor) |
| **Function pointer** (`void(*)(T*)`) | `sizeof(T*) + sizeof(void(*)(T*))` = 16 bytes | Function pointer is always stored — no EBO | ❌ (can't capture pool without global state) |

#### Why functor over lambda in this case

For the pool deleter, both a stateful functor and a capturing lambda produce the same `sizeof` (16 bytes). However:

**Functor advantages:**
1. **Named type** — `ObjectPool::Deleter` can be used in type aliases: `using Ptr = unique_ptr<T, Deleter>;`. Lambda types are unnameable.
2. **Reusable** — Can be used in multiple `unique_ptr`s without repeating the lambda definition.
3. **Debuggable** — The type name appears in debugger, compiler errors, and stack traces.

**Lambda advantages:**
1. **Inline definition** — Good for one-off deleters defined close to usage.
2. **Terse** — Less boilerplate.

**In practice for a pool:** Use a **named functor** (as shown above). The deleter is part of the pool's API and appears in the return type — it should have a meaningful name.

#### Size impact analysis

```
unique_ptr<T> with default_delete<T>:
  ┌────────────┐
  │ T* ptr (8B)│    sizeof = 8 bytes (EBO: default_delete is empty)
  └────────────┘

unique_ptr<T, PoolDeleter>:
  ┌────────────┬──────────────────┐
  │ T* ptr (8B)│ ObjectPool* (8B) │    sizeof = 16 bytes
  └────────────┴──────────────────┘
```

For an array of 1024 pool-allocated `unique_ptr`s:
- Default deleter: 8 KB
- Pool deleter: 16 KB
- Extra cost: 8 KB (one pointer per object)

This is usually acceptable. If the extra 8 bytes per pointer matters, you can make the pool a **global/singleton** and use a stateless functor that finds the pool by other means (template parameter, thread-local, etc.) — then EBO gives you 8 bytes per `unique_ptr`:

```cpp
template<typename T>
struct GlobalPoolDeleter {
    void operator()(T* ptr) const noexcept {
        ptr->~T();
        GlobalPool<T>::instance().reclaim(ptr);
    }
};
// sizeof(unique_ptr<T, GlobalPoolDeleter<T>>) == sizeof(T*) == 8 bytes
```

But this trades design flexibility (global state) for space. Prefer the stateful functor unless profiling shows the extra 8 bytes is a bottleneck.

---

## Q30. Compile-Time Ownership Documentation via Function Signatures

> *Show how function signatures alone (parameter and return types using smart pointers) document six different ownership contracts. List each signature and explain what it communicates to callers.*

### Answer

#### The six ownership contracts

Function signatures with smart pointers form a **vocabulary of ownership** — each signature communicates a different contract without any comments or documentation needed.

---

### Contract 1: "I observe, I don't own" — raw pointer / reference

```cpp
void observe(const Widget& w);   // preferred for non-null
void observe(const Widget* w);   // allows null
```

**Contract:** "I will read/use the object during this call. I do NOT affect its lifetime. The caller is responsible for ensuring the object outlives the call."

**When to use:** The vast majority of function parameters (~90%). Any synchronous function that uses an object without storing it.

**What it tells the caller:** "I won't store this. I won't delete this. Just keep it alive until I return."

---

### Contract 2: "Give me sole ownership" — `unique_ptr` by value (sink)

```cpp
void consume(std::unique_ptr<Widget> w);
```

**Contract:** "Transfer exclusive ownership to me. I will manage the object's lifetime from now on. The caller gives up all access."

**When to use:** Functions that store, transform, or destroy the object:
- Container insertion: `void addWidget(unique_ptr<Widget> w);`
- Ownership transfer: `void setStrategy(unique_ptr<Strategy> s);`

**What it tells the caller:** "You must `std::move()` into me. After the call, your `unique_ptr` is null. You no longer own this object."

```cpp
auto w = std::make_unique<Widget>();
consume(std::move(w));   // explicit transfer
// w is now nullptr — ownership transferred
```

---

### Contract 3: "I'm a factory — here's a new object you now own" — `unique_ptr` return

```cpp
std::unique_ptr<Widget> create();
std::unique_ptr<Widget> clone(const Widget& original);
```

**Contract:** "I create (or transform) an object and give you exclusive ownership. It's your responsibility to manage its lifetime."

**When to use:** Factory functions, clone/copy functions, builder patterns.

**What it tells the caller:** "You get a unique owner. You decide when to destroy it, or you can convert to `shared_ptr` if you need shared ownership."

```cpp
auto w = create();           // caller owns it
auto sp = std::shared_ptr<Widget>(create());  // convert to shared if needed
```

---

### Contract 4: "I share ownership with you" — `shared_ptr` by value

```cpp
void share(std::shared_ptr<Widget> w);
```

**Contract:** "I will store a copy of this `shared_ptr`, extending the object's lifetime. The object lives until all `shared_ptr`s (including mine) are destroyed."

**When to use:** Functions that **store** the pointer for later use:
- Caches: `void cache(shared_ptr<Texture> tex);`
- Async dispatch: `void enqueue(shared_ptr<Task> task);`
- Observer registration: `void registerHandler(shared_ptr<Handler> h);`

**Cost:** Atomic reference count increment on copy-in, decrement on destruction.

**What it tells the caller:** "I'm taking shared ownership. The object won't die while I hold it. But I pay for atomic ref counting."

---

### Contract 5: "I might share ownership" — `const shared_ptr<Widget>&`

```cpp
void maybeShare(const std::shared_ptr<Widget>& w);
```

**Contract:** "I may or may not copy the `shared_ptr`. If I do, I extend the lifetime. If I don't, I'm just observing."

**When to use:** Functions that conditionally store the pointer (e.g., add to cache only if not already cached).

**What it tells the caller:** "No ref-count cost if I don't store it. But I reserve the right to copy it."

**In practice:** This is a niche signature. Prefer Contract 1 (raw reference) if you never store, or Contract 4 (`shared_ptr` by value) if you always store.

---

### Contract 6: "I need to detect lifetime, but I don't own" — `weak_ptr`

```cpp
void watch(std::weak_ptr<Widget> w);
```

**Contract:** "I will store a `weak_ptr` to observe the object. I can check if it's still alive later with `lock()`. I do NOT extend its lifetime."

**When to use:** Observer/subscriber patterns, caches that should not prevent destruction, back-references that must avoid circular ownership.

**What it tells the caller:** "The object can still die on its own terms. I'll check before using it."

```cpp
auto sp = std::make_shared<Widget>();
watcher.watch(sp);   // watcher stores weak_ptr

sp.reset();          // last shared_ptr gone — Widget destroyed
// watcher.lock() will return nullptr — Widget is gone
```

---

### Summary table

| Signature | Ownership transfer | Lifetime effect | Cost |
|---|---|---|---|
| `f(const T&)` / `f(T*)` | None | None | Zero |
| `f(unique_ptr<T>)` | Caller → callee | Callee decides | Zero (move) |
| `unique_ptr<T> f()` | Callee → caller | Caller decides | Zero (move + RVO) |
| `f(shared_ptr<T>)` | Shared | Extended | Atomic inc/dec |
| `f(const shared_ptr<T>&)` | Maybe shared | Maybe extended | Zero or atomic |
| `f(weak_ptr<T>)` | None | None (observing) | Atomic (weak count) |

#### The guiding principle (Herb Sutter, CppCoreGuidelines F.7):

> **Don't pass a smart pointer as a function parameter unless you need to use or manipulate the smart pointer itself (its ownership semantics).** If the function just uses the underlying object, pass a raw `T*` or `T&`.

Smart pointers in function signatures are **ownership vocabulary** — they tell the story of who owns what, when, and for how long. Using raw pointers/references for non-owning access keeps the vocabulary precise: if you see a `shared_ptr` parameter, you **know** the function is participating in ownership — it's not just reading the object.
