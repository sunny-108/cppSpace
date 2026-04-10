# Item 20 Exercises: `std::weak_ptr` — MCQs and Practical Tasks

---

## 📝 Multiple Choice Questions (15 Questions)

### Question 1
What is the primary purpose of `std::weak_ptr`?

A) To exclusively own a resource like `unique_ptr`  
B) To share ownership of a resource and keep it alive  
C) To observe a `shared_ptr`-managed object without affecting its lifetime  
D) To replace raw pointers in all situations  

**Answer: C**  
**Explanation:** `weak_ptr` observes an object managed by `shared_ptr` but does not increment the reference count — it cannot keep the object alive.

---

### Question 2
What does `std::weak_ptr::expired()` return when the managed object has been destroyed?

A) `false`  
B) `true`  
C) A null `shared_ptr`  
D) It throws an exception  

**Answer: B**  
**Explanation:** `expired()` returns `true` when the reference count of the associated `shared_ptr` has dropped to 0 and the object has been destroyed.

---

### Question 3
Which method should you use to safely access the object watched by a `weak_ptr`?

A) Dereference directly with `*wp`  
B) Call `wp.get()`  
C) Call `wp.lock()` and check the returned `shared_ptr`  
D) Cast to `shared_ptr` with `static_cast`  

**Answer: C**  
**Explanation:** `lock()` atomically checks liveness and returns a valid `shared_ptr` (or an empty one). Direct dereference is not supported on `weak_ptr`.

---

### Question 4
After the following code, what is the reference count of the `shared_ptr` managing the integer?

```cpp
auto sp1 = std::make_shared<int>(10);
auto sp2 = sp1;
std::weak_ptr<int> wp = sp1;
```

A) 1  
B) 2  
C) 3  
D) 0  

**Answer: B**  
**Explanation:** `sp1` and `sp2` each add 1 to the strong (shared) reference count. `weak_ptr` does **not** increment the strong count, so it stays at 2.

---

### Question 5
What is the result of calling `wp.lock()` when the watched object has already been destroyed?

A) Returns a `shared_ptr` pointing to garbage memory  
B) Throws `std::bad_weak_ptr`  
C) Returns an empty (null) `shared_ptr`  
D) Reconstructs the object  

**Answer: C**  
**Explanation:** `lock()` returns an empty `shared_ptr{}` when the object is gone. It never returns a dangling pointer.

---

### Question 6
Which problem does `weak_ptr` solve in the Observer pattern?

A) Observers can outlive the subject  
B) Dead observers are automatically removed without the subject needing explicit cleanup logic  
C) The subject takes ownership of observers  
D) It prevents the subject from being destroyed  

**Answer: B**  
**Explanation:** When an observer is destroyed, its `shared_ptr` ref count drops to 0. The subject's stored `weak_ptr` becomes expired, so notifications to dead observers can be silently skipped.

---

### Question 7
Why is the pattern `if (!wp.expired()) { auto sp = wp.lock(); }` considered dangerous in multithreaded code?

A) `expired()` is not thread-safe  
B) Between `expired()` returning `false` and `lock()` running, another thread may destroy the object  
C) `lock()` increments the reference count incorrectly  
D) It causes a double free  

**Answer: B**  
**Explanation:** The check and the lock are two separate operations — the object can be destroyed between them. `lock()` alone is the atomic, thread-safe solution.

---

### Question 8
What type of pointer should you use for the "back" (parent) pointer in a doubly-linked list to prevent memory leaks?

A) `std::shared_ptr`  
B) Raw pointer  
C) `std::unique_ptr`  
D) `std::weak_ptr`  

**Answer: D**  
**Explanation:** If both `next` and `prev` are `shared_ptr`, the two nodes keep each other alive forever (circular reference). Using `weak_ptr` for `prev` breaks the cycle.

---

### Question 9
What is a shared_ptr's "weak count"?

A) The number of `shared_ptr` instances sharing ownership  
B) The number of `weak_ptr` instances observing the same control block  
C) The number of expired `weak_ptr` instances  
D) A count maintained inside the managed object  

**Answer: B**  
**Explanation:** A `shared_ptr` control block holds two counts: a strong count (owning `shared_ptr`s) and a weak count (observing `weak_ptr`s). The control block itself is freed only when both reach 0.

---

### Question 10
Can you create a `weak_ptr` from a raw pointer directly?

```cpp
int x = 5;
std::weak_ptr<int> wp = &x;   // ?
```

A) Yes, it works fine  
B) Yes, but it leaks memory  
C) No — `weak_ptr` can only be constructed from a `shared_ptr` or another `weak_ptr`  
D) Yes, but `wp.lock()` always returns null  

**Answer: C**  
**Explanation:** `weak_ptr` requires a control block, which only exists for objects managed by `shared_ptr`. You cannot build one from a raw pointer.

---

### Question 11
Which scenario describes the best use case for `weak_ptr` caches?

A) A cache that must guarantee the object is always available  
B) A cache that serves already-loaded objects without preventing their destruction when no one else needs them  
C) A cache that owns all objects it stores  
D) A permanent object registry  

**Answer: B**  
**Explanation:** `weak_ptr` in a cache acts as a "remember if still alive" reference. If all external owners release the object, the cache doesn't artificially keep it alive.

---

### Question 12
After both `sp1` and `sp2` are reset, what does `wp.lock()` return?

```cpp
auto sp1 = std::make_shared<int>(7);
std::weak_ptr<int> wp = sp1;
auto sp2 = sp1;
sp1.reset();
sp2.reset();
auto result = wp.lock();
```

A) A valid `shared_ptr` to `7`  
B) An empty `shared_ptr`  
C) A `shared_ptr` to garbage  
D) Throws an exception  

**Answer: B**  
**Explanation:** After both `sp1` and `sp2` are reset, the strong count drops to 0 and the integer is destroyed. `wp.lock()` then returns an empty `shared_ptr`.

---

### Question 13
Which statement about `weak_ptr` and object lifetime is correct?

A) A `weak_ptr` extends an object's lifetime by one extra second  
B) A `weak_ptr` keeps the **control block** alive but not the managed object  
C) A `weak_ptr` keeps both the control block and the managed object alive  
D) A `weak_ptr` has no effect on either  

**Answer: B**  
**Explanation:** The managed object is destroyed when the strong count reaches 0. The control block (which holds both counts) stays alive until the weak count also reaches 0, so that `expired()` / `lock()` remain safe to call.

---

### Question 14
Which method returns the current number of `shared_ptr` owners (strong count) as seen from a `weak_ptr`?

A) `wp.count()`  
B) `wp.strong_count()`  
C) `wp.use_count()`  
D) `wp.ref_count()`  

**Answer: C**  
**Explanation:** `weak_ptr::use_count()` returns the strong reference count (same as `shared_ptr::use_count()`). It does not count `weak_ptr` observers themselves.

---

### Question 15
What is printed?

```cpp
#include <memory>
#include <iostream>

int main() {
    std::weak_ptr<int> wp;
    {
        auto sp = std::make_shared<int>(100);
        wp = sp;
        std::cout << "A:" << wp.expired() << " ";
    }
    std::cout << "B:" << wp.expired() << "\n";
}
```

A) `A:0 B:0`  
B) `A:1 B:1`  
C) `A:0 B:1`  
D) `A:1 B:0`  

**Answer: C**  
**Explanation:** Inside the block `sp` is alive so `expired()` is `false` (0). After the block `sp` is destroyed so `expired()` is `true` (1).

---

## 🛠️ Practical Exercises

---

## Exercise 1 — Code Review

### 🎯 Goal
Review the code below and identify all problems related to `weak_ptr` and `shared_ptr` usage.

```cpp
#include <memory>
#include <iostream>
#include <vector>

struct Observer {
    std::string name;
    Observer(std::string n) : name(std::move(n)) {}
};

class EventBus {
    std::vector<std::weak_ptr<Observer>> subs_;
public:
    void subscribe(std::shared_ptr<Observer> o) {
        subs_.push_back(o);
    }

    void broadcast(const std::string& msg) {
        for (auto& wp : subs_) {
            // BUG 1: is this safe?
            if (!wp.expired()) {
                auto sp = wp.lock();
                std::cout << sp->name << ": " << msg << "\n";
            }
        }
    }

    void cleanup() {
        for (int i = subs_.size() - 1; i >= 0; --i) {
            if (subs_[i].expired()) {
                subs_.erase(subs_.begin() + i);
            }
        }
    }
};

int main() {
    EventBus bus;
    {
        auto o1 = std::make_shared<Observer>("Alice");
        auto o2 = std::make_shared<Observer>("Bob");
        bus.subscribe(o1);
        bus.subscribe(o2);
        bus.broadcast("Hello");
        // o2 goes out of scope here
    }
    bus.broadcast("World");   // BUG 2: what happens here?
    bus.cleanup();
}
```

### ✅ Issues to Find
1. **BUG 1 — Race condition:** `!wp.expired()` followed by `wp.lock()` is not atomic. In multithreaded code the object can die between the two calls. Fix: use `if (auto sp = wp.lock())` directly.
2. **BUG 2 — Stale iteration:** After `o2` goes out of scope, `subs_` still contains an expired `weak_ptr`. `broadcast()` silently skips it (because `lock()` returns null) but never removes it — the vector grows with dead entries over time. Fix: clean up expired entries inside `broadcast()` or combine with `remove_if`.

### ✅ Fixed Version
```cpp
void broadcast(const std::string& msg) {
    subs_.erase(
        std::remove_if(subs_.begin(), subs_.end(),
            [&](const std::weak_ptr<Observer>& wp) {
                if (auto sp = wp.lock()) {
                    std::cout << sp->name << ": " << msg << "\n";
                    return false;   // keep
                }
                return true;        // erase expired
            }),
        subs_.end()
    );
}
```

---

## Exercise 2 — Debugging

### 🎯 Goal
The code below compiles but has a **memory leak**. Find the root cause and fix it using `weak_ptr`.

```cpp
#include <memory>
#include <iostream>

struct Employee {
    std::string name;
    std::shared_ptr<Employee> manager;   // ← points to the manager
    std::shared_ptr<Employee> report;    // ← points to a direct report

    Employee(std::string n) : name(std::move(n)) {
        std::cout << name << " hired\n";
    }
    ~Employee() {
        std::cout << name << " left the company\n";
    }
};

int main() {
    auto ceo  = std::make_shared<Employee>("CEO");
    auto vp   = std::make_shared<Employee>("VP");

    ceo->report  = vp;    // CEO's report is VP
    vp->manager  = ceo;   // VP's manager is CEO — circular!
}
// Expected: both destructors print. Actual: neither prints.
```

### 🔍 Diagnosis
`ceo->report = vp` makes CEO hold a strong reference to VP.  
`vp->manager = ceo` makes VP hold a strong reference to CEO.  
When `main` ends, `ceo` and `vp` locals are destroyed but each object still holds a `shared_ptr` to the other → strong count never reaches 0 → **memory leak**.

### ✅ Fix
```cpp
struct Employee {
    std::string name;
    std::weak_ptr<Employee>   manager;   // ← weak: manager doesn't "own" the report
    std::shared_ptr<Employee> report;

    Employee(std::string n) : name(std::move(n)) {
        std::cout << name << " hired\n";
    }
    ~Employee() {
        std::cout << name << " left the company\n";
    }

    void printManager() {
        if (auto mgr = manager.lock()) {
            std::cout << name << "'s manager is " << mgr->name << "\n";
        } else {
            std::cout << name << " has no manager\n";
        }
    }
};

int main() {
    auto ceo = std::make_shared<Employee>("CEO");
    auto vp  = std::make_shared<Employee>("VP");

    ceo->report  = vp;
    vp->manager  = ceo;   // weak_ptr — no circular ownership

    vp->printManager();
}
```

**Correct Output:**
```
CEO hired
VP hired
VP's manager is CEO
VP left the company
CEO left the company
```

---

## Exercise 3 — Implementation from Scratch

### 🎯 Goal
Implement a **`WeakCache<K, V>`** class template — a cache that:
- Returns a `shared_ptr<V>` if an entry is still alive
- Re-creates it via a factory function if the entry has expired
- Does not hold strong references (cache cannot keep values alive)
- Automatically purges expired entries when the cache size exceeds a threshold

### 📋 Requirements
1. Template parameters: `K` (key type), `V` (value type)
2. Method: `std::shared_ptr<V> get(const K& key, std::function<std::shared_ptr<V>()> factory)`
3. Method: `std::size_t size() const` — returns the number of **live** entries
4. Method: `void purge()` — removes all expired entries
5. Auto-purge when more than `maxSize` entries are stored (passed in constructor)

### 💻 Starter Code
```cpp
#include <memory>
#include <unordered_map>
#include <functional>
#include <iostream>

template <typename K, typename V>
class WeakCache {
public:
    explicit WeakCache(std::size_t maxSize = 100)
        : maxSize_(maxSize) {}

    std::shared_ptr<V> get(const K& key,
                           std::function<std::shared_ptr<V>()> factory) {
        // TODO: look up key in cache_
        // TODO: if found and not expired, return locked shared_ptr
        // TODO: otherwise call factory(), store weak_ptr, return shared_ptr
        // TODO: auto-purge if cache_ size exceeds maxSize_
    }

    std::size_t size() const {
        // TODO: count only live (non-expired) entries
    }

    void purge() {
        // TODO: remove all expired entries from cache_
    }

private:
    std::unordered_map<K, std::weak_ptr<V>> cache_;
    std::size_t maxSize_;
};

// --- Test ---
struct Texture {
    std::string path;
    Texture(std::string p) : path(std::move(p)) {
        std::cout << "Loading texture: " << path << "\n";
    }
    ~Texture() { std::cout << "Unloading texture: " << path << "\n"; }
};

int main() {
    WeakCache<std::string, Texture> cache(5);

    auto factory = [](const std::string& p) {
        return std::make_shared<Texture>(p);
    };

    auto t1 = cache.get("hero.png", [&]{ return factory("hero.png"); });
    auto t2 = cache.get("hero.png", [&]{ return factory("hero.png"); }); // cache hit

    std::cout << "Live entries: " << cache.size() << "\n";

    t1.reset(); t2.reset();   // hero.png unloaded

    auto t3 = cache.get("hero.png", [&]{ return factory("hero.png"); }); // miss — reload
    std::cout << "Live entries: " << cache.size() << "\n";
}
```

### ✅ Reference Solution
```cpp
std::shared_ptr<V> get(const K& key,
                       std::function<std::shared_ptr<V>()> factory) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        if (auto sp = it->second.lock()) {
            return sp;              // cache hit
        }
    }
    // cache miss or expired
    auto sp = factory();
    cache_[key] = sp;

    if (cache_.size() > maxSize_) {
        purge();
    }
    return sp;
}

std::size_t size() const {
    std::size_t count = 0;
    for (const auto& [k, wp] : cache_) {
        if (!wp.expired()) ++count;
    }
    return count;
}

void purge() {
    for (auto it = cache_.begin(); it != cache_.end(); ) {
        it = it->second.expired() ? cache_.erase(it) : std::next(it);
    }
}
```

**Expected Output:**
```
Loading texture: hero.png
Live entries: 1
Unloading texture: hero.png
Loading texture: hero.png
Live entries: 1
Unloading texture: hero.png
```

---

## Exercise 4 — Performance Optimization

### 🎯 Goal
The notification system below works correctly but is **inefficient**. Profile the inefficiencies and rewrite it.

```cpp
#include <memory>
#include <vector>
#include <string>
#include <iostream>

class Listener {
public:
    std::string id;
    Listener(std::string i) : id(std::move(i)) {}
    void hear(const std::string& msg) {
        std::cout << id << ": " << msg << "\n";
    }
};

class Broadcaster {
    std::vector<std::weak_ptr<Listener>> listeners_;
public:
    void add(std::shared_ptr<Listener> l) {
        listeners_.push_back(l);
    }

    // PROBLEM 1: scans all listeners on every notify, even expired ones
    // PROBLEM 2: never removes expired entries — vector grows unboundedly
    // PROBLEM 3: lock() called, then expired() checked — redundant
    void notify(const std::string& msg) {
        for (std::size_t i = 0; i < listeners_.size(); ++i) {
            if (!listeners_[i].expired()) {
                auto sp = listeners_[i].lock();  // second lock — wasteful
                sp->hear(msg);
            }
        }
    }
};
```

### 🔍 Problems
| # | Problem | Impact |
|---|---|---|
| 1 | `expired()` + `lock()` = two atomic operations on the control block | Wasteful — one `lock()` does both |
| 2 | Dead `weak_ptr` entries accumulate in the vector forever | Memory grows; iteration slows over time |
| 3 | Index-based loop prevents efficient erase-remove | |

### ✅ Optimized Version
```cpp
void notify(const std::string& msg) {
    // Single pass: notify live listeners AND remove expired ones
    auto it = listeners_.begin();
    while (it != listeners_.end()) {
        if (auto sp = it->lock()) {      // one call: check + get
            sp->hear(msg);
            ++it;
        } else {
            it = listeners_.erase(it);   // remove expired in-place
        }
    }
}
```

### 📊 Why This Is Better
- **One `lock()` call** per entry instead of `expired()` + `lock()` — halves control-block accesses.
- **Expired entries removed during the same pass** — no extra iteration needed, vector stays compact.
- **No extra allocation** — in-place erase on a `std::vector` is cache-friendly.

### 🧪 Benchmark to Run
```cpp
int main() {
    Broadcaster b;
    std::vector<std::shared_ptr<Listener>> alive;

    for (int i = 0; i < 10; ++i)
        alive.push_back(std::make_shared<Listener>("L" + std::to_string(i)));

    for (auto& l : alive) b.add(l);

    // Kill half the listeners
    for (int i = 0; i < 5; ++i) alive[i].reset();

    // notify should only call 5 listeners, not 10, and clean up the dead 5
    b.notify("ping");
}
```
