# Item 20 — `std::weak_ptr` — Detailed Interview Answers

**Target:** Senior C++ Developer (10+ years C++, 14+ years overall)  
**Source:** Effective Modern C++ — Item 20

---

## Q12. Dangling Detection and the `lock()` Pattern

> *Why is the following pattern unsafe in a multithreaded environment?*
>
> ```cpp
> if (!wp.expired()) {
>     auto sp = wp.lock();
>     sp->doSomething();
> }
> ```
>
> *Rewrite it correctly and explain the atomicity guarantee of `lock()`.*

### Answer

#### Why the pattern is unsafe — the TOCTOU race

The code above is a textbook **Time-Of-Check to Time-Of-Use (TOCTOU)** bug. Two separate operations are performed in sequence, with no guarantee that the world hasn't changed between them:

```
Thread A (this code)                Thread B
────────────────────               ──────────────────
1. wp.expired() → false            
   (object is alive)               
                                   2. last_sp.reset()
                                      → strong count drops to 0
                                      → object DESTROYED
                                      → memory FREED
3. wp.lock()
   → strong count is 0
   → returns EMPTY shared_ptr
4. sp->doSomething()
   → sp is nullptr
   → UNDEFINED BEHAVIOUR (null dereference)
```

Between step 1 (`expired()` returns `false`) and step 3 (`lock()`), Thread B destroys the last `shared_ptr`. By the time `lock()` runs, the object is gone. `lock()` returns an empty `shared_ptr`, and the subsequent dereference is a null-pointer access — undefined behaviour.

Even without a null deref, consider a subtler variant:

```cpp
if (!wp.expired()) {
    auto sp = wp.lock();
    if (sp) {                  // programmer adds a null check
        sp->doSomething();     // safe? almost...
    }
}
```

This is *functionally correct* but the `expired()` check is **redundant and misleading** — it gives a false sense of safety while `lock()` alone is sufficient. Code reviewers may rely on `expired()` as the "real" check and overlook the `lock()` result, leading to maintenance bugs.

#### The correct pattern — single atomic check

```cpp
if (auto sp = wp.lock()) {     // atomic: check + increment strong count
    sp->doSomething();          // safe — sp keeps object alive for this scope
}
```

`lock()` performs the following as a **single atomic operation**:

1. Read the strong reference count.
2. If `strong_count > 0`, atomically increment it and return a valid `shared_ptr`.
3. If `strong_count == 0`, return an empty `shared_ptr` — no increment, no object access.

There is **no gap** between the liveness check and the reference-count increment. This is implemented with a compare-and-swap (CAS) loop internally:

```cpp
// Simplified pseudocode of what lock() does internally:
shared_ptr<T> weak_ptr<T>::lock() const noexcept {
    // Atomically try to increment the strong count
    auto* ctrl = control_block_;
    if (!ctrl) return shared_ptr<T>();

    long count = ctrl->strong_count.load(std::memory_order_relaxed);
    while (count > 0) {
        if (ctrl->strong_count.compare_exchange_weak(
                count, count + 1,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {
            // Success — strong count incremented atomically
            return shared_ptr<T>(/* ptr, ctrl */);
        }
        // CAS failed (another thread changed count), retry with updated value
    }
    // strong_count was 0 — object is gone
    return shared_ptr<T>();
}
```

The CAS loop ensures that the check (`count > 0`) and the action (`count + 1`) happen atomically with respect to all other threads.

#### Alternative: throwing variant

If you expect the object to be alive and consider it an error if it's not, use the `shared_ptr(weak_ptr)` constructor which **throws** `std::bad_weak_ptr`:

```cpp
try {
    std::shared_ptr<Widget> sp(wp);   // throws if expired
    sp->doSomething();
} catch (const std::bad_weak_ptr& e) {
    // Handle the unexpected case
    std::cerr << "Object was unexpectedly destroyed\n";
}
```

This is useful in scenarios where the object *should* be alive (e.g., a parent holding a `weak_ptr` to a child that must outlive the current operation).

#### Summary

| Pattern | Thread-safe? | Correct? |
|---|---|---|
| `if (!wp.expired()) { auto sp = wp.lock(); sp->use(); }` | ❌ TOCTOU race | ❌ `sp` may be null |
| `if (auto sp = wp.lock()) { sp->use(); }` | ✅ Atomic | ✅ Safe |
| `shared_ptr<T> sp(wp);  // throws if expired` | ✅ Atomic | ✅ Safe (exception on failure) |

---

## Q13. Circular Reference Detection

> *A doubly-linked list uses `shared_ptr` for both `next` and `prev`. Nobody can see the leak in testing — all unit tests pass, Valgrind shows no **invalid reads** — yet memory usage grows over time. Explain what is happening and how to fix it.*

### Answer

#### What is happening — the invisible leak

```cpp
struct Node {
    int value;
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;   // ← the problem
    ~Node() { std::cout << "Node " << value << " destroyed\n"; }
};

auto a = std::make_shared<Node>(Node{1, nullptr, nullptr});
auto b = std::make_shared<Node>(Node{2, nullptr, nullptr});

a->next = b;      // a's strong count on b: b has 2 owners (b, a->next)
b->prev = a;      // b's strong count on a: a has 2 owners (a, b->prev)
```

After the local variables `a` and `b` go out of scope:

```
Before scope exit:
  a (local)  →  Node(1) [strong=2]  ──next──▶  Node(2) [strong=2]
                    ▲                                │
                    └──────────prev──────────────────┘

After scope exit (locals destroyed):
  Node(1) [strong=1]  ──next──▶  Node(2) [strong=1]
       ▲                              │
       └──────────prev────────────────┘
```

Both nodes still have a strong count of **1** (held by the other node's `next`/`prev`). Neither count can ever reach 0. The destructors **never run**. The memory is **never freed**.

#### Why Valgrind and unit tests miss it

**Valgrind's Memcheck** detects:
- ✅ Invalid reads/writes (use-after-free)
- ✅ Leaked memory that is **unreachable** (no pointer to it)
- ❌ Leaked memory that is **still reachable** (some pointer chain leads to it)

In the circular reference case, each node has a `shared_ptr` pointing to it from the other node. Valgrind sees the memory as "still reachable" — it's pointed to by valid, live objects. It reports it as:

```
LEAK SUMMARY:
   definitely lost: 0 bytes          ← nothing "definitely" lost
   indirectly lost: 0 bytes
   possibly lost: 0 bytes
   still reachable: 192 bytes        ← here it is, but often ignored
```

Most teams configure Valgrind to suppress "still reachable" warnings (they're usually harmless global singletons). The circular leak hides in this bucket.

**Unit tests** pass because:
- The logic is correct — nodes are linked properly, traversal works.
- No crash — there's no dangling pointer, just leaked memory.
- Tests don't check for destructor invocation or memory count.
- Short-lived tests exit before memory growth becomes visible.

**How to actually catch it:**

| Tool | Detects circular leak? | How |
|---|---|---|
| Valgrind `--leak-check=full --show-reachable=yes` | ⚠️ Shows as "still reachable" | Need to inspect and not ignore |
| ASan (LeakSanitizer) | ✅ | Reports all leaked blocks at exit |
| Heap profiler (tcmalloc, Heaptrack) | ✅ | Shows monotonically growing memory |
| `shared_ptr::use_count()` assertions in tests | ✅ | Assert expected ref count after operations |
| Custom destructor logging | ✅ | If destructor message never appears, you know |

#### The fix — `weak_ptr` for back-links

The principle: in any ownership cycle, **at least one link must be `weak_ptr`** to break the cycle. In a doubly-linked list, the ownership direction is forward (`next`), so the back-link (`prev`) becomes `weak_ptr`:

```cpp
struct Node {
    int value;
    std::shared_ptr<Node> next;    // owning: keeps next node alive
    std::weak_ptr<Node>   prev;    // non-owning: observes previous node
    ~Node() { std::cout << "Node " << value << " destroyed\n"; }
};
```

Now the reference counts after local variables go out of scope:

```
After scope exit:
  Node(1) [strong=0] ← destroyed!   Node(2) [strong=0] ← destroyed!
                                     (prev was weak — didn't contribute to count)
```

Destruction sequence:
1. Local `b` is destroyed → `Node(2)` strong count drops from 2 to 1 (still held by `a->next`).
2. Local `a` is destroyed → `Node(1)` strong count drops from 1 to 0 → **`Node(1)` is destroyed**.
3. `Node(1)`'s destructor destroys `a->next` → `Node(2)` strong count drops from 1 to 0 → **`Node(2)` is destroyed**.

Both nodes are properly cleaned up.

#### Accessing the `prev` node safely

Since `prev` is now a `weak_ptr`, you must lock it before use:

```cpp
void printBackward(const std::shared_ptr<Node>& node) {
    if (!node) return;
    std::cout << node->value << " ";

    if (auto prev = node->prev.lock()) {   // safe atomic check
        printBackward(prev);
    }
}
```

#### General rule for breaking cycles

```
In any ownership graph with cycles:
  1. Identify the "owning" direction (parent→child, head→tail)
  2. Use shared_ptr for the owning direction
  3. Use weak_ptr for ALL back-links / cross-links

Common patterns:
  Tree:           parent→children = shared_ptr, child→parent = weak_ptr
  Doubly-linked:  next = shared_ptr, prev = weak_ptr
  Observer:       subject→observers = weak_ptr (subject doesn't own observers)
  Graph:          pick a spanning tree for shared_ptr, all other edges = weak_ptr
```

---

## Q14. `weak_ptr` for Caching

> *Design a thread-safe `weak_ptr`-based cache for expensive-to-create objects. What happens when a cache lookup finds an expired `weak_ptr`? What is the trade-off vs a `shared_ptr`-based cache?*

### Answer

#### Design: thread-safe `weak_ptr` cache

```cpp
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>

template<typename Key, typename Value>
class WeakCache {
public:
    using Factory = std::function<std::shared_ptr<Value>(const Key&)>;

    explicit WeakCache(Factory factory)
        : factory_(std::move(factory)) {}

    std::shared_ptr<Value> get(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = cache_.find(key);
        if (it != cache_.end()) {
            // Attempt to promote weak_ptr to shared_ptr
            if (auto sp = it->second.lock()) {
                return sp;     // cache HIT — object still alive
            }
            // weak_ptr expired — object was destroyed by external owners
            // fall through to re-create
        }

        // Cache MISS or expired entry — create a new object
        auto sp = factory_(key);
        cache_[key] = sp;     // store as weak_ptr (implicit conversion)
        return sp;
    }

    // Periodically clean up expired entries to prevent map bloat
    void purge() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = cache_.begin(); it != cache_.end(); ) {
            if (it->second.expired()) {
                it = cache_.erase(it);
            } else {
                ++it;
            }
        }
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<Key, std::weak_ptr<Value>> cache_;
    Factory factory_;
};
```

Usage:

```cpp
// Expensive-to-create texture objects
struct Texture {
    std::string name;
    std::vector<uint8_t> data;   // potentially hundreds of MB

    Texture(const std::string& n) : name(n) {
        // Simulate expensive loading (file I/O, GPU upload, etc.)
        data.resize(1024 * 1024);
    }
};

WeakCache<std::string, Texture> textureCache(
    [](const std::string& name) {
        return std::make_shared<Texture>(name);
    }
);

void renderScene() {
    auto tex1 = textureCache.get("brick.png");    // creates texture
    auto tex2 = textureCache.get("brick.png");    // cache hit — same object
    // tex1 and tex2 point to the same Texture

    auto tex3 = textureCache.get("wood.png");     // creates another texture
}
// After renderScene returns, tex1/tex2/tex3 are destroyed.
// Strong counts drop to 0. Textures are freed.
// Cache entries become expired weak_ptrs.

void renderNextFrame() {
    auto tex = textureCache.get("brick.png");     // cache miss — re-creates
}
```

#### What happens when a lookup finds an expired `weak_ptr`

1. `cache_.find(key)` succeeds — an entry exists for this key.
2. `it->second.lock()` returns an **empty `shared_ptr`** — the `weak_ptr`'s strong count is 0 because all external `shared_ptr` owners have released the object.
3. The object's destructor has already run. The memory for the object is freed (or pending, if `make_shared` was used — see Q15).
4. The cache treats this as a **miss**: it calls the factory to create a fresh object, stores the new `weak_ptr`, and returns the new `shared_ptr`.

The expired `weak_ptr` itself is still valid (not UB to call `lock()` or `expired()` on it) — it simply holds a reference to the control block, which persists until the weak count also drops to zero.

#### Thread-safety breakdown

| Operation | Protection | Atomic by itself? |
|---|---|---|
| `cache_.find()` / `cache_[key]` | `mutex_` (map is not thread-safe) | ❌ |
| `weak_ptr::lock()` | Internally uses CAS (atomic) | ✅ |
| `factory_(key)` | Runs under `mutex_` in this design | N/A |
| `shared_ptr` copy/move | Reference count is atomic | ✅ |

The `mutex_` protects the **map** — insertions, lookups, and erasures. The `lock()` call on the `weak_ptr` is independently atomic, but we still need the mutex to safely read the map entry.

**Potential improvement:** For high-concurrency scenarios, replace `std::mutex` with a `std::shared_mutex` (read-write lock) — cache hits (`lock()` succeeds) only need a shared/read lock, while misses (factory + insert) need an exclusive/write lock.

#### Trade-offs: `weak_ptr` cache vs `shared_ptr` cache

| Aspect | `weak_ptr` cache | `shared_ptr` cache |
|---|---|---|
| **Object lifetime** | Object dies when all external owners release it. Cache does not prevent eviction. | Cache keeps object alive indefinitely — object is never freed while in cache. |
| **Memory behaviour** | Self-regulating: unused objects are freed automatically. Memory usage follows actual demand. | Monotonically growing unless explicit eviction (LRU, TTL) is implemented. Can cause memory bloat. |
| **Cache hit rate** | Lower — objects may be evicted between accesses if no external owner holds a reference. | Higher — objects are always in cache once loaded. |
| **Eviction policy** | Implicit: "evict when unused." No knobs to tune. | Explicit: must implement LRU, LFU, TTL, or manual eviction. More complex but more control. |
| **Memory pressure** | Naturally adapts — under memory pressure, objects are freed as owners release them. | Does not adapt — cache holds references regardless of memory pressure (unless custom eviction logic exists). |
| **Reconstruction cost** | May re-create expensive objects if they're evicted too eagerly. | No reconstruction — objects are always cached. |
| **Map bloat** | Expired `weak_ptr` entries accumulate in the map. Need periodic `purge()`. | Map entries always point to live objects. No stale entries. |
| **Best for** | Objects that are expensive to hold in memory, accessed sporadically, and cheap enough to re-create when needed. | Objects that are expensive to create, frequently accessed, and small enough to keep in memory permanently. |

#### Hybrid approach

In practice, many systems combine both strategies:

```cpp
// Two-level cache:
// Level 1: shared_ptr cache — keeps "hot" objects alive (LRU, bounded size)
// Level 2: weak_ptr cache — remembers recently-evicted objects in case they're
//          still alive elsewhere in the system

// On lookup:
// 1. Check L1 (shared_ptr) → hit → return
// 2. Check L2 (weak_ptr) → lock() succeeds → promote to L1 → return
// 3. Miss → create → insert into L1 and L2 → return
```

---

## Q15. Control Block Lifetime with `weak_ptr`

> *After all `shared_ptr` instances are gone but `weak_ptr`s still exist, is the managed object destroyed? Is the control block destroyed? When is each freed?*

### Answer

#### Two separate lifetimes

A `shared_ptr`-managed resource has **two independent components** on the heap, each with its own lifetime:

| Component | Created when | Destroyed when |
|---|---|---|
| **Managed object** (the `Widget`, etc.) | `make_shared` or `new` | Strong count reaches **0** |
| **Control block** (ref counts, deleter, allocator) | `make_shared` or `shared_ptr` constructor | Strong count **AND** weak count both reach **0** |

```
Timeline:
                                                        
  ──────────────────────────────────────────────────────▶ time
  │                          │                          │
  create                 strong→0                   weak→0
  (make_shared)          (last shared_ptr gone)     (last weak_ptr gone)
  │                          │                          │
  │◄── object ALIVE ────────▶│                          │
  │                          │◄── object DESTROYED ────▶│
  │                                but control block    │
  │                                still allocated      │
  │                                                     │
  │◄───────── control block ALIVE ─────────────────────▶│
                                                        │
                                                    control block FREED
```

#### Why the control block must outlive the object

`weak_ptr` needs the control block to answer the question "is the object still alive?":

- `weak_ptr::expired()` reads the strong count from the control block.
- `weak_ptr::lock()` atomically reads and conditionally increments the strong count in the control block.

If the control block were freed when the object dies, these operations would access freed memory — undefined behaviour. So the control block **must** remain valid as long as any `weak_ptr` references it.

#### The `make_shared` complication — delayed memory release

This is where the two allocation strategies produce critically different behaviour:

**Strategy 1: Separate allocations (new + shared_ptr constructor)**

```cpp
std::shared_ptr<Widget> sp(new Widget());
```

```
Heap block A:          Heap block B:
┌────────────┐         ┌──────────────────┐
│   Widget   │         │  Control Block   │
│   object   │         │  strong: 1       │
│            │         │  weak: 0         │
└────────────┘         └──────────────────┘
```

When strong count → 0:
- **Block A is freed** (Widget's destructor runs, memory returned to allocator).
- Block B survives (control block still needed by `weak_ptr`s).

When weak count → 0:
- **Block B is freed** (control block no longer needed).

**Heap memory for the Widget is reclaimed immediately when the last `shared_ptr` goes away.** Good.

**Strategy 2: Single allocation (make_shared)**

```cpp
auto sp = std::make_shared<Widget>();
```

```
Single heap block:
┌──────────────────────────────────────┐
│  Control Block  │   Widget object    │
│  strong: 1      │   member1_         │
│  weak: 0        │   member2_         │
└──────────────────────────────────────┘
```

When strong count → 0:
- **Widget's destructor runs** — the object is logically destroyed.
- But the **memory cannot be freed**. The control block and the Widget live in the same allocation block. The allocator gave back one contiguous chunk; you must return the entire chunk or nothing.
- The Widget's memory is now **dead bytes** — destructor ran, but the memory is still allocated.

When weak count → 0:
- **The entire block is freed** (control block + Widget memory).

**Heap memory for the Widget remains allocated until the last `weak_ptr` is also gone.** This can be a problem.

#### Visualising the delayed-release issue

```
make_shared allocates:
┌───────────────────────────────────────────┐
│  Control Block  │   Widget (100 MB data)  │
└───────────────────────────────────────────┘

After all shared_ptrs destroyed, weak_ptrs remain:
┌───────────────────────────────────────────┐
│  Control Block  │   DEAD BYTES (100 MB)   │  ← destructor ran,
│  strong: 0      │   memory NOT freed      │     but allocator
│  weak: 3        │                         │     can't reclaim this
└───────────────────────────────────────────┘

After all weak_ptrs also destroyed:
(entire 100+ MB block finally returned to allocator)
```

#### When this matters in practice

The delayed release is usually **irrelevant** for small objects (a few hundred bytes). It becomes a real concern when:

1. **Large objects with long-lived `weak_ptr`s:**

   ```cpp
   // 500 MB image buffer
   auto img = std::make_shared<ImageBuffer>(width, height);  // single allocation

   // Cache stores weak_ptr
   cache[key] = img;

   img.reset();  // Last shared_ptr gone. Destructor runs.
                  // But 500 MB is NOT freed — cache's weak_ptr keeps the
                  // control block (and the colocated 500 MB) alive.
   ```

   Fix: use separate allocations for very large objects:
   ```cpp
   std::shared_ptr<ImageBuffer> img(new ImageBuffer(width, height));
   // Now object and control block are in separate heap blocks.
   // When strong count → 0, the 500 MB is freed immediately.
   // Only the small control block (~32–64 bytes) persists for weak_ptrs.
   ```

2. **Object pools / caches with many expired `weak_ptr`s:**

   If a cache holds thousands of `weak_ptr`s to objects created with `make_shared`, and those objects are all expired but the `weak_ptr` entries haven't been purged, the dead objects' memory is still allocated despite their destructors having run.

3. **High-memory-pressure environments** (embedded, mobile, game engines):

   Every byte counts. The delayed reclamation from `make_shared` can push the application over a memory budget.

#### Complete lifecycle example

```cpp
#include <memory>
#include <iostream>

struct HeavyObject {
    char data[1024 * 1024];  // 1 MB
    HeavyObject()  { std::cout << "HeavyObject constructed\n"; }
    ~HeavyObject() { std::cout << "HeavyObject DESTROYED\n"; }
};

int main() {
    std::weak_ptr<HeavyObject> wp;

    {
        auto sp = std::make_shared<HeavyObject>();   // 1 allocation: ~1 MB + control block
        wp = sp;

        std::cout << "strong: " << sp.use_count() << "\n";  // 1
        std::cout << "weak expired? " << wp.expired() << "\n";  // 0 (false)
    }
    // sp destroyed → strong count = 0
    // HeavyObject destructor runs ("HeavyObject DESTROYED")
    // BUT: 1 MB memory is NOT freed (make_shared colocated allocation)

    std::cout << "weak expired? " << wp.expired() << "\n";  // 1 (true)
    std::cout << "lock returns null? " << (!wp.lock()) << "\n";  // 1 (true)

    // At this point:
    //   - Object is logically dead (destructor ran)
    //   - Object memory is physically still allocated (~1 MB wasted)
    //   - Control block is alive (wp keeps it alive)

    wp.reset();  // weak count → 0 → entire block freed (1 MB + control block)
    std::cout << "Memory fully reclaimed\n";
}
```

Output:
```
HeavyObject constructed
strong: 1
weak expired? 0
HeavyObject DESTROYED
weak expired? 1
lock returns null? 1
Memory fully reclaimed
```

#### Summary table

| Event | Object destructor | Object memory freed | Control block freed |
|---|---|---|---|
| Strong count → 0 | ✅ Runs | ✅ If separate alloc (`new`) | ❌ |
| Strong count → 0 | ✅ Runs | ❌ If colocated (`make_shared`) | ❌ |
| Weak count → 0 (and strong already 0) | Already ran | ✅ (both strategies) | ✅ |

#### Decision guide

```
Is the object large AND weak_ptrs may outlive it significantly?
├── YES → use shared_ptr<T>(new T(...)) — separate allocations
│         Object memory freed promptly when strong count → 0
│
└── NO  → use make_shared<T>(...) — single allocation
          Better cache locality, fewer alloc calls, exception-safe
```
