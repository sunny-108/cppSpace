# Item 21 Exercises: `std::make_unique` / `std::make_shared` vs `new` — MCQs and Practical Tasks

---

## 📝 Multiple Choice Questions (15 Questions)

### Question 1
Which of the following is the **preferred** way to create a `std::shared_ptr<Widget>`?

A) `std::shared_ptr<Widget> sp(new Widget());`  
B) `auto sp = std::make_shared<Widget>();`  
C) `auto sp = std::shared_ptr<Widget>(std::make_unique<Widget>());`  
D) `Widget* raw = new Widget(); auto sp = std::shared_ptr<Widget>(raw);`  

**Answer: B**  
**Explanation:** `make_shared` is preferred: one allocation, exception-safe, and the type is written only once.

---

### Question 2
How many heap allocations does the following line perform?

```cpp
std::shared_ptr<int> sp(new int(42));
```

A) 0  
B) 1  
C) 2  
D) 3  

**Answer: C**  
**Explanation:** `new int(42)` is one allocation for the integer. The `shared_ptr` constructor then allocates a **separate control block**. Total: 2 allocations.

---

### Question 3
How many heap allocations does the following line perform?

```cpp
auto sp = std::make_shared<int>(42);
```

A) 0  
B) 1  
C) 2  
D) 3  

**Answer: B**  
**Explanation:** `make_shared` performs a **single** allocation containing both the integer and the control block in one contiguous chunk.

---

### Question 4
Why can the following code leak memory?

```cpp
void process(std::shared_ptr<Widget> sp, int priority);
int computePriority();   // may throw

process(std::shared_ptr<Widget>(new Widget), computePriority());
```

A) `shared_ptr` destructor is never called  
B) The compiler may execute `new Widget` before `computePriority()` and then `computePriority()` may throw before the `shared_ptr` is constructed  
C) `computePriority()` deletes the Widget  
D) There is no leak — `shared_ptr` always catches exceptions  

**Answer: B**  
**Explanation:** C++ may evaluate function arguments in any order. If `new Widget` runs, then `computePriority()` throws, the `shared_ptr` wrapper is never constructed, and the Widget is leaked.

---

### Question 5
Which scenario requires using `new` directly instead of `make_shared`?

A) When the type has no default constructor  
B) When a custom deleter must be provided  
C) When the object is small  
D) When exception safety is required  

**Answer: B**  
**Explanation:** Make functions do not accept a custom deleter parameter. You must use `new` and immediately pass it to the `shared_ptr` constructor together with the deleter.

---

### Question 6
Why can't `make_shared` be used with brace-initialization (`{}`)?

A) Brace-initialization is not valid C++  
B) Make functions use perfect forwarding with `()`, which selects a different constructor than `{}`  
C) `shared_ptr` does not support aggregate types  
D) The destructor cannot be called on brace-initialized objects  

**Answer: B**  
**Explanation:** `make_shared<T>(args...)` forwards `args` with parentheses. For types where `{}` selects a different constructor (e.g., `std::vector`'s initializer-list constructor), this will choose the wrong overload.

---

### Question 7
What is the key memory concern when `make_shared` is used with objects that many `weak_ptr`s observe?

A) Strong reference count overflows  
B) The object and control block share one allocation, so the object's memory cannot be freed until all `weak_ptr`s also expire  
C) The object is copied for each `weak_ptr`  
D) `make_shared` prevents `weak_ptr` from being created  

**Answer: B**  
**Explanation:** Because `make_shared` combines object + control block in one block, the whole allocation stays alive as long as any `weak_ptr` references the control block. With separate allocations (`new`), the object can be freed as soon as the strong count hits 0.

---

### Question 8
In what C++ standard was `std::make_unique` introduced?

A) C++11  
B) C++14  
C) C++17  
D) C++20  

**Answer: B**  
**Explanation:** `make_unique` was accidentally omitted from C++11 and added in C++14. It is easy to implement yourself for C++11 codebases.

---

### Question 9
Does `make_unique` provide the same single-allocation efficiency benefit as `make_shared`?

A) Yes — both perform a single allocation  
B) No — `make_unique` still allocates only the object; there is no control block  
C) No — `make_unique` always allocates two blocks  
D) Yes — both combine the object with a control block  

**Answer: B**  
**Explanation:** `unique_ptr` has no reference-counting control block. `make_unique` allocates only the managed object — its benefits are type deduplication and exception safety, not allocation efficiency.

---

### Question 10
Which statement is TRUE about using `new` safely when a custom deleter is needed?

A) Pass `new Widget` directly as a function argument inside any function call  
B) Construct the `shared_ptr` on a **separate statement** before passing it to any function  
C) Use `std::allocate_shared` instead  
D) Call `delete` immediately after passing to `shared_ptr`  

**Answer: B**  
**Explanation:** By constructing the `shared_ptr` on its own statement, you guarantee the memory is safely managed before any other function argument evaluation begins.

---

### Question 11
A class overrides `operator new` to allocate exactly `sizeof(MyClass)` bytes. What happens when `make_shared<MyClass>()` is called?

A) It uses the custom `operator new` correctly  
B) It bypasses the custom `operator new` because it allocates a combined block of `sizeof(MyClass) + control_block_size`  
C) It calls `operator new` twice  
D) Compile error  

**Answer: B**  
**Explanation:** `make_shared` requests a single allocation larger than `sizeof(MyClass)` to hold the control block too, so the custom `operator new` (which expects exactly `sizeof(MyClass)`) is bypassed.

---

### Question 12
What is the primary advantage of `make_unique` over `unique_ptr<T>(new T())`?

A) Faster destruction  
B) Exception safety in function argument lists and no type repetition  
C) Smaller binary output  
D) Thread-safety guarantees  

**Answer: B**  
**Explanation:** `make_unique` avoids both the type-repetition problem and the potential exception leak when used inside function argument lists, just like `make_shared`. Speed is not a benefit for `make_unique`.

---

### Question 13
What is printed? 

```cpp
#include <memory>
#include <iostream>

struct Foo {
    Foo()  { std::cout << "born "; }
    ~Foo() { std::cout << "dead "; }
};

int main() {
    {
        auto sp1 = std::make_shared<Foo>();
        auto sp2 = sp1;
        sp1.reset();
    }
    std::cout << "end";
}
```

A) `born dead end`  
B) `born end dead`  
C) `born dead dead end`  
D) Compile error  

**Answer: A**  
**Explanation:** `sp2` is the last strong owner. When the block ends, `sp2` is destroyed, the strong count drops to 0, and the `Foo` destructor runs. Then "end" is printed.

---

### Question 14
Which `make_shared` call is INCORRECT?

A) `auto sp = std::make_shared<std::string>("hello");`  
B) `auto sp = std::make_shared<int>(42);`  
C) `auto sp = std::make_shared<Widget>([](Widget* p){ delete p; });`  
D) `auto sp = std::make_shared<std::pair<int,int>>(1, 2);`  

**Answer: C**  
**Explanation:** `make_shared` does not accept a deleter. Option C would try to construct a `Widget` using a lambda as its constructor argument, which is wrong. If you need a custom deleter, use `std::shared_ptr<Widget>(new Widget(), deleter)`.

---

### Question 15
The following code is written to be exception-safe. Is it?

```cpp
void consume(std::unique_ptr<Widget> up, int n);
int getN();   // may throw

auto up = std::make_unique<Widget>();
consume(std::move(up), getN());
```

A) No — `make_unique` can still throw  
B) No — `getN()` can throw before `move` is called  
C) Yes — `up` is constructed before the function call, so the Widget is always owned  
D) No — `move` can throw  

**Answer: C**  
**Explanation:** `up` is constructed on a **separate statement** before the function call. Even if `getN()` throws, `up`'s destructor will clean up the Widget — no leak.

---

## 🛠️ Practical Exercises

---

## Exercise 1 — Code Review

### 🎯 Goal
Review the code below and identify every problem related to make functions and `new` usage. Suggest fixes.

```cpp
#include <memory>
#include <string>
#include <iostream>

class Connection {
public:
    Connection(const std::string& host, int port) {
        std::cout << "Connecting to " << host << ":" << port << "\n";
    }
    ~Connection() { std::cout << "Connection closed\n"; }
    void send(const std::string& data) {
        std::cout << "Sending: " << data << "\n";
    }
};

// Logs every deletion
void loggedDelete(Connection* c) {
    std::cout << "Logging deletion\n";
    delete c;
}

int getPort();   // may throw

// Version A
void versionA() {
    // Issue 1
    std::shared_ptr<Connection> c1(new Connection("localhost", 8080));

    // Issue 2 — potentially leaks
    std::shared_ptr<Connection> c2(new Connection("remote", getPort()));

    c1->send("hello");
}

// Version B
void versionB() {
    // Issue 3 — is this correct?
    auto c = std::make_shared<Connection>("db.server", 5432,
                 [](Connection* p){ std::cout << "cleanup\n"; delete p; });

    c->send("query");
}

// Version C
void versionC() {
    // Issue 4 — risky pattern
    Connection* raw = new Connection("api", 443);
    // ... many lines of code ...
    std::shared_ptr<Connection> c(raw);
    c->send("request");
}
```

### ✅ Issues and Fixes

**Issue 1 — Version A, line 1:** Type `Connection` is written twice. Use `auto c1 = std::make_shared<Connection>("localhost", 8080);`

**Issue 2 — Version A, line 2:** `new Connection(...)` may complete but `getPort()` may throw before the `shared_ptr` is constructed → memory leak. Fix: 
```cpp
// Either separate statements:
auto c2 = std::make_shared<Connection>("remote", 9000);  // or safe port

// Or if getPort() result is needed, assign to local first:
int port = getPort();   // if this throws, no allocation happened
auto c2 = std::make_shared<Connection>("remote", port);
```

**Issue 3 — Version B:** `make_shared` does not accept a custom deleter. This is a compile error. Fix: use `new` directly:
```cpp
auto c = std::shared_ptr<Connection>(
    new Connection("db.server", 5432),
    [](Connection* p){ std::cout << "cleanup\n"; delete p; }
);
```

**Issue 4 — Version C:** Bare `raw` pointer sits unprotected for "many lines of code". If an exception occurs before `shared_ptr<Connection> c(raw)`, the connection is leaked. Fix: wrap immediately:
```cpp
auto c = std::shared_ptr<Connection>(new Connection("api", 443));
// raw pointer is owned from the very next line
```

---

## Exercise 2 — Debugging

### 🎯 Goal
The code compiles and runs but exhibits an **unexpected memory leak** and a **wrong-constructor** bug. Find both.

```cpp
#include <memory>
#include <vector>
#include <iostream>

int main() {
    // Bug 1: Wrong constructor selected
    auto v = std::make_shared<std::vector<int>>(10, 20);
    // Intended: vector containing {10, 20}  (size 2, values 10 and 20)
    // Actual:   vector of 10 elements, all value 20

    std::cout << "vector size: " << v->size() << "\n";  // prints 10, not 2
    for (int x : *v) std::cout << x << " ";
    std::cout << "\n";

    // Bug 2: potential memory leak
    auto makeWidget = []() -> std::shared_ptr<int> {
        // Imagine computeValue() throws occasionally
        return std::shared_ptr<int>(new int(42));
    };
    // Called in a function argument list:
    auto process = [](std::shared_ptr<int> p, int n) {
        std::cout << "val=" << *p << " n=" << n << "\n";
    };
    auto computeN = []() -> int { return 7; };

    process(makeWidget(), computeN());  // Is this safe?
}
```

### 🔍 Diagnosis

**Bug 1 — Wrong constructor:**  
`std::make_shared<std::vector<int>>(10, 20)` uses `()` which calls `vector<int>(size, value)` → 10 elements of value 20.  
To get `{10, 20}` you need brace-initialization, which make functions cannot directly do.

**Fix for Bug 1:**
```cpp
// Option A: use new with braces
auto v = std::shared_ptr<std::vector<int>>(new std::vector<int>{10, 20});

// Option B: create initializer_list first
auto il = {10, 20};
auto v  = std::make_shared<std::vector<int>>(il);
```

**Bug 2 — Safe in this example, but fragile pattern:**  
`makeWidget()` returns a fully-constructed `shared_ptr`, so by the time it's used as an argument the memory is already protected. However, if `makeWidget` were written inline as `std::shared_ptr<int>(new int(42))` directly in the argument list, an exception from `computeN()` could strike before the `shared_ptr` is constructed — leaking the int.

Best fix: use `make_shared` inside `makeWidget`:
```cpp
auto makeWidget = []() -> std::shared_ptr<int> {
    return std::make_shared<int>(42);   // exception-safe always
};
```

---

## Exercise 3 — Implementation from Scratch

### 🎯 Goal
Implement a **`make_unique`** function template that works in C++11 (before it was standardised in C++14). Then implement a factory function `makeShape` that uses it correctly.

### 📋 Requirements
1. Implement `make_unique<T>(args...)` using perfect forwarding — handles single objects, not arrays.
2. Implement a `Shape` hierarchy: `Shape` (base), `Circle` (radius), `Rectangle` (width, height).
3. Implement `makeShape(std::string type, ...)` — a factory returning `std::unique_ptr<Shape>`.
4. Demonstrate exception safety: if the factory argument computation can throw, show the safe pattern.

### 💻 Starter Code
```cpp
#include <memory>
#include <string>
#include <iostream>
#include <stdexcept>

// TODO: implement make_unique<T>
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    // Your implementation here
}

// Shape hierarchy
struct Shape {
    virtual void draw() const = 0;
    virtual ~Shape() = default;
};

struct Circle : Shape {
    double radius;
    Circle(double r) : radius(r) {
        std::cout << "Circle(r=" << r << ") created\n";
    }
    ~Circle() { std::cout << "Circle destroyed\n"; }
    void draw() const override {
        std::cout << "Drawing circle r=" << radius << "\n";
    }
};

struct Rectangle : Shape {
    double w, h;
    Rectangle(double w, double h) : w(w), h(h) {
        std::cout << "Rectangle(" << w << "x" << h << ") created\n";
    }
    ~Rectangle() { std::cout << "Rectangle destroyed\n"; }
    void draw() const override {
        std::cout << "Drawing rectangle " << w << "x" << h << "\n";
    }
};

// TODO: implement makeShape
std::unique_ptr<Shape> makeShape(const std::string& type, double a, double b = 0.0) {
    // Your implementation here
}

int main() {
    auto c = makeShape("circle", 5.0);
    auto r = makeShape("rectangle", 4.0, 3.0);
    c->draw();
    r->draw();

    // Safe usage in argument list
    auto display = [](std::unique_ptr<Shape> s, int n) {
        s->draw();
        std::cout << "n=" << n << "\n";
    };

    auto up = makeShape("circle", 2.0);   // constructed first
    display(std::move(up), 99);           // then passed — safe even if 99 computation throws
}
```

### ✅ Reference Solution
```cpp
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

std::unique_ptr<Shape> makeShape(const std::string& type, double a, double b) {
    if (type == "circle")    return make_unique<Circle>(a);
    if (type == "rectangle") return make_unique<Rectangle>(a, b);
    throw std::invalid_argument("Unknown shape: " + type);
}
```

**Expected Output:**
```
Circle(r=5) created
Rectangle(4x3) created
Drawing circle r=5
Drawing rectangle 4x3
Circle(r=2) created
Drawing circle r=2
n=99
Circle destroyed
Rectangle destroyed
Circle destroyed
```

---

## Exercise 4 — Performance Optimization

### 🎯 Goal
The object pool below creates `shared_ptr` objects using `new` in a tight loop. Profile the inefficiency and rewrite using `make_shared` to reduce allocation overhead.

```cpp
#include <memory>
#include <vector>
#include <chrono>
#include <iostream>
#include <numeric>

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float mass;

    Particle(float x, float y, float z, float m)
        : x(x), y(y), z(z), vx(0), vy(0), vz(0), mass(m) {}
};

// ❌ SLOW VERSION — two allocations per particle
std::vector<std::shared_ptr<Particle>> createParticles_slow(int n) {
    std::vector<std::shared_ptr<Particle>> particles;
    particles.reserve(n);
    for (int i = 0; i < n; ++i) {
        particles.push_back(
            std::shared_ptr<Particle>(new Particle(
                float(i), float(i * 2), float(i * 3), 1.0f
            ))
        );
    }
    return particles;
}

// TODO: implement fast version using make_shared
std::vector<std::shared_ptr<Particle>> createParticles_fast(int n) {
    // Your implementation here
}

void benchmark(const std::string& label,
               std::function<std::vector<std::shared_ptr<Particle>>(int)> fn,
               int n) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto result = fn(n);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << label << ": " << ms << " µs  ("
              << result.size() << " particles)\n";
}

int main() {
    const int N = 100'000;
    benchmark("slow (new)", createParticles_slow, N);
    benchmark("fast (make_shared)", createParticles_fast, N);
}
```

### ✅ Fast Version

```cpp
std::vector<std::shared_ptr<Particle>> createParticles_fast(int n) {
    std::vector<std::shared_ptr<Particle>> particles;
    particles.reserve(n);
    for (int i = 0; i < n; ++i) {
        particles.push_back(
            std::make_shared<Particle>(
                float(i), float(i * 2), float(i * 3), 1.0f
            )
        );
    }
    return particles;
}
```

### 📊 Why This Is Faster

| | `new` + `shared_ptr` | `make_shared` |
|---|---|---|
| Allocations per particle | 2 | 1 |
| Control block location | Separate (random heap) | Adjacent to Particle |
| Cache misses when iterating | High — two pointer chases | Low — data is contiguous |
| Total allocator calls (N=100k) | 200,000 | 100,000 |

### 🔬 Expected Benchmark Result
On a typical machine with N=100,000:
- `slow (new)` : ~8,000–12,000 µs
- `fast (make_shared)` : ~4,000–6,000 µs (~2x improvement)

Exact numbers vary by hardware, OS scheduler, and compiler settings. The important point is that halving allocator calls reduces both time and memory fragmentation.

### ⚠️ Trade-off to Remember
If `Particle` is large and many `weak_ptr<Particle>` observers exist in a separate long-lived system, the `make_shared` version may keep the Particle's memory alive longer than expected (because object + control block share one allocation). In that case, the two-allocation `new` approach releases Particle memory sooner. Always consider this trade-off for large objects.
