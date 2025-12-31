
In C++, every expression is either an **lvalue** or an **rvalue**.

### Simple Definition

* **Lvalue ("Left Value"):** An object that has a **name** and a specific **memory address**. You can refer to it again later.
  * *Think:* "Location Value". It lives in a specific location.
* **Rvalue ("Right Value"):** A **temporary** value that doesn't have a name. It usually disappears immediately after the line of code finishes.
  * *Think:* "Read value" or "Temporary value". It's just data passing through.

---

### The "Address Test"

The easiest way to tell them apart:

* **Can you take its address using `&`?**
  * **Yes?** It's an **lvalue**.
  * **No?** It's an **rvalue**.

### Examples

I've created a code example for you in rvalue_lvalue_demo.cpp.

#### 1. Lvalues (The "Normal" Variables)

```cpp
int x = 10; 
// 'x' is an lvalue.
// It has a name.
// It has an address (&x works).
// It persists beyond this line.

std::string s1 = "Hello";
// 's1' is an lvalue.
```

#### 2. Rvalues (Temporaries)

```cpp
int x = 10; // '10' is an rvalue (it's just a literal number, no address).

int y = x + 5; 
// 'x + 5' evaluates to 15. That 15 is a temporary result. 
// It is an rvalue. You can't do &(x + 5).

std::string getName() { return "Sunny"; }
// The result of getName() is a temporary string. It is an rvalue.
```

### Why does this matter? (Move Semantics)

This distinction is the key to performance in modern C++.

* **If you have an Lvalue:** You must **COPY** it, because the user might still need the original variable later.
* **If you have an Rvalue:** You can **MOVE** (steal) from it, because it's a temporary object that is about to be destroyed anyway. No one else can possibly reference it.

#### Code Demonstration

I created a file rvalue_lvalue_demo.cpp to demonstrate this. Here is the output of running it:

```text
--- Lvalue Examples ---
Processed lvalue (persistent object): John
Processed lvalue (persistent object): Doe

--- Rvalue Examples ---
Processed rvalue (temporary object): Hello World   <-- Literal
Processed rvalue (temporary object): John Doe      <-- Result of concatenation
Processed rvalue (temporary object): John          <-- Result of std::move()
```

In the code, I defined two versions of `process`:

1. `void process(const std::string& s)`: Accepts lvalues (copies/references).
2. `void process(std::string&& s)`: Accepts rvalues (can move/steal).

The compiler automatically picks the "rvalue" version when it sees a temporary object, allowing you to write more efficient code.

Made changes.
