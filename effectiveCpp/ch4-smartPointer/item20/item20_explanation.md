# Item 20: Use `std::weak_ptr` for `std::shared_ptr`-like Pointers That Can Dangle

## 📌 Core Concept

`std::weak_ptr` is a smart pointer that **observes** an object managed by `std::shared_ptr` — without **owning** it.  
Think of it as a "keep an eye on this, but don't hold on to it" pointer.

A raw pointer can silently become a dangling pointer (pointing to deleted memory) with no way to know. `std::weak_ptr` solves this: it can tell you **whether the object it is watching is still alive** before you use it.

---

## 🗺️ The Problem `std::weak_ptr` Solves

### Dangling Pointer with Raw Pointer (Dangerous ❌)
```cpp
#include <memory>
#include <iostream>

int main() {
    int* rawPtr = nullptr;

    {
        auto sp = std::make_shared<int>(42);
        rawPtr = sp.get();          // raw pointer pointing to the same object
        std::cout << *rawPtr;       // OK: 42
    }
    // sp is gone, the int is deleted, but rawPtr still "points" to that memory

    // std::cout << *rawPtr;        // 💥 Undefined Behaviour — dangling pointer!
}
```

### Observe Safely with `std::weak_ptr` (Safe ✅)
```cpp
#include <memory>
#include <iostream>

int main() {
    std::weak_ptr<int> wp;

    {
        auto sp = std::make_shared<int>(42);
        wp = sp;                        // wp observes sp's object
        std::cout << *wp.lock();        // OK: 42
    }
    // sp is destroyed, the int is deleted

    if (wp.expired()) {
        std::cout << "Object is gone — safe, no crash!\n";
    }
}
```

---

## 🔑 Key Characteristics

### 1. **Does NOT Affect Reference Count**
`shared_ptr` keeps a reference count. When count hits 0, the object is destroyed.  
`weak_ptr` **does not increment** this count — it cannot keep an object alive.

```cpp
auto sp1 = std::make_shared<int>(10);
auto sp2 = sp1;            // ref count = 2
std::weak_ptr<int> wp = sp1; // ref count STILL = 2 (weak_ptr doesn't add to it)

sp1.reset();               // ref count = 1
sp2.reset();               // ref count = 0 → object destroyed
// wp now "expired"
```

### 2. **Two Ways to Check If Object Is Still Alive**

| Method | What It Does |
|---|---|
| `wp.expired()` | Returns `true` if the object has been destroyed |
| `wp.lock()` | Returns a `shared_ptr` — empty (null) if destroyed, valid otherwise |

```cpp
std::weak_ptr<int> wp = std::make_shared<int>(99);
// make_shared creates a temporary shared_ptr that immediately dies
// so wp is expired right away

if (wp.expired()) {
    std::cout << "Already gone!\n";   // This prints
}
```

### 3. **`lock()` Is the Safe Way to Access the Object**
Always use `lock()` to get a `shared_ptr` before using the value. This is **atomic** — the object cannot be destroyed between the check and the use.

```cpp
auto sp = std::make_shared<std::string>("hello");
std::weak_ptr<std::string> wp = sp;

// Safe access pattern:
if (auto locked = wp.lock()) {
    std::cout << *locked << "\n";   // "hello" — object is alive
} else {
    std::cout << "Object gone\n";
}
```

> **Why not `expired()` + dereference?** Between calling `expired()` and dereferencing, another thread could destroy the object. `lock()` does both atomically.

---

## 📖 Real-World Use Cases

---

### Use Case 1: Cache (Avoid Keeping Objects Alive Unnecessarily)

A cache should serve already-loaded objects but not prevent them from being freed if nobody else needs them.

```cpp
#include <memory>
#include <unordered_map>
#include <string>
#include <iostream>

class Widget {
public:
    Widget(int id) : id_(id) {
        std::cout << "Widget " << id_ << " created\n";
    }
    ~Widget() {
        std::cout << "Widget " << id_ << " destroyed\n";
    }
    int id() const { return id_; }
private:
    int id_;
};

// Cache stores weak_ptr — it can't keep widgets alive on its own
std::unordered_map<int, std::weak_ptr<Widget>> cache;

std::shared_ptr<Widget> loadWidget(int id) {
    auto it = cache.find(id);
    if (it != cache.end()) {
        auto sp = it->second.lock();    // Try to get a shared_ptr
        if (sp) {
            std::cout << "Cache hit for Widget " << id << "\n";
            return sp;
        }
    }
    // Object is gone or not yet loaded — create fresh
    auto sp = std::make_shared<Widget>(id);
    cache[id] = sp;                      // Store only weak reference
    return sp;
}

int main() {
    {
        auto w1 = loadWidget(1);    // Creates new Widget 1
        auto w2 = loadWidget(1);    // Cache hit — returns same object
    }
    // w1 and w2 go out of scope — Widget 1 is destroyed

    auto w3 = loadWidget(1);        // Cache miss — Widget 1 re-created
}
```

**Output:**
```
Widget 1 created
Cache hit for Widget 1
Widget 1 destroyed
Widget 1 created
Widget 1 destroyed
```

---

### Use Case 2: Observer Pattern (Observer Doesn't Own the Subject)

Observers should stop receiving notifications if they are destroyed — without the subject needing to know.

```cpp
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

class Observer {
public:
    virtual void onEvent(const std::string& event) = 0;
    virtual ~Observer() = default;
};

class ConcreteObserver : public Observer {
    std::string name_;
public:
    ConcreteObserver(std::string name) : name_(std::move(name)) {}
    void onEvent(const std::string& event) override {
        std::cout << name_ << " received: " << event << "\n";
    }
};

class Subject {
    // Stores weak_ptr — observers can die without us knowing
    std::vector<std::weak_ptr<Observer>> observers_;
public:
    void subscribe(std::shared_ptr<Observer> obs) {
        observers_.push_back(obs);
    }

    void notify(const std::string& event) {
        // Erase expired observers while notifying live ones
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&](const std::weak_ptr<Observer>& wp) {
                    if (auto obs = wp.lock()) {
                        obs->onEvent(event);
                        return false;   // Keep alive observer
                    }
                    return true;        // Remove dead observer
                }),
            observers_.end()
        );
    }
};

int main() {
    Subject subject;

    auto obs1 = std::make_shared<ConcreteObserver>("Observer A");
    auto obs2 = std::make_shared<ConcreteObserver>("Observer B");

    subject.subscribe(obs1);
    subject.subscribe(obs2);

    subject.notify("Click");        // Both A and B receive it

    obs2.reset();                   // Observer B is destroyed

    subject.notify("Scroll");       // Only A receives it, B is silently skipped
}
```

**Output:**
```
Observer A received: Click
Observer B received: Click
Observer A received: Scroll
```

---

### Use Case 3: Breaking Circular `shared_ptr` References (Memory Leak Fix)

If two objects hold `shared_ptr` to each other, neither's reference count ever reaches 0 → **memory leak**.

```cpp
// ❌ Circular Reference — MEMORY LEAK
struct Node {
    std::shared_ptr<Node> next;   // Both point to each other
    std::shared_ptr<Node> prev;
    ~Node() { std::cout << "Node destroyed\n"; }
};

int main() {
    auto a = std::make_shared<Node>();
    auto b = std::make_shared<Node>();
    a->next = b;
    b->prev = a;   // Circular! ref counts never reach 0
}
// Neither "Node destroyed" is ever printed ← leak!
```

**Fix: Use `weak_ptr` for the "back" pointer:**

```cpp
// ✅ Fixed with weak_ptr
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev;   // weak_ptr breaks the cycle
    ~Node() { std::cout << "Node destroyed\n"; }
};

int main() {
    auto a = std::make_shared<Node>();
    auto b = std::make_shared<Node>();
    a->next = b;
    b->prev = a;    // weak_ptr — doesn't increase ref count
}
// Both "Node destroyed" are printed ✅
```

---

### Use Case 4: Parent-Child Tree (Child Has Back-Reference to Parent)

```cpp
#include <memory>
#include <vector>
#include <string>
#include <iostream>

struct TreeNode {
    std::string name;
    std::weak_ptr<TreeNode>              parent;    // Back-link: non-owning
    std::vector<std::shared_ptr<TreeNode>> children; // Forward-link: owning

    TreeNode(std::string n) : name(std::move(n)) {}

    void addChild(std::shared_ptr<TreeNode> child) {
        child->parent = shared_from_this(); // requires : public enable_shared_from_this
        children.push_back(std::move(child));
    }
};
```

---

## 🧠 Mental Model Summary

```
shared_ptr  ──owns──▶  Object (Reference Count: N)
                           ▲
weak_ptr  ──watches──┘   (doesn't affect count)
                           │
               When count → 0: Object destroyed
               weak_ptr becomes "expired"
```

---

## ⚡ `weak_ptr` vs `shared_ptr` vs Raw Pointer

| Feature | `raw pointer` | `shared_ptr` | `weak_ptr` |
|---|---|---|---|
| Owns the object | ❌ (manually) | ✅ | ❌ |
| Keeps object alive | N/A | ✅ | ❌ |
| Can detect if object is gone | ❌ | N/A | ✅ |
| Automatic cleanup | ❌ | ✅ | N/A |
| Safe to use without check | ❌ | ✅ | ❌ — must `lock()` first |
| Size overhead | Pointer size | ~2x pointer (ptr + control block) | ~2x pointer |

---

## 🚫 Common Mistakes

### Mistake 1: Dereferencing Without `lock()`
```cpp
std::weak_ptr<int> wp = std::make_shared<int>(5);
// make_shared creates temporary — immediately destroyed!

// *wp;     // ❌ Can't dereference weak_ptr directly — compile error
auto sp = wp.lock();
if (sp) {
    std::cout << *sp;
}
```

### Mistake 2: Using `expired()` Then Accessing (Race Condition)
```cpp
// Dangerous in multi-threaded code:
if (!wp.expired()) {
    auto sp = wp.lock();    // Object could be destroyed between these two lines!
    // ...
}

// Correct — use lock() and check in one shot:
if (auto sp = wp.lock()) {
    // safe: object is alive for the entire if-block
}
```

### Mistake 3: `weak_ptr` From a Non-`shared_ptr` Managed Object
```cpp
Widget w;
// std::weak_ptr<Widget> wp = &w;  // ❌ Not possible — w is not shared_ptr-managed
```

---

## 📋 Quick Reference

```cpp
#include <memory>

auto sp = std::make_shared<int>(42);

// Create weak_ptr from shared_ptr
std::weak_ptr<int> wp = sp;

// Check if alive
bool dead  = wp.expired();

// Access safely (preferred pattern)
if (auto locked = wp.lock()) {
    std::cout << *locked;   // safe access
}

// Get use_count (shared owners only, not counting weak_ptr)
long count = wp.use_count();

// Reset (stop observing)
wp.reset();
```

---

## 🎯 When to Use `std::weak_ptr` — Decision Guide

```
Do you need a pointer that...

  Can dangle without UB?          → weak_ptr
  Should NOT keep object alive?   → weak_ptr
  Breaks a shared_ptr cycle?      → weak_ptr (back-link)
  Is an observer (not an owner)?  → weak_ptr
  Is the sole/primary owner?      → unique_ptr
  Shares ownership?               → shared_ptr
```
