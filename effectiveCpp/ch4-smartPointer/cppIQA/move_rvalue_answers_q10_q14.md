# Answers — Q10 to Q14: Move Semantics in Class Design

**Source:** `std::move`, Lvalue & Rvalue Interview Questions  
**Scope:** Section 3 — Move Semantics in Class Design

---

## Q10. Writing a move constructor

### The Class

```cpp
class Buffer {
    int* data_;
    size_t size_;
public:
    // ...
};
```

### Correct Move Constructor

```cpp
Buffer(Buffer&& other) noexcept
    : data_(other.data_), size_(other.size_)
{
    other.data_ = nullptr;
    other.size_ = 0;
}
```

**Step by step:**

1. **Steal the resources** — copy the pointer and size from `other` into `this` via the member initialiser list. This is a cheap pointer/integer copy — no heap allocation.
2. **Null out the source** — set `other.data_` to `nullptr` and `other.size_` to `0`. This leaves `other` in a valid, destructible state.

### Correct Move Assignment

```cpp
Buffer& operator=(Buffer&& other) noexcept {
    if (this != &other) {
        delete[] data_;          // (1) release our current resource
        data_ = other.data_;     // (2) steal other's resource
        size_ = other.size_;
        other.data_ = nullptr;   // (3) null out source
        other.size_ = 0;
    }
    return *this;
}
```

**Why self-assignment check?** `a = std::move(a)` is legal (though unusual). Without the guard, step (1) would delete the data that step (2) then tries to steal.

### What Happens If You Forget to Null Out the Source?

```cpp
// BAD — does not null out source
Buffer(Buffer&& other) noexcept
    : data_(other.data_), size_(other.size_)
{
    // other.data_ still points to the same memory as this->data_
}
```

When `other` is destroyed, its destructor calls `delete[] data_` — which frees the memory that `this` is now using. You get a **double-free**:

```
Timeline:
  1. Buffer b2 = std::move(b1);   → b2.data_ = b1.data_ (same address!)
  2. b1 goes out of scope          → ~Buffer() calls delete[] b1.data_  ← frees the memory
  3. b2 uses data_                 → UNDEFINED BEHAVIOUR (dangling pointer)
  4. b2 goes out of scope          → ~Buffer() calls delete[] b2.data_  ← double-free
```

### The Role of `noexcept`

#### Why it matters:

`std::vector` uses `std::move_if_noexcept` during reallocation. If your move constructor is **not** `noexcept`, the vector will **copy** instead of move to maintain strong exception safety.

```cpp
// What vector does internally during reallocation (simplified):
for (auto& elem : old_storage) {
    new (new_slot) T(std::move_if_noexcept(elem));
    //                ^^^^^^^^^^^^^^^^^^^^
    // If T's move ctor is noexcept → uses std::move (rvalue)
    // If T's move ctor may throw   → returns lvalue (forces copy)
}
```

#### The exception safety reasoning:

During reallocation, vector has moved some elements to new storage. If the move constructor throws mid-way:

- Some elements are in new storage (moved-to)
- Some elements are in old storage (moved-from = "valid but unspecified")
- The operation cannot be rolled back — moved-from elements can't be reliably restored

By copying instead of moving, the original storage remains intact if an exception occurs — the vector can simply deallocate the partial new storage and leave everything unchanged. **Strong exception guarantee preserved.**

#### Performance impact:

| Move ctor | Vector reallocation strategy | Cost for N elements |
|-----------|------------------------------|---------------------|
| `noexcept` | Move | O(N) cheap pointer copies |
| May throw | Copy | O(N) deep copies — potentially orders of magnitude slower |

#### Rule of thumb:

> If your move constructor can't throw, **always mark it `noexcept`**. A move that steals pointers/handles cannot throw. The only moves that might throw involve allocation (rare and usually a design smell).

### Alternative: Copy-and-Swap Move Assignment

```cpp
// Uses the by-value parameter trick
Buffer& operator=(Buffer other) noexcept {
    swap(*this, other);
    return *this;
}
```

This handles both copy and move assignment in a single function. When called with an rvalue, `other` is move-constructed; when called with an lvalue, `other` is copy-constructed. The old resource is released when `other` is destroyed.

---

## Q11. The Rule of Five and the Rule of Zero

### The Rule of Five

> If you define (or `= delete`) **any** of the five special member functions, you should explicitly define (or `= default` / `= delete`) **all five**.

The five:

| # | Special Member | Purpose |
|---|---------------|---------|
| 1 | Destructor | Release resources |
| 2 | Copy constructor | Deep copy of resources |
| 3 | Copy assignment | Deep copy + release old |
| 4 | Move constructor | Steal resources |
| 5 | Move assignment | Steal resources + release old |

**Why?** These five functions are intimately connected. If you need a custom destructor, it means your class manages a resource. If it manages a resource, the compiler-generated copy/move operations are almost certainly wrong (they'll do shallow copies of pointers, leading to double-free). Defining one without the others leaves gaps.

```cpp
// Rule of Five in practice:
class Buffer {
    int* data_;
    size_t size_;
public:
    // Destructor
    ~Buffer() { delete[] data_; }

    // Copy constructor
    Buffer(const Buffer& other)
        : data_(new int[other.size_]), size_(other.size_) {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // Copy assignment
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }

    // Move constructor
    Buffer(Buffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // Move assignment
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
};
```

### The Rule of Zero

> Design your classes so they **don't manage resources directly**. Use RAII types (`unique_ptr`, `shared_ptr`, `vector`, `string`) as members, and let the compiler generate all five special members correctly.

```cpp
// Rule of Zero in practice:
class Buffer {
    std::vector<int> data_;  // vector handles memory
public:
    // NO destructor, NO copy ctor, NO copy assign, NO move ctor, NO move assign
    // The compiler generates all five correctly because vector's are correct.
};
```

**Why is Rule of Zero preferred?**

| Aspect | Rule of Five | Rule of Zero |
|--------|-------------|--------------|
| Lines of code | ~40 lines of boilerplate | 0 lines |
| Bug surface | Self-assignment, exception safety, null-out gotchas | None — compiler can't get it wrong |
| Maintenance | Every new member must be added to all 5 functions | Automatic |
| `noexcept` correctness | Must manually verify and declare | Compiler deduces it from members |

### When You Must Fall Back to the Rule of Five

The Rule of Zero works when RAII wrappers exist for your resource. You need Rule of Five when:

| Scenario | Example |
|----------|---------|
| Wrapping a C resource with no existing RAII wrapper | `FILE*`, `sqlite3*`, POSIX file descriptors, GPU handles (`cudaStream_t`) |
| Building the RAII wrappers themselves | Implementing your own `unique_ptr`, `shared_ptr`, or scoped handle |
| Custom allocator or memory pool | Class manages raw memory blocks directly |
| Non-owning observer that needs custom copy semantics | Deep-copy-on-copy but shallow-move semantics |

**Best practice:** If you find yourself writing Rule of Five, consider whether you can first write a small RAII wrapper (Rule of Five applied once), then compose that wrapper into your main class (Rule of Zero).

```cpp
// Step 1: Small RAII wrapper (Rule of Five — written once)
class FileHandle {
    FILE* fp_;
public:
    explicit FileHandle(const char* path) : fp_(fopen(path, "r")) {}
    ~FileHandle() { if (fp_) fclose(fp_); }
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&& o) noexcept : fp_(o.fp_) { o.fp_ = nullptr; }
    FileHandle& operator=(FileHandle&& o) noexcept {
        if (this != &o) { if (fp_) fclose(fp_); fp_ = o.fp_; o.fp_ = nullptr; }
        return *this;
    }
    FILE* get() const { return fp_; }
};

// Step 2: Business class (Rule of Zero — no special members needed)
class LogParser {
    FileHandle file_;
    std::string filename_;
    std::vector<LogEntry> entries_;
    // Compiler-generated move works. Copy is deleted (because FileHandle is non-copyable).
};
```

---

## Q12. Implicitly deleted and suppressed moves

### The Code

```cpp
class Widget {
    std::string name_;
    ~Widget() { log("destroyed"); }
};
```

### Does `Widget` Have a Move Constructor?

**No.** The user-declared destructor **suppresses** implicit generation of the move constructor and move assignment operator.

### Does `Widget` Have a Copy Constructor?

**Yes.** The compiler still implicitly generates the copy constructor and copy assignment operator. (This is technically **deprecated** behaviour since C++11 — the standard says generating copy operations when a destructor is user-declared is deprecated, but it is still allowed for backward compatibility.)

### What Happens When You Try to Move?

```cpp
Widget w1;
Widget w2 = std::move(w1);  // Calls the COPY constructor, not move!
```

Because no move constructor exists, `std::move(w1)` produces an rvalue that binds to `const Widget&` — the copy constructor is selected. **The move silently degrades to a copy.** There is no error, no warning (unless you enable specific warnings).

### The Suppression Rules

The compiler generates special members according to these rules:

| User declares... | Copy ctor | Copy assign | Move ctor | Move assign | Destructor |
|-----------------|:---------:|:-----------:|:---------:|:-----------:|:----------:|
| Nothing | ✅ | ✅ | ✅ | ✅ | ✅ |
| Destructor | ✅† | ✅† | ❌ | ❌ | — |
| Copy ctor | — | ✅ | ❌ | ❌ | ✅ |
| Copy assign | ✅ | — | ❌ | ❌ | ✅ |
| Move ctor | ❌ (deleted) | ❌ (deleted) | — | ❌ | ✅ |
| Move assign | ✅ | ✅ | ❌ | — | ✅ |

† = deprecated, generated for backward compatibility

**Key insight:** Declaring **any** of {destructor, copy ctor, copy assign} suppresses both move operations. Declaring a move operation deletes the copy operations.

### The Rationale

> If the programmer needed a custom destructor, the class probably manages a resource. The compiler-generated move (which does member-wise move) might leave the source in a state that the custom destructor can't handle safely (e.g., double-free). Rather than risk silent bugs, the compiler refuses to generate the move.

### How to Fix

Explicitly default the move operations:

```cpp
class Widget {
    std::string name_;
public:
    ~Widget() { log("destroyed"); }

    // Explicitly request move operations
    Widget(Widget&&) = default;
    Widget& operator=(Widget&&) = default;

    // Must also explicitly handle copy (since move ctor is now declared)
    Widget(const Widget&) = default;
    Widget& operator=(const Widget&) = default;
};
```

By writing `= default`, you tell the compiler: "I've considered the interaction between my destructor and the move operations, and member-wise move is correct."

### Detecting This Problem

| Tool | Detection |
|------|-----------|
| Clang | `-Wdeprecated` warns about implicit copy generation when destructor is present |
| Clang-Tidy | `cppcoreguidelines-special-member-functions` — flags classes that declare some but not all |
| Core Guidelines | C.21: "If you define or `= delete` any copy, move, or destructor function, define or `= delete` them all" |

---

## Q13. `noexcept` and `move_if_noexcept`

### The Problem

During `std::vector` reallocation (e.g., `push_back` when `size() == capacity()`), the vector must transfer all existing elements from old storage to new storage. The question is: **move or copy?**

### Why Not Always Move?

Consider reallocation with a throwing move constructor:

```
Old storage: [ A  B  C  D  E ]
New storage: [ A' B' _  _  _  _  _  _ ]
                        ↑
                   C's move ctor THROWS here!

State after exception:
  Old: [ A?  B?  C  D  E ]   ← A and B are moved-from ("valid but unspecified")
  New: [ A'  B'  💥 ]         ← partially constructed, must be destroyed

Can we restore old storage? NO — A? and B? are in unspecified states.
```

**Strong exception safety is lost.** The vector can't roll back to its original state because moved-from elements can't be reliably restored.

### The Solution: `move_if_noexcept`

```cpp
// Simplified implementation:
template<typename T>
constexpr auto move_if_noexcept(T& x) noexcept {
    if constexpr (std::is_nothrow_move_constructible_v<T> ||
                  !std::is_copy_constructible_v<T>) {
        return std::move(x);    // move — safe (noexcept) or only option
    } else {
        return x;               // copy — preserve strong guarantee
    }
}
```

**The logic:**

| Move ctor is `noexcept`? | Copyable? | `move_if_noexcept` returns | Rationale |
|:------------------------:|:---------:|:--------------------------:|-----------|
| ✅ Yes | ✅ Yes | rvalue (move) | Safe — can't throw |
| ✅ Yes | ❌ No | rvalue (move) | Safe — can't throw |
| ❌ No | ✅ Yes | lvalue (copy) | Copy to preserve strong guarantee |
| ❌ No | ❌ No | rvalue (move) | No choice — move is the only option |

### With Copy — Strong Exception Safety Preserved

```
Old storage: [ A  B  C  D  E ]
New storage: [ A' B' _  _  _  _  _  _ ]
                        ↑
                   C's COPY ctor THROWS here!

State after exception:
  Old: [ A  B  C  D  E ]   ← untouched! Originals are still valid.
  New: [ A'  B'  💥 ]       ← destroy A' and B', deallocate new storage

Roll back: simply keep using old storage. Strong guarantee maintained ✅
```

### Performance Impact

```cpp
struct Heavy {
    std::vector<int> data;
    // Move ctor: moves one pointer + two ints ≈ 24 bytes
    // Copy ctor: allocates + copies N ints ≈ O(N)
};
```

| Scenario | `noexcept` on move ctor | Reallocation of 1M elements |
|----------|:-----------------------:|---------------------------|
| Move | ✅ | ~24 MB of pointer shuffling |
| Copy (fallback) | ❌ | Allocates and deep-copies every element — potentially GB of work |

This can be a **100×–1000× performance difference** in real workloads.

### How This Bites in Practice

```cpp
class Connection {
    Socket socket_;
    std::string label_;
public:
    // Forgot noexcept!
    Connection(Connection&& other)
        : socket_(std::move(other.socket_)),
          label_(std::move(other.label_)) {}
};

std::vector<Connection> connections;
connections.push_back(Connection{...});
// Every subsequent reallocation COPIES all connections
// even though the move ctor never actually throws
```

The fix is trivial — add `noexcept`:

```cpp
Connection(Connection&& other) noexcept
    : socket_(std::move(other.socket_)),
      label_(std::move(other.label_)) {}
```

Or better — don't write a move constructor at all (Rule of Zero). The compiler-generated one deduces `noexcept` from all members and gets it right.

### The `noexcept(noexcept(...))` Pattern

For generic code, conditionally propagate `noexcept`:

```cpp
template<typename T>
class Wrapper {
    T value_;
public:
    Wrapper(Wrapper&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(other.value_)) {}
};
```

This declares the move ctor as `noexcept` if and only if `T`'s move ctor is `noexcept`. Generic containers and wrappers should always do this.

---

## Q14. Move semantics with inheritance

### The Bug

```cpp
class Base {
    std::string name_;
public:
    Base(Base&& other) noexcept : name_(std::move(other.name_)) {}
};

class Derived : public Base {
    std::vector<int> data_;
public:
    Derived(Derived&& other) noexcept
        : Base(other),                     // ⚠️ BUG — calls Base COPY ctor
          data_(std::move(other.data_)) {}
};
```

### The Problem

`Base(other)` passes `other` as an **lvalue**. Remember from Q3: a named rvalue reference is an lvalue. Even though `other` is declared as `Derived&&`, the expression `other` inside the function body is an lvalue.

Overload resolution for `Base(other)`:
- `Base(Base&& other)` — needs an rvalue. `other` is an lvalue. **No match.**
- `Base(const Base& other)` — (implicitly generated) needs an lvalue. `other` is an lvalue. **Match!**

Result: `Base`'s **copy** constructor is called. The `name_` member is deep-copied instead of moved.

### The Fix

```cpp
Derived(Derived&& other) noexcept
    : Base(std::move(other)),          // ✅ explicitly move the base sub-object
      data_(std::move(other.data_)) {}
```

`std::move(other)` casts `other` to `Derived&&`. This binds to `Base&&` via slicing (derived-to-base rvalue reference conversion), so `Base`'s move constructor is selected.

### "But wait — after `std::move(other)` in the base initialiser, is `other` moved-from when we access `other.data_`?"

**No — only the `Base` sub-object is moved.** The `Base` move constructor only touches `Base`'s members (`name_`). The `Derived` portion (`data_`) is untouched by `Base(std::move(other))`. The member initialiser list processes base classes first, then members, so:

```
1. Base(std::move(other))     → moves other.name_  (Base sub-object)
2. data_(std::move(other.data_))  → moves other.data_ (Derived member)
```

After both: `other.name_` is moved-from, `other.data_` is moved-from. Each piece was moved exactly once. This is correct.

### Visualising the Object Layout

```
other (Derived):
┌─────────────────────────┐
│ Base sub-object          │
│   name_: "hello"  ──────┼──→  moved by Base(std::move(other))
├─────────────────────────┤
│ Derived members          │
│   data_: [1,2,3]  ──────┼──→  moved by data_(std::move(other.data_))
└─────────────────────────┘
```

### The General Rule

> In a hand-written move constructor, every **base class** and every **member** must be individually `std::move`'d.

```cpp
Derived(Derived&& other) noexcept
    : Base(std::move(other)),              // move base
      member1_(std::move(other.member1_)), // move each member
      member2_(std::move(other.member2_))
{}
```

### This Is Why `= default` Is Almost Always Better

```cpp
class Derived : public Base {
    std::vector<int> data_;
public:
    Derived(Derived&&) = default;  // compiler moves Base AND data_ correctly
};
```

The compiler-generated move constructor automatically calls `std::move` on every base and member. It cannot forget. It also correctly deduces `noexcept`.

### The Same Bug in Move Assignment

The identical mistake occurs in move assignment:

```cpp
// BAD
Derived& operator=(Derived&& other) noexcept {
    Base::operator=(other);           // ⚠️ calls Base COPY assignment
    data_ = std::move(other.data_);
    return *this;
}

// GOOD
Derived& operator=(Derived&& other) noexcept {
    Base::operator=(std::move(other));  // ✅ calls Base MOVE assignment
    data_ = std::move(other.data_);
    return *this;
}
```

### How to Catch This Bug

| Tool | Check |
|------|-------|
| Clang-Tidy | `performance-move-constructor-init` — warns when calling base/member copy ctor in a move ctor |
| Code review | Look for any base or member initialisation in a move ctor that doesn't use `std::move()` |
| Prevention | Prefer `= default` — the compiler can't get it wrong |
