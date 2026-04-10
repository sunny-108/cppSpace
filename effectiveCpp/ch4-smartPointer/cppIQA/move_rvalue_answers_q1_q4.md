# Answers — Q1 to Q4: Value Categories & Foundations

**Source:** `std::move`, Lvalue & Rvalue Interview Questions  
**Scope:** Section 1 — Value Categories

---

## Q1. Taxonomy of value categories

### The Five Categories

C++11 replaced the simple C++03 lvalue/rvalue split with a refined taxonomy built on two orthogonal properties:

| Property | Meaning |
|----------|---------|
| **has identity** (i) | The expression refers to an object whose address can be taken / that persists beyond the expression |
| **can be moved from** (m) | The expression's resources may be safely stolen |

From these two bits, three primary and two composite categories emerge:

| Category | has identity | can be moved from | Examples |
|----------|:-----------:|:------------------:|----------|
| **lvalue** | ✅ | ❌ | named variables, `*ptr`, `arr[i]`, string literals |
| **xvalue** (eXpiring) | ✅ | ✅ | `std::move(x)`, `static_cast<T&&>(x)`, member of an rvalue |
| **prvalue** (pure rvalue) | ❌ | ✅ | literals (`42`), temporaries (`std::string("hi")`), arithmetic results (`a + b`) |

The two composite categories are unions:

| Composite | = | Meaning |
|-----------|---|---------|
| **glvalue** (generalised lvalue) | lvalue ∪ xvalue | everything that **has identity** |
| **rvalue** | xvalue ∪ prvalue | everything that **can be moved from** |

### Relationship Diagram

```
              expression
              /        \
           glvalue    rvalue
           /    \    /    \
        lvalue  xvalue  prvalue
```

- **glvalue** = has identity (left branch)
- **rvalue** = can be moved from (right branch)
- **xvalue** sits in the intersection — it has identity AND can be moved from

### Why This Taxonomy?

In C++03 the world was simple: lvalue (has a name/address) vs rvalue (temporary). C++11 needed a way to express "this object has identity but I'm done with it, so you can steal its guts." That concept is the **xvalue**. It enabled move semantics without breaking the existing lvalue/rvalue distinction.

### Key Takeaways for Interviews

- Every expression belongs to exactly one of {lvalue, xvalue, prvalue}.
- `std::move(x)` produces an **xvalue** — it has identity (still refers to `x`) but signals "safe to move."
- A prvalue has no identity — you can't take its address (pre-C++17 it was a temporary; post-C++17 it's an initialiser that hasn't materialised yet).

---

## Q2. Identify the value category

Given:

```cpp
int x = 42;
int& ref = x;
int&& rref = std::move(x);
```

### (a) `x` → **lvalue**

`x` is a named variable. It has identity (you can write `&x`) and is not implicitly movable.

```cpp
int* p = &x;    // ✅ — you can take address of an lvalue
```

### (b) `std::move(x)` → **xvalue**

`std::move(x)` is equivalent to `static_cast<int&&>(x)`. It produces a reference to an existing object (has identity) marked for potential moving.

```cpp
// Under the hood:
static_cast<int&&>(x);  // xvalue: has identity + can be moved from
```

### (c) `ref` → **lvalue**

`ref` is a named reference. The expression `ref` names the same object as `x`. Named references are always lvalues.

```cpp
int* p = &ref;   // ✅ — same address as &x
```

### (d) `rref` → **lvalue** ⚠️ (the critical gotcha)

**This is the most commonly missed case.** Despite being declared as `int&&`, the expression `rref` is an **lvalue** because it is a **named** variable.

The declaration `int&& rref` tells us what `rref` can *bind to* (rvalues only). But once bound and named, the expression `rref` itself is an lvalue.

```cpp
int* p = &rref;          // ✅ — you can take its address → lvalue
// To treat it as an rvalue again, you must explicitly cast:
int&& moved = std::move(rref);  // now std::move(rref) is an xvalue
```

**Rationale:** If `rref` were treated as an rvalue every time it appeared in an expression, its resources could be silently stolen on the first use, leaving it in a moved-from state for subsequent uses.

### (e) `42` → **prvalue**

`42` is an integer literal. It has no identity — you cannot take its address.

```cpp
// int* p = &42;  // ❌ — compile error, can't take address of a prvalue
```

### (f) `x + 1` → **prvalue**

Arithmetic expressions produce temporaries with no identity.

```cpp
// int* p = &(x + 1);  // ❌ — compile error
```

### (g) `std::string("hello")` → **prvalue**

This is a constructor call producing a temporary. Pre-C++17 it creates a temporary object; in C++17+ it is a prvalue initialiser (the object isn't materialised until needed — guaranteed copy elision).

```cpp
// In C++17:
std::string s = std::string("hello");
// No temporary is created — the prvalue directly initialises s
```

### (h) `static_cast<int&&>(x)` → **xvalue**

This is exactly what `std::move(x)` does. The cast produces an xvalue — the expression still refers to `x` (has identity) but is marked as movable.

### Summary Table

| Expression | Category | Why |
|-----------|----------|-----|
| `x` | lvalue | Named variable |
| `std::move(x)` | xvalue | Cast to `T&&` |
| `ref` | lvalue | Named reference |
| `rref` | **lvalue** | Named rvalue reference — still named! |
| `42` | prvalue | Literal, no identity |
| `x + 1` | prvalue | Arithmetic temporary |
| `std::string("hello")` | prvalue | Temporary / initialiser |
| `static_cast<int&&>(x)` | xvalue | Explicit cast to `T&&` |

---

## Q3. "A named rvalue reference is an lvalue"

### The Rule

```cpp
void process(Widget&& w) {
    // Inside this function, `w` is an LVALUE
    // even though its type is `Widget&&`
}
```

The type (`Widget&&`) tells us how `w` was *bound* — it can only bind to rvalues on the caller side. But within the function body, the expression `w` is a named variable and therefore an **lvalue**.

### Rationale — Preventing Accidental Double-Move

Consider what would happen if named rvalue references were rvalues:

```cpp
void dangerous(Widget&& w) {
    // If w were an rvalue...
    gadget.consume(w);     // (1) steals w's resources
    widget.reset(w);       // (2) OOPS — w is already moved-from!
}
```

If `w` were treated as an rvalue on every use, line (1) would move from `w`, leaving it in a "valid but unspecified" state. Then line (2) would silently operate on a zombie object.

By making `w` an lvalue, the language forces the programmer to **explicitly opt in** to the move at the exact point they intend it:

```cpp
void safe(Widget&& w) {
    gadget.inspect(w);          // (1) reads w, no move
    widget.reset(std::move(w)); // (2) explicitly moves — programmer's conscious decision
}
```

### The "Use It Once" Contract

The rule enforces a clean mental model:

1. **Caller side** — `std::move(obj)` signals "I'm done with `obj`, you may steal its resources."
2. **Callee side** — The parameter `w` is an lvalue. The callee may examine it multiple times safely. When it's finally ready to steal, it writes `std::move(w)` — one explicit, auditable transfer point.

### What Would Go Wrong Without This Rule?

| Problem | Explanation |
|---------|-------------|
| **Silent double-move** | Any expression mentioning `w` would trigger a move. Multiple references to `w` in the same function → use-after-move. |
| **Unpredictable evaluation order** | In `f(w, g(w))`, both uses of `w` would attempt to move, and the result would depend on evaluation order — undefined behaviour territory. |
| **Impossible to inspect before moving** | You couldn't read members of `w` without accidentally destroying it. |
| **Breaks generic code** | Templates that use a parameter more than once would silently corrupt data. |

### The General Principle

> **Value category is a property of the *expression*, not the *type*.**

- `Widget&&` is a **type** — "rvalue reference to Widget"
- `w` (the expression naming that variable) is an **lvalue** — it has identity, it persists, you can take `&w`.

To convert the lvalue back to an rvalue (xvalue), you must explicitly call `std::move(w)`. This is a deliberate, zero-cost cast — it generates no runtime code but changes the expression's value category so overload resolution picks the move constructor/assignment.

---

## Q4. Reference collapsing rules

### The Four Rules

When references-to-references arise (through template deduction or type aliases), the compiler collapses them according to these rules:

| Combination | Result | Mnemonic |
|------------|--------|----------|
| `T& &` | `T&` | lvalue ref to lvalue ref → lvalue ref |
| `T& &&` | `T&` | lvalue ref to rvalue ref → lvalue ref |
| `T&& &` | `T&` | rvalue ref to lvalue ref → lvalue ref |
| `T&& &&` | `T&&` | rvalue ref to rvalue ref → rvalue ref |

**The mnemonic:** If *either* reference is an lvalue reference (`&`), the result is an lvalue reference. Only `&& &&` produces `&&`.

> Think of `&` as "sticky" or "dominant." Alternatively: `&` is like 0 in multiplication — the moment one `&` appears, the result is `&`.

### How Forwarding References Work

Given:

```cpp
template<typename T>
void foo(T&& param);
```

`T&&` is a **forwarding reference** (a.k.a. universal reference) because `T` is deduced in the context of `&&`.

#### Case 1: Called with an lvalue

```cpp
Widget w;
foo(w);  // w is an lvalue
```

1. **Deduction:** The argument is an lvalue of type `Widget`, so `T` is deduced as `Widget&`.
2. **Substitution:** `param` type = `Widget& &&`
3. **Collapsing:** `Widget& &&` → `Widget&` (rule 2: `& &&` → `&`)

Result: `T = Widget&`, `param` is `Widget&` — an lvalue reference. The function receives the original object by reference, no copy.

#### Case 2: Called with an rvalue

```cpp
foo(Widget{});  // temporary, rvalue
```

1. **Deduction:** The argument is an rvalue, so `T` is deduced as `Widget` (no reference).
2. **Substitution:** `param` type = `Widget&&`
3. **Collapsing:** No collapsing needed — already `Widget&&`.

Result: `T = Widget`, `param` is `Widget&&` — an rvalue reference.

### Why This Enables Perfect Forwarding

Inside `foo`, `std::forward<T>(param)` uses `T` to decide what to do:

- If `T = Widget&` (lvalue was passed): `std::forward<Widget&>(param)` → returns `Widget&` (lvalue)
- If `T = Widget` (rvalue was passed): `std::forward<Widget>(param)` → returns `Widget&&` (rvalue)

The original value category is preserved — this is **perfect forwarding**.

```cpp
template<typename T>
void foo(T&& param) {
    bar(std::forward<T>(param));
    // If foo received an lvalue  → bar receives an lvalue
    // If foo received an rvalue → bar receives an rvalue
}
```

### Where Reference Collapsing Occurs

Reference collapsing is not limited to templates. It applies in four contexts:

1. **Template instantiation** — `template<typename T> void f(T&& param);`
2. **`auto&&` deduction** — `auto&& x = expr;`
3. **`typedef` / `using`** — `using Ref = int&; using RRef = Ref&&;` → `RRef` is `int&`
4. **`decltype`** — `decltype(expr)` may produce a reference type that then collapses

### Concrete Example with `typedef`

```cpp
using LRef = Widget&;

LRef&  a = w;   // Widget& &   → Widget&     (rule 1)
LRef&& b = w;   // Widget& &&  → Widget&     (rule 2)

using RRef = Widget&&;

RRef&  c = w;   // Widget&& &  → Widget&     (rule 3)
RRef&& d = std::move(w);  // Widget&& && → Widget&&    (rule 4)
```

### Common Interview Follow-Up: Why Can't Users Write References-to-References?

```cpp
int& & r = x;  // ❌ compile error — you cannot declare a reference to a reference
```

Reference collapsing only happens as a result of **type manipulation** (template deduction, aliases, `decltype`). You cannot explicitly write `& &` in a declaration. The collapsing rules are the compiler's mechanism for resolving situations where type substitution produces what would otherwise be an illegal reference-to-reference.
