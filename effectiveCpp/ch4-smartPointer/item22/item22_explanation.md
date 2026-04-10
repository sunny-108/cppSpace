# Item 22: When Using the Pimpl Idiom, Define Special Member Functions in the Implementation File

## 📌 Core Concept

The **Pimpl Idiom** (Pointer to Implementation — also called "compilation firewall") is a technique where you move a class's data members into a separate *implementation struct* and access it through a pointer in the header. This hides implementation details from the header file.

Item 22 is specifically about **why you must declare special member functions in the header but define them in the `.cpp` file** when you use `std::unique_ptr` as the Pimpl pointer.

---

## 🗺️ Why Pimpl Exists — The Problem It Solves

### Without Pimpl — Every Change Triggers Recompilation

```cpp
// widget.h — exposes ALL implementation details
#include <string>
#include <vector>
#include "gadget.h"    // any change to gadget.h recompiles everyone!

class Widget {
public:
    Widget();
    void doSomething();
private:
    std::string   name_;
    std::vector<double> data_;
    Gadget        gadget_;   // full type must be visible to every includer
};
```

Any file that `#include "widget.h"` must also indirectly know about `<string>`, `<vector>`, and `gadget.h`. If `gadget.h` changes, **every translation unit that includes `widget.h` must recompile** — even if no Widget-related code changed.

### With Pimpl — Header Becomes a Firewall

```cpp
// widget.h — hides ALL implementation details
#include <memory>

class Widget {
public:
    Widget();
    ~Widget();
    Widget(Widget&& rhs) noexcept;
    Widget& operator=(Widget&& rhs) noexcept;
    void doSomething();
private:
    struct Impl;                      // forward declaration only
    std::unique_ptr<Impl> pImpl_;     // pointer to incomplete type
};
```

```cpp
// widget.cpp — all the messy includes live HERE
#include "widget.h"
#include <string>
#include <vector>
#include "gadget.h"   // only widget.cpp recompiles when gadget.h changes

struct Widget::Impl {
    std::string   name;
    std::vector<double> data;
    Gadget        gadget;
};

Widget::Widget()  : pImpl_(std::make_unique<Impl>()) {}
Widget::~Widget() = default;
// ...
```

Now `gadget.h` changes → only `widget.cpp` recompiles. All other translation units that use `Widget` are untouched.

---

## 🔑 The Core Problem: Incomplete Types and `unique_ptr`

### Why the Special Members Must Be Defined in `.cpp`

`std::unique_ptr` uses a **static_assert** inside its default deleter to ensure the type it deletes is **complete** (fully defined) at the point `delete` is called.

```cpp
// Inside unique_ptr's default deleter (simplified):
template<typename T>
void default_delete<T>::operator()(T* ptr) const {
    static_assert(sizeof(T) > 0, "can't delete pointer to incomplete type");
    delete ptr;
}
```

The compiler generates the destructor, move operations, etc. from the **header**. At that point, `Widget::Impl` is only *declared*, not *defined* — it is an **incomplete type**. The `static_assert` fires.

```
widget.h ── compiler sees ──▶ unique_ptr<Impl>
                                    │
                                    ▼
                         Impl is incomplete here!
                         unique_ptr tries to delete it → ERROR
```

**The fix:** declare the special members in the header so the compiler knows they exist, but **define them in the `.cpp` file** where `Widget::Impl` is fully defined.

---

## 📖 Step-by-Step: The Correct Pimpl with `unique_ptr`

### Step 1 — The Header (Declarations Only)

```cpp
// widget.h
#pragma once
#include <memory>

class Widget {
public:
    Widget();                                      // declared
    ~Widget();                                     // declared — NOT defaulted here!
    Widget(Widget&& rhs) noexcept;                 // declared
    Widget& operator=(Widget&& rhs) noexcept;      // declared

    // Copy operations (opt-in if needed):
    Widget(const Widget& rhs);
    Widget& operator=(const Widget& rhs);

    void doSomething();

private:
    struct Impl;                        // incomplete type — forward decl
    std::unique_ptr<Impl> pImpl_;
};
```

### Step 2 — The Implementation File (Definitions)

```cpp
// widget.cpp
#include "widget.h"
#include <string>
#include <vector>
#include <iostream>

// NOW Impl is fully defined — unique_ptr can safely delete it
struct Widget::Impl {
    std::string   name   = "unnamed";
    std::vector<double> data  = {};
    int           version = 0;
};

// All special members defined HERE where Impl is complete:
Widget::Widget()
    : pImpl_(std::make_unique<Impl>()) {}

Widget::~Widget() = default;    // unique_ptr<Impl> can now call delete safely

Widget::Widget(Widget&& rhs) noexcept = default;
Widget& Widget::operator=(Widget&& rhs) noexcept = default;

Widget::Widget(const Widget& rhs)
    : pImpl_(std::make_unique<Impl>(*rhs.pImpl_)) {}

Widget& Widget::operator=(const Widget& rhs) {
    *pImpl_ = *rhs.pImpl_;
    return *this;
}

void Widget::doSomething() {
    pImpl_->name = "modified";
    std::cout << "Widget::doSomething() — name=" << pImpl_->name << "\n";
}
```

---

## ❌ Common Mistake: Defaulting the Destructor in the Header

```cpp
// widget.h ← WRONG
class Widget {
public:
    Widget();
    ~Widget() = default;   // ❌ compiler generates this WHERE Impl is INCOMPLETE
    // ...
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
```

**Error you'd see:**
```
error: invalid application of 'sizeof' to an incomplete type 'Widget::Impl'
note: required from 'void std::default_delete<_Tp>::operator()(_Tp*) const'
```

**Fix:** Move `= default` to the `.cpp` file:
```cpp
// widget.cpp
Widget::~Widget() = default;   // ✅ Impl is complete here
```

---

## 🔄 `unique_ptr` vs `shared_ptr` for Pimpl

This is a key insight in Item 22: `shared_ptr` does **not** have the incomplete-type problem.

### Why `shared_ptr` Doesn't Need The Trick

`unique_ptr` bakes the deleter **into its type** — `unique_ptr<T, Deleter>`. The deleter is invoked wherever the `unique_ptr` is destroyed, which includes compiler-generated special members in the header. The type must be complete there.

`shared_ptr` stores its deleter in the **control block** (erased-type storage). The deleter is captured at construction time in the `.cpp` file, where `Impl` *is* complete. When the `shared_ptr` later destroys the object, it uses the stored deleter — no need for the type to be complete at that point.

```cpp
// widget.h — with shared_ptr, no special member declarations needed
#include <memory>

class Widget {
public:
    Widget();
    // ✅ No destructor declaration needed
    // ✅ No move declarations needed (but shared_ptr copy semantics differ!)
    void doSomething();
private:
    struct Impl;
    std::shared_ptr<Impl> pImpl_;   // shared_ptr — no incomplete-type issue
};
```

### Trade-offs

| | `unique_ptr` Pimpl | `shared_ptr` Pimpl |
|---|---|---|
| Header declarations needed | ✅ Must declare destructor + moves | ❌ No special declarations needed |
| Ownership semantics | Exclusive (correct for most cases) | Shared (widgets share Impl!) |
| Size overhead | 1 pointer | 2 pointers (ptr + control block ptr) |
| Recommended | ✅ **Yes — correct semantics** | ❌ Usually wrong (unintended sharing) |

**Prefer `unique_ptr` Pimpl** — shared ownership of the implementation is usually not what you want. The extra header ceremony is a small price for correct semantics.

---

## 📖 Full Working Example

```cpp
// ===== gadget.h =====
#pragma once
#include <iostream>
struct Gadget {
    int id;
    Gadget(int i = 0) : id(i) { std::cout << "Gadget " << id << " created\n"; }
    ~Gadget()                  { std::cout << "Gadget " << id << " destroyed\n"; }
};

// ===== widget.h =====
#pragma once
#include <memory>

class Widget {
public:
    explicit Widget(int id);
    ~Widget();
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;
    Widget(const Widget&);
    Widget& operator=(const Widget&);

    void show() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// ===== widget.cpp =====
#include "widget.h"
#include "gadget.h"
#include <string>
#include <iostream>

struct Widget::Impl {           // fully defined only here
    int    id;
    Gadget gadget;
    std::string label;

    Impl(int i) : id(i), gadget(i), label("widget-" + std::to_string(i)) {}
};

Widget::Widget(int id)   : pImpl_(std::make_unique<Impl>(id)) {}
Widget::~Widget()        = default;
Widget::Widget(Widget&&) noexcept            = default;
Widget& Widget::operator=(Widget&&) noexcept = default;

Widget::Widget(const Widget& rhs)
    : pImpl_(std::make_unique<Impl>(*rhs.pImpl_)) {}

Widget& Widget::operator=(const Widget& rhs) {
    *pImpl_ = *rhs.pImpl_;
    return *this;
}

void Widget::show() const {
    std::cout << "Widget id=" << pImpl_->id
              << " label=" << pImpl_->label << "\n";
}

// ===== main.cpp =====
#include "widget.h"
#include <vector>

int main() {
    Widget w1(1);
    w1.show();

    Widget w2 = std::move(w1);   // move — w1 is now empty
    w2.show();

    Widget w3(w2);               // copy — deep copy of Impl
    w3.show();
}
```

**Output:**
```
Gadget 1 created
Widget id=1 label=widget-1
Widget id=1 label=widget-1
Gadget 1 created
Widget id=1 label=widget-1
Gadget 1 destroyed
Gadget 1 destroyed
```

---

## 🧠 Mental Model

```
widget.h (header — seen by ALL translation units)
─────────────────────────────────────────────
  class Widget {
      struct Impl;           ← incomplete: just a name
      unique_ptr<Impl> p;    ← pointer to unknown size
      ~Widget();             ← declared but NOT defined here
  };

widget.cpp (seen ONLY by this translation unit)
─────────────────────────────────────────────
  struct Widget::Impl { ... };   ← NOW complete!
  Widget::~Widget() = default;   ← unique_ptr<Impl> delete is safe here
```

---

## 📋 Quick-Reference Checklist

When writing a Pimpl class with `unique_ptr`:

```
☐ Forward-declare Impl in header:         struct Impl;
☐ Hold it as unique_ptr:                  std::unique_ptr<Impl> pImpl_;
☐ Declare destructor in header:           ~MyClass();
☐ Declare move ctor in header:            MyClass(MyClass&&) noexcept;
☐ Declare move assign in header:          MyClass& operator=(MyClass&&) noexcept;
☐ Declare copy ops in header (if needed): MyClass(const MyClass&);
                                          MyClass& operator=(const MyClass&);
☐ Define Impl fully in .cpp
☐ Define ALL declared special members in .cpp (even if = default)
```

---

## 🎯 When to Use the Pimpl Idiom

```
Large class with many private members that change frequently? → Pimpl
Want to hide third-party library details from your header?    → Pimpl
Library ABI stability required?                               → Pimpl
Performance-critical tiny class with 1-2 members?             → probably not worth it
```
