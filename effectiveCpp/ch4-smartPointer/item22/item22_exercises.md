# Item 22 Exercises: Pimpl Idiom with `unique_ptr` — MCQs and Practical Tasks

---

## 📝 Multiple Choice Questions (15 Questions)

### Question 1
What does "Pimpl" stand for?

A) Pointer to Member  
B) Pointer to Implementation  
C) Private Implementation  
D) Both B and C are commonly used names  

**Answer: D**  
**Explanation:** "Pimpl" commonly expands to both "Pointer to Implementation" and "Private Implementation". Both names refer to the same idiom.

---

### Question 2
What is the primary compilation benefit of the Pimpl idiom?

A) Faster runtime performance  
B) Reduced binary size  
C) Implementation changes don't force recompilation of client translation units  
D) Enables virtual dispatch without vtables  

**Answer: C**  
**Explanation:** By moving implementation details (and their `#include` dependencies) to a `.cpp` file, changes to those details only require recompiling that one file. All other translation units that use the class are unaffected.

---

### Question 3
Why does `std::unique_ptr<Impl>` cause a compiler error when the destructor is defaulted in the header file?

A) `unique_ptr` cannot point to struct types  
B) `unique_ptr`'s default deleter uses `static_assert` to require the pointed-to type to be **complete** when `delete` is called, but `Impl` is only forward-declared in the header  
C) The destructor must always be `virtual` with Pimpl  
D) `unique_ptr` requires a custom deleter for Pimpl  

**Answer: B**  
**Explanation:** `unique_ptr<T>`'s deleter calls `delete` with a compile-time check that `T` is complete. The compiler generates the destructor at the header-include site where `Impl` is incomplete — triggering the `static_assert`.

---

### Question 4
Where must `Widget::~Widget() = default` be defined when using `unique_ptr` Pimpl?

A) In the header file, immediately after the class definition  
B) In the header file, inside the class body  
C) In the `.cpp` file, after the full definition of `Widget::Impl`  
D) It must never use `= default`; a manual body is required  

**Answer: C**  
**Explanation:** The `= default` definition must appear in the `.cpp` file **after** `Widget::Impl` is fully defined, so that `unique_ptr`'s deleter can see a complete type.

---

### Question 5
Which special member functions must be **declared** in the header (and defined in `.cpp`) when using `unique_ptr` Pimpl?

A) Only the destructor  
B) Only move constructor and move assignment  
C) Destructor, move constructor, and move assignment operator  
D) All six special member functions must always be declared  

**Answer: C**  
**Explanation:** The destructor is needed because `unique_ptr` requires a complete type to call `delete`. Move operations must also be declared (and defined in `.cpp`) because the compiler-generated versions are defined inline in the header where `Impl` is incomplete.

---

### Question 6
Why does `std::shared_ptr<Impl>` NOT require you to declare the destructor in the header?

A) `shared_ptr` cannot delete objects  
B) `shared_ptr` stores its deleter in the control block with type erasure; the deleter is captured at construction in `.cpp` where `Impl` is complete — no completeness check at the header's include site  
C) `shared_ptr` always uses virtual destructors  
D) `shared_ptr` never calls `delete`  

**Answer: B**  
**Explanation:** Unlike `unique_ptr`, `shared_ptr`'s deleter is not part of its type — it is erased into the control block when the `shared_ptr` is created (in the `.cpp`), where `Impl` is fully defined. The header-level destructor code just decrements a reference count via the erased deleter.

---

### Question 7
A colleague changes the implementation of `Widget::Impl` by adding a new private data member. Which files need to be recompiled?

A) All files that `#include "widget.h"`  
B) Only `widget.cpp`  
C) All `.cpp` files in the project  
D) No files — the change is invisible  

**Answer: B**  
**Explanation:** This is the Pimpl benefit. Since `Impl`'s definition is entirely inside `widget.cpp`, only that file needs to be recompiled. All other translation units only see the header, which is unchanged.

---

### Question 8
What is the key semantic problem with using `std::shared_ptr` for the Pimpl pointer instead of `std::unique_ptr`?

A) `shared_ptr` is always slower  
B) Copy-constructing the outer object would cause two `Widget` instances to **share the same** `Impl`, meaning mutating one changes the other  
C) `shared_ptr` cannot hold struct types  
D) There is no problem — `shared_ptr` is equally correct  

**Answer: B**  
**Explanation:** With `shared_ptr`, the default copy constructor copies the pointer — two `Widget` objects now point to the same `Impl`. Modifying one silently affects the other. `unique_ptr` forces you to write an explicit deep-copy constructor, making ownership semantics correct.

---

### Question 9
What happens if you forget to declare the move constructor in the header when using `unique_ptr` Pimpl?

A) The compiler auto-generates it correctly  
B) The compiler attempts to generate it inline at the header's include site, where `Impl` is incomplete — causing a compile error  
C) The move constructor is silently deleted  
D) The object becomes immovable at runtime only  

**Answer: B**  
**Explanation:** If you declare a destructor, the compiler suppresses auto-generation of move operations. If you *don't* declare a destructor and rely on the compiler, it tries to generate the move inline in the header — same incomplete-type problem. Either way you need explicit declarations + `.cpp` definitions.

---

### Question 10
What is printed when the following code runs?

```cpp
// Assume correct Pimpl implementation
Widget a(1);
Widget b = std::move(a);
b.show();        // prints id=1
a.show();        // what happens?
```

A) Prints id=1 twice  
B) Prints id=1, then crashes (undefined behaviour — `a.pImpl_` is null)  
C) Prints id=1, then `a.show()` dereferences a null `pImpl_` — undefined behaviour  
D) Both B and C describe the same issue  

**Answer: D**  
**Explanation:** After move, `a.pImpl_` is `nullptr`. Calling `a.show()` dereferences it — undefined behaviour (likely a crash). This is expected move semantics: the moved-from object is in a valid but unspecified state. Callers must not use a moved-from object.

---

### Question 11
Which statement about Pimpl and ABI stability is TRUE?

A) Pimpl has no effect on ABI  
B) Pimpl provides ABI stability: adding new private data members to `Impl` does not change `sizeof(Widget)` or its layout, so existing compiled code remains valid  
C) Pimpl breaks ABI because pointers change size  
D) ABI stability only matters for virtual classes  

**Answer: B**  
**Explanation:** Because `Widget` only holds a pointer (`pImpl_`), its size is always `sizeof(pointer)` regardless of how many members `Impl` has. Shared libraries can add fields to `Impl` without breaking binary compatibility with code compiled against the old version.

---

### Question 12
To implement a correct deep-copy constructor for a Pimpl class, which is the right approach?

A) `Widget::Widget(const Widget& rhs) : pImpl_(rhs.pImpl_) {}`  
B) `Widget::Widget(const Widget& rhs) : pImpl_(std::make_unique<Impl>(*rhs.pImpl_)) {}`  
C) `Widget::Widget(const Widget& rhs) = default;`  
D) Deep copy is not needed; the default compiler-generated copy is sufficient  

**Answer: B**  
**Explanation:** You must create a **new** `Impl` object by copying `*rhs.pImpl_`. Option A would attempt to copy a `unique_ptr` (compile error). Option C `= default` would try to copy the `unique_ptr` (also compile error because copy is deleted).

---

### Question 13
Which header file is the **minimum** required to use `unique_ptr` in the Pimpl header?

A) `<new>`  
B) `<utility>`  
C) `<memory>`  
D) No header needed — `unique_ptr` is built-in  

**Answer: C**  
**Explanation:** `std::unique_ptr` is defined in `<memory>`. This is the only include needed in the Pimpl header file itself (implementation-side includes go in the `.cpp`).

---

### Question 14
A library ships `widget.h` (Pimpl-based) as part of a public API. The developer adds a new private method to `Widget::Impl`. What must the library user do?

A) Rewrite their code  
B) Recompile their code against the new header  
C) Nothing — the header is unchanged, `sizeof(Widget)` is unchanged; simply relink with the new `.so`/`.dll`  
D) Recompile everything from scratch  

**Answer: C**  
**Explanation:** Since `Widget`'s header is unchanged and its size/layout is unchanged (only a pointer), the user's compiled code remains valid. They just need to link against the updated library — no recompilation required.

---

### Question 15
Which of the following is a valid reason to prefer raw pointer over `unique_ptr` for Pimpl?

A) Raw pointers are simpler to use  
B) `unique_ptr` has runtime overhead  
C) Raw pointers support custom deleters  
D) There is generally no good reason — `unique_ptr` is strictly safer and equally efficient  

**Answer: D**  
**Explanation:** `unique_ptr` provides automatic memory management with zero overhead compared to raw pointers (same size, no runtime cost). Using a raw pointer requires a manual `delete` in the destructor and careful handling of copy/move, with no benefit.

---

## 🛠️ Practical Exercises

---

## Exercise 1 — Code Review

### 🎯 Goal
Review the following Pimpl implementation and identify every problem. Suggest fixes.

```cpp
// ===== engine.h =====
#pragma once
#include <memory>
#include <string>        // Issue A
#include "fuel_system.h" // Issue B

class Engine {
public:
    Engine();
    ~Engine() = default;  // Issue C
    Engine(Engine&&) = default;   // Issue D
    Engine& operator=(Engine&&) = default;

    void start();
    std::string status() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// ===== engine.cpp =====
#include "engine.h"

struct Engine::Impl {
    FuelSystem fs;
    std::string state = "off";
    int rpm = 0;
};

Engine::Engine() : pImpl_(new Impl()) {}  // Issue E

void Engine::start() {
    pImpl_->state = "running";
    pImpl_->rpm = 3000;
}

std::string Engine::status() const {
    return pImpl_->state + " @ " + std::to_string(pImpl_->rpm) + " rpm";
}
```

### ✅ Issues and Fixes

**Issue A — `#include <string>` in the header:**  
`std::string` appears in the public API (`status()` return type), so the include is necessary. This one is acceptable. However, any include purely for `Impl`'s internals (like a third-party library) should move to the `.cpp`.

**Issue B — `#include "fuel_system.h"` in the header:**  
`FuelSystem` is only needed inside `Impl`. It must **not** be in the header — it defeats the entire purpose of Pimpl. Move it to `engine.cpp`.

**Issue C — `~Engine() = default` in the header:**  
At the header include site, `Impl` is an incomplete type. `unique_ptr`'s deleter `static_assert`s that the type is complete → **compile error**. Fix: declare only, define in `.cpp`.

**Issue D — Move operations `= default` in the header:**  
Same problem: the compiler tries to generate move operations inline where `Impl` is incomplete. Fix: declare in header, define `= default` in `.cpp`.

**Issue E — `new Impl()` instead of `make_unique`:**  
Prefer `std::make_unique<Impl>()` for exception safety and stylistic consistency.

### ✅ Fixed Version

```cpp
// ===== engine.h =====
#pragma once
#include <memory>
#include <string>

class Engine {
public:
    Engine();
    ~Engine();                               // declared only
    Engine(Engine&&) noexcept;               // declared only
    Engine& operator=(Engine&&) noexcept;    // declared only

    void start();
    std::string status() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// ===== engine.cpp =====
#include "engine.h"
#include "fuel_system.h"   // internal dependency — hidden here
#include <string>

struct Engine::Impl {
    FuelSystem  fs;
    std::string state = "off";
    int         rpm   = 0;
};

Engine::Engine()                        : pImpl_(std::make_unique<Impl>()) {}
Engine::~Engine()                       = default;
Engine::Engine(Engine&&) noexcept            = default;
Engine& Engine::operator=(Engine&&) noexcept = default;

void Engine::start() {
    pImpl_->state = "running";
    pImpl_->rpm   = 3000;
}

std::string Engine::status() const {
    return pImpl_->state + " @ " + std::to_string(pImpl_->rpm) + " rpm";
}
```

---

## Exercise 2 — Debugging

### 🎯 Goal
The code below compiles but has two runtime bugs. Find and fix them.

```cpp
// ===== session.h =====
#pragma once
#include <memory>
#include <string>

class Session {
public:
    Session(std::string host, int port);
    ~Session();
    Session(Session&&) noexcept;
    Session& operator=(Session&&) noexcept;

    // Bug 1 — copy constructor is missing; compiler will refuse to compile it anyway
    // but let's say someone tries to use shared_ptr for pImpl instead:

    void sendMessage(const std::string& msg);
    std::string getLog() const;

private:
    struct Impl;
    std::shared_ptr<Impl> pImpl_;   // ← using shared_ptr
};

// ===== session.cpp =====
#include "session.h"
#include <iostream>
#include <vector>

struct Session::Impl {
    std::string host;
    int         port;
    std::vector<std::string> log;

    Impl(std::string h, int p) : host(std::move(h)), port(p) {}
};

Session::Session(std::string host, int port)
    : pImpl_(std::make_shared<Impl>(std::move(host), port)) {}

Session::~Session()                       = default;
Session::Session(Session&&) noexcept            = default;
Session& Session::operator=(Session&&) noexcept = default;

void Session::sendMessage(const std::string& msg) {
    pImpl_->log.push_back(msg);
    std::cout << "[" << pImpl_->host << "] sent: " << msg << "\n";
}

std::string Session::getLog() const {
    std::string result;
    for (const auto& entry : pImpl_->log)
        result += entry + "\n";
    return result;
}

// ===== main.cpp =====
#include "session.h"
#include <iostream>

int main() {
    Session s1("server.com", 8080);
    s1.sendMessage("hello");

    Session s2 = s1;              // Bug 1: what does this do with shared_ptr?

    s2.sendMessage("world");      // Bug 2: accidental mutation

    std::cout << "s1 log:\n" << s1.getLog();
    std::cout << "s2 log:\n" << s2.getLog();
    // Expected: s1 log has "hello" only, s2 log has "world" only.
    // Actual:   BOTH logs show "hello" AND "world"!
}
```

### 🔍 Diagnosis

**Bug 1 — Shallow copy via `shared_ptr`:**  
`Session s2 = s1` uses the compiler-generated copy constructor which copies the `shared_ptr`. Now `s1.pImpl_` and `s2.pImpl_` point to the **same `Impl`**. Any mutation to one is visible in the other.

**Bug 2 — Same root cause:**  
`s2.sendMessage("world")` appends to the shared log. Both `s1.getLog()` and `s2.getLog()` return the same data from the same `Impl`.

### ✅ Fix — Switch to `unique_ptr` and Implement Deep Copy

```cpp
// ===== session.h (fixed) =====
#pragma once
#include <memory>
#include <string>

class Session {
public:
    Session(std::string host, int port);
    ~Session();
    Session(Session&&) noexcept;
    Session& operator=(Session&&) noexcept;
    Session(const Session& rhs);              // explicit deep copy
    Session& operator=(const Session& rhs);

    void sendMessage(const std::string& msg);
    std::string getLog() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;   // ← unique_ptr: forces correct semantics
};

// ===== session.cpp (copy ops added) =====
Session::Session(const Session& rhs)
    : pImpl_(std::make_unique<Impl>(*rhs.pImpl_)) {}  // deep copy

Session& Session::operator=(const Session& rhs) {
    *pImpl_ = *rhs.pImpl_;   // deep copy-assign
    return *this;
}
```

**Corrected Output:**
```
[server.com] sent: hello
[server.com] sent: world
s1 log:
hello
s2 log:
world
```

---

## Exercise 3 — Implementation from Scratch

### 🎯 Goal
Implement a `DatabaseConnection` class using the Pimpl idiom. The class must:

1. Hide all implementation details (connection string, query history, mock handle) behind `unique_ptr<Impl>`
2. Declare all required special member functions in the header
3. Support copy semantics (deep copy of connection details; new connection for copy)
4. Support move semantics
5. Expose: `connect()`, `query(std::string sql)`, `disconnect()`, `history() const`

### 💻 Skeleton

```cpp
// ===== db_connection.h =====
#pragma once
#include <memory>
#include <string>
#include <vector>

class DatabaseConnection {
public:
    explicit DatabaseConnection(std::string connectionString);

    // TODO: declare destructor
    // TODO: declare move constructor and move assignment
    // TODO: declare copy constructor and copy assignment

    void connect();
    void disconnect();
    void query(const std::string& sql);
    std::vector<std::string> history() const;
    bool isConnected() const;

private:
    struct Impl;
    // TODO: declare pImpl_ with correct smart pointer type
};

// ===== db_connection.cpp =====
#include "db_connection.h"
#include <iostream>
#include <vector>
#include <stdexcept>

struct DatabaseConnection::Impl {
    // TODO: define fields: connectionString, connected, queryLog, handle (int for mock)
};

// TODO: implement all declared functions
```

### ✅ Reference Solution

```cpp
// ===== db_connection.h =====
#pragma once
#include <memory>
#include <string>
#include <vector>

class DatabaseConnection {
public:
    explicit DatabaseConnection(std::string connectionString);
    ~DatabaseConnection();
    DatabaseConnection(DatabaseConnection&&) noexcept;
    DatabaseConnection& operator=(DatabaseConnection&&) noexcept;
    DatabaseConnection(const DatabaseConnection&);
    DatabaseConnection& operator=(const DatabaseConnection&);

    void connect();
    void disconnect();
    void query(const std::string& sql);
    std::vector<std::string> history() const;
    bool isConnected() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// ===== db_connection.cpp =====
#include "db_connection.h"
#include <iostream>
#include <stdexcept>

struct DatabaseConnection::Impl {
    std::string              connectionString;
    bool                     connected = false;
    int                      handle    = -1;      // mock handle
    std::vector<std::string> queryLog;

    explicit Impl(std::string cs) : connectionString(std::move(cs)) {}
};

DatabaseConnection::DatabaseConnection(std::string cs)
    : pImpl_(std::make_unique<Impl>(std::move(cs))) {}

DatabaseConnection::~DatabaseConnection()                              = default;
DatabaseConnection::DatabaseConnection(DatabaseConnection&&) noexcept = default;
DatabaseConnection& DatabaseConnection::operator=(DatabaseConnection&&) noexcept = default;

DatabaseConnection::DatabaseConnection(const DatabaseConnection& rhs)
    : pImpl_(std::make_unique<Impl>(*rhs.pImpl_)) {
    pImpl_->connected = false;   // copy gets its own disconnected state
    pImpl_->handle    = -1;
}

DatabaseConnection& DatabaseConnection::operator=(const DatabaseConnection& rhs) {
    if (this != &rhs) {
        *pImpl_ = *rhs.pImpl_;
        pImpl_->connected = false;
        pImpl_->handle    = -1;
    }
    return *this;
}

void DatabaseConnection::connect() {
    if (pImpl_->connected)
        throw std::runtime_error("Already connected");
    pImpl_->handle    = 42;   // mock
    pImpl_->connected = true;
    std::cout << "Connected to: " << pImpl_->connectionString << "\n";
}

void DatabaseConnection::disconnect() {
    if (!pImpl_->connected) return;
    pImpl_->handle    = -1;
    pImpl_->connected = false;
    std::cout << "Disconnected from: " << pImpl_->connectionString << "\n";
}

void DatabaseConnection::query(const std::string& sql) {
    if (!pImpl_->connected)
        throw std::runtime_error("Not connected");
    pImpl_->queryLog.push_back(sql);
    std::cout << "Query: " << sql << "\n";
}

std::vector<std::string> DatabaseConnection::history() const {
    return pImpl_->queryLog;
}

bool DatabaseConnection::isConnected() const {
    return pImpl_->connected;
}

// ===== main.cpp =====
int main() {
    DatabaseConnection db("host=localhost dbname=myapp");
    db.connect();
    db.query("SELECT * FROM users");
    db.query("UPDATE users SET active=1");

    DatabaseConnection db2 = db;   // deep copy — db2 starts disconnected
    db2.connect();
    db2.query("SELECT * FROM orders");

    std::cout << "\ndb history:\n";
    for (const auto& q : db.history())  std::cout << "  " << q << "\n";

    std::cout << "\ndb2 history:\n";
    for (const auto& q : db2.history()) std::cout << "  " << q << "\n";

    db.disconnect();
    db2.disconnect();
}
```

**Expected Output:**
```
Connected to: host=localhost dbname=myapp
Query: SELECT * FROM users
Query: UPDATE users SET active=1
Connected to: host=localhost dbname=myapp
Query: SELECT * FROM orders

db history:
  SELECT * FROM users
  UPDATE users SET active=1

db2 history:
  SELECT * FROM orders

Disconnected from: host=localhost dbname=myapp
Disconnected from: host=localhost dbname=myapp
```

---

## Exercise 4 — Performance Optimization

### 🎯 Goal
A class uses Pimpl correctly for compilation isolation, but accessing `Impl` fields through the pointer introduces an **extra indirection** on every call. Profile the hot path and apply targeted optimizations without abandoning Pimpl.

```cpp
// ===== particle.h =====
#pragma once
#include <memory>

class Particle {
public:
    Particle(float x, float y, float z, float mass);
    ~Particle();
    Particle(Particle&&) noexcept;
    Particle& operator=(Particle&&) noexcept;

    // Hot path: called millions of times per frame
    void update(float dt);
    float kineticEnergy() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// ===== particle.cpp =====
#include "particle.h"
#include <cmath>

struct Particle::Impl {
    float x, y, z;
    float vx, vy, vz;
    float mass;
};

Particle::Particle(float x, float y, float z, float mass)
    : pImpl_(std::make_unique<Impl>()) {
    pImpl_->x = x; pImpl_->y = y; pImpl_->z = z;
    pImpl_->vx = 0; pImpl_->vy = 0; pImpl_->vz = 0;
    pImpl_->mass = mass;
}
Particle::~Particle()                  = default;
Particle::Particle(Particle&&) noexcept      = default;
Particle& Particle::operator=(Particle&&) noexcept = default;

void Particle::update(float dt) {
    pImpl_->x += pImpl_->vx * dt;   // pointer dereference on every access
    pImpl_->y += pImpl_->vy * dt;
    pImpl_->z += pImpl_->vz * dt;
}

float Particle::kineticEnergy() const {
    float v2 = pImpl_->vx * pImpl_->vx
             + pImpl_->vy * pImpl_->vy
             + pImpl_->vz * pImpl_->vz;
    return 0.5f * pImpl_->mass * v2;
}
```

### 🔍 Performance Problem

Every single field access goes through `pImpl_->` — a pointer dereference. For code called millions of times per frame, this means:
- The CPU must load `pImpl_` pointer first, then follow it to the `Impl` block
- If `Impl` is on a cold cache line, the CPU stalls waiting for memory
- Individual `unique_ptr` allocations scatter `Impl` blocks all over the heap → cache thrashing when iterating many `Particle` objects

### ✅ Optimization Strategy 1 — Cache the Impl Reference in Hot Functions

In `.cpp` functions, immediately bind a reference to reduce repeated pointer dereferences:

```cpp
void Particle::update(float dt) {
    auto& d = *pImpl_;       // one dereference, then use d like a local struct
    d.x += d.vx * dt;
    d.y += d.vy * dt;
    d.z += d.vz * dt;
}

float Particle::kineticEnergy() const {
    const auto& d = *pImpl_;
    float v2 = d.vx * d.vx + d.vy * d.vy + d.vz * d.vz;
    return 0.5f * d.mass * v2;
}
```

The compiler can now see that `d` is a stable reference and may avoid repeated pointer loads. This is semantically identical but gives the optimizer more freedom.

### ✅ Optimization Strategy 2 — Use a Pool Allocator for Impl

For systems with many small `Impl` objects, replacing the default heap allocator with a pool allocator reduces fragmentation and improves cache locality:

```cpp
// In particle.cpp
#include <array>
#include <cstddef>

// Simple fixed-size pool (illustrative — use a production pool in practice)
class ImplPool {
    static constexpr std::size_t MAX = 10000;
    std::array<Particle::Impl, MAX> pool_;
    std::size_t next_ = 0;
public:
    Particle::Impl* allocate() { return &pool_[next_++]; }
    void deallocate(Particle::Impl*) { /* reclaim in production */ }
    static ImplPool& instance() {
        static ImplPool p;
        return p;
    }
};

// Custom deleter that returns to pool
struct PoolDeleter {
    void operator()(Particle::Impl* p) const {
        ImplPool::instance().deallocate(p);
    }
};

Particle::Particle(float x, float y, float z, float mass)
    : pImpl_(ImplPool::instance().allocate(), PoolDeleter{}) {
    pImpl_->x = x; pImpl_->y = y; pImpl_->z = z;
    pImpl_->vx = 0; pImpl_->vy = 0; pImpl_->vz = 0;
    pImpl_->mass = mass;
}
```

Note: changing `pImpl_` type to `unique_ptr<Impl, PoolDeleter>` keeps all Pimpl rules intact while using pool memory.

### ✅ Optimization Strategy 3 — Consider SoA for Hot Loops

If `update()` is called on **millions** of Particle objects in a tight loop, the Pimpl per-object allocation pattern is fundamentally at odds with cache-friendly Structure-of-Arrays (SoA) layout. In this case, consider a different architecture:

```
Per-object Pimpl (cache-unfriendly for hot loops):
  [x,y,z,vx,vy,vz,m]  [x,y,z,vx,vy,vz,m]  ...   scattered on heap

SoA layout (cache-friendly):
  [x, x, x, x, ...]   [y, y, y, y, ...]   [vx, vx, ...]
```

Use Pimpl for the configuration/API boundary (compilation isolation), and keep the actual physics data in a flat array managed by a `ParticleSystem` — the Pimpl class becomes a thin handle.

### 📊 Performance Summary

| Technique | Benefit | Trade-off |
|---|---|---|
| Bind `auto& d = *pImpl_` | Fewer pointer loads, better optimizer hints | None — pure improvement |
| Pool allocator | Contiguous `Impl` blocks → cache locality | More complex memory management |
| SoA in particle system | SIMD-friendly, L1/L2 cache stays hot | Redesign needed; less OOP |
