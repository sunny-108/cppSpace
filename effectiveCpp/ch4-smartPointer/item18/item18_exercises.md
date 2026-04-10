# Item 18 Exercises: MCQs and Practical Tasks

## 📝 Multiple Choice Questions (15 Questions)

### Question 1
What is the primary advantage of `std::unique_ptr` over raw pointers?

A) It's faster than raw pointers  
B) It can be copied freely  
C) It automatically manages memory and prevents leaks  
D) It allows multiple owners  

**Answer: C**  
**Explanation:** `unique_ptr` follows RAII principles and automatically deletes the owned object when it goes out of scope, preventing memory leaks.

---

### Question 2
Which statement about `std::unique_ptr` is TRUE?

A) It can be copied but not moved  
B) It can be moved but not copied  
C) It can be neither copied nor moved  
D) It can be both copied and moved  

**Answer: B**  
**Explanation:** `unique_ptr` is move-only to enforce exclusive ownership. Copying would violate the single-owner semantics.

---

### Question 3
What is the size overhead of `std::unique_ptr` with a default deleter compared to a raw pointer?

A) Double the size  
B) Triple the size  
C) Same size  
D) Half the size  

**Answer: C**  
**Explanation:** With the default deleter, `unique_ptr` has the same size as a raw pointer (typically 8 bytes on 64-bit systems) due to empty base optimization.

---

### Question 4
What happens when you try to copy a `std::unique_ptr`?

```cpp
auto p1 = std::make_unique<int>(42);
auto p2 = p1;  // What happens?
```

A) `p2` becomes a copy, both point to different objects  
B) `p2` becomes a copy, both point to the same object  
C) Compilation error  
D) Runtime error  

**Answer: C**  
**Explanation:** The copy constructor of `unique_ptr` is deleted, resulting in a compilation error.

---

### Question 5
Which function should you prefer for creating `std::unique_ptr`?

A) `new` with constructor  
B) `std::make_unique`  
C) `std::make_shared` then convert  
D) Custom factory function with `new`  

**Answer: B**  
**Explanation:** `std::make_unique` is exception-safe, more concise, and preferred over using `new` directly.

---

### Question 6
What does `unique_ptr::get()` return?

A) A new `unique_ptr` pointing to the same object  
B) A raw pointer to the managed object  
C) A `shared_ptr` to the managed object  
D) The managed object by value  

**Answer: B**  
**Explanation:** `get()` returns a raw pointer without transferring ownership. The `unique_ptr` still owns the object.

---

### Question 7
What is the result of this code?

```cpp
std::unique_ptr<int> p1(new int(42));
std::unique_ptr<int> p2 = std::move(p1);
std::cout << (p1 == nullptr);
```

A) false  
B) true  
C) Compilation error  
D) Undefined behavior  

**Answer: B**  
**Explanation:** After `std::move`, `p1` is set to `nullptr` and `p2` takes ownership of the object.

---

### Question 8
Which syntax is correct for `unique_ptr` with arrays?

A) `std::unique_ptr<int> arr(new int[10]);`  
B) `std::unique_ptr<int[]> arr(new int[10]);`  
C) `std::unique_ptr<int*> arr(new int[10]);`  
D) `std::unique_ptr<std::array<int>> arr(new int[10]);`  

**Answer: B**  
**Explanation:** Use `unique_ptr<T[]>` for arrays to ensure `delete[]` is called instead of `delete`.

---

### Question 9
Can you convert `std::unique_ptr` to `std::shared_ptr`?

A) No, they are incompatible  
B) Yes, by moving the `unique_ptr`  
C) Yes, by copying the `unique_ptr`  
D) Only with explicit cast  

**Answer: B**  
**Explanation:** You can move a `unique_ptr` to a `shared_ptr`: `std::shared_ptr<T> sp = std::move(up);`

---

### Question 10
What is a custom deleter used for in `std::unique_ptr`?

A) To make deletion faster  
B) To handle resources that require special cleanup beyond `delete`  
C) To prevent deletion  
D) To enable copying  

**Answer: B**  
**Explanation:** Custom deleters allow `unique_ptr` to manage resources like file handles, sockets, or objects requiring special cleanup.

---

### Question 11
What is the type of a `unique_ptr` with a lambda deleter?

```cpp
auto deleter = [](int* p) { delete p; };
std::unique_ptr<int, decltype(deleter)> p(new int, deleter);
```

A) `std::unique_ptr<int>`  
B) `std::unique_ptr<int, std::function<void(int*)>>`  
C) `std::unique_ptr<int, decltype(deleter)>`  
D) `std::unique_ptr<int, void(*)(int*)>`  

**Answer: C**  
**Explanation:** Lambda deleter type must be part of the `unique_ptr` type using `decltype`.

---

### Question 12
What is the output of this code?

```cpp
std::unique_ptr<int> p1 = std::make_unique<int>(10);
std::unique_ptr<int> p2 = std::make_unique<int>(20);
p1 = std::move(p2);
// How many ints are alive now?
```

A) 0  
B) 1  
C) 2  
D) Undefined  

**Answer: B**  
**Explanation:** When `p1` is assigned from `p2`, `p1`'s original object (10) is deleted. Now only the object with value 20 exists, owned by `p1`.

---

### Question 13
Which statement is FALSE about `std::unique_ptr`?

A) It can be stored in STL containers  
B) It can be used with polymorphic types  
C) It has reference counting overhead  
D) It can have custom deleters  

**Answer: C**  
**Explanation:** `unique_ptr` does NOT have reference counting overhead—that's `shared_ptr`. `unique_ptr` has zero overhead with default deleters.

---

### Question 14
What happens if you call `release()` on a `unique_ptr`?

A) Deletes the managed object and returns nullptr  
B) Returns raw pointer and relinquishes ownership  
C) Returns raw pointer but keeps ownership  
D) Throws an exception  

**Answer: B**  
**Explanation:** `release()` returns the raw pointer and sets the `unique_ptr` to nullptr without deleting the object. You're responsible for deleting it manually.

---

### Question 15
Which is the correct way to pass a `unique_ptr` to a function that takes ownership?

A) `void func(std::unique_ptr<T>& p)`  
B) `void func(std::unique_ptr<T>* p)`  
C) `void func(std::unique_ptr<T> p)`  
D) `void func(const std::unique_ptr<T>& p)`  

**Answer: C**  
**Explanation:** To transfer ownership, pass by value: `func(std::move(ptr))`. The function takes ownership of the object.

---

## 💻 Practical Exercises

### Exercise 1: Code Review - Find the Bugs 🐛

**Difficulty:** Medium  
**Objective:** Identify and fix all bugs in the following code.

```cpp
#include <memory>
#include <iostream>
#include <vector>

class Resource {
public:
    Resource(int id) : id_(id) {
        std::cout << "Resource " << id_ << " created\n";
    }
    ~Resource() {
        std::cout << "Resource " << id_ << " destroyed\n";
    }
    int getId() const { return id_; }
private:
    int id_;
};

void processResource(std::unique_ptr<Resource>& res) {
    std::cout << "Processing: " << res->getId() << "\n";
}

std::unique_ptr<Resource> createResource(int id) {
    Resource* raw = new Resource(id);
    return std::unique_ptr<Resource>(raw);
}

int main() {
    // Bug 1: Double ownership
    Resource* raw = new Resource(1);
    std::unique_ptr<Resource> p1(raw);
    std::unique_ptr<Resource> p2(raw);
    
    // Bug 2: Attempting to copy
    auto p3 = std::make_unique<Resource>(2);
    auto p4 = p3;
    
    // Bug 3: Wrong array syntax
    std::unique_ptr<Resource> arr(new Resource[5]);
    
    // Bug 4: Using after move
    auto p5 = std::make_unique<Resource>(3);
    auto p6 = std::move(p5);
    std::cout << "ID: " << p5->getId() << "\n";
    
    // Bug 5: Storing temporary unique_ptr
    std::vector<std::unique_ptr<Resource>> resources;
    resources.push_back(createResource(4));
    auto temp = createResource(5);
    resources.push_back(temp);  // Wrong!
    
    // Bug 6: Passing by reference when ownership transfer intended
    auto p7 = std::make_unique<Resource>(6);
    processResource(p7);
    processResource(std::move(p7));  // What happens?
    
    return 0;
}
```

**Your Task:**
1. Identify all 6+ bugs
2. Explain why each is a bug
3. Provide corrected code
4. Explain the proper pattern for each case

---

### Exercise 2: Debugging - Memory Leak Hunt 🔍

**Difficulty:** Medium  
**Objective:** Find and fix memory leaks in this plugin system.

```cpp
#include <memory>
#include <map>
#include <string>
#include <iostream>

class Plugin {
public:
    virtual ~Plugin() {
        std::cout << "Plugin destroyed\n";
    }
    virtual void execute() = 0;
};

class AnalyticsPlugin : public Plugin {
private:
    int* data_;
public:
    AnalyticsPlugin() {
        data_ = new int[1000];
        std::cout << "AnalyticsPlugin created\n";
    }
    ~AnalyticsPlugin() {
        std::cout << "AnalyticsPlugin destroyed\n";
        // Missing: delete[] data_;
    }
    void execute() override {
        std::cout << "Analyzing...\n";
    }
};

class PluginManager {
private:
    std::map<std::string, Plugin*> plugins_;
    
public:
    void registerPlugin(const std::string& name, Plugin* plugin) {
        plugins_[name] = plugin;
    }
    
    void unregisterPlugin(const std::string& name) {
        // Missing: delete plugins_[name];
        plugins_.erase(name);
    }
    
    Plugin* getPlugin(const std::string& name) {
        auto it = plugins_.find(name);
        if (it != plugins_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    ~PluginManager() {
        plugins_.clear();  // Doesn't delete the plugins!
    }
};

int main() {
    PluginManager manager;
    
    // Leak 1: Raw pointer with new
    manager.registerPlugin("analytics", new AnalyticsPlugin());
    
    // Leak 2: Getting raw pointer and forgetting to track
    Plugin* p = manager.getPlugin("analytics");
    
    // Leak 3: Unregister doesn't delete
    manager.unregisterPlugin("analytics");
    
    return 0;
}
```

**Your Task:**
1. Run the code and identify memory leaks (use valgrind or AddressSanitizer)
2. Rewrite using `std::unique_ptr` throughout
3. Ensure proper ownership semantics
4. Add methods: `transferPlugin()` to move between managers
5. Verify no memory leaks remain

**Hints:**
- Change `Plugin*` to `std::unique_ptr<Plugin>`
- Use `std::move` for transferring ownership
- Consider return types carefully
- Should `getPlugin()` return raw pointer or reference?

---

### Exercise 3: Implementation from Scratch - Unique Pointer Cache 🔨

**Difficulty:** Advanced  
**Objective:** Implement a generic cache that stores `unique_ptr` to expensive objects.

```cpp
#include <memory>
#include <map>
#include <functional>
#include <optional>

template<typename Key, typename Value>
class UniqueCache {
private:
    // Your implementation here
    
public:
    // Constructor with max size
    explicit UniqueCache(size_t maxSize);
    
    // Add to cache, returns true if added, false if key exists
    bool add(Key key, std::unique_ptr<Value> value);
    
    // Get from cache - returns reference, throws if not found
    Value& get(const Key& key);
    
    // Try get from cache - returns optional reference
    std::optional<std::reference_wrapper<Value>> tryGet(const Key& key);
    
    // Remove from cache, returns the unique_ptr
    std::unique_ptr<Value> remove(const Key& key);
    
    // Check if key exists
    bool contains(const Key& key) const;
    
    // Get or create: if not in cache, creates using factory function
    template<typename Factory>
    Value& getOrCreate(const Key& key, Factory factory);
    
    // Clear all entries
    void clear();
    
    // Size and capacity
    size_t size() const;
    size_t capacity() const;
    
    // When cache is full, evict LRU (Least Recently Used)
    // Implement simple LRU policy
};

// Test your implementation
int main() {
    UniqueCache<int, std::string> cache(3);
    
    // Add items
    cache.add(1, std::make_unique<std::string>("one"));
    cache.add(2, std::make_unique<std::string>("two"));
    cache.add(3, std::make_unique<std::string>("three"));
    
    // Access
    std::cout << cache.get(1) << "\n";
    
    // Causes eviction of LRU (should be key 2)
    cache.add(4, std::make_unique<std::string>("four"));
    
    // Should not find key 2
    assert(!cache.contains(2));
    
    // Get or create
    auto& val = cache.getOrCreate(5, [] {
        return std::make_unique<std::string>("five");
    });
    
    return 0;
}
```

**Requirements:**
1. Implement LRU eviction policy
2. Thread-safety NOT required (for simplicity)
3. Must transfer ownership properly
4. No memory leaks
5. Efficient operations (use appropriate data structures)

**Bonus Challenges:**
- Add thread safety with mutex
- Implement TTL (Time To Live) for cache entries
- Add cache hit/miss statistics
- Support weak references for occasionally accessed items

---

### Exercise 4: Performance Optimization ⚡

**Difficulty:** Advanced  
**Objective:** Optimize this graph implementation for better performance while maintaining safety.

```cpp
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

// Inefficient version - optimize this!
class Node {
public:
    std::string name;
    std::vector<std::unique_ptr<Node>> children;
    
    explicit Node(std::string n) : name(std::move(n)) {}
    
    void addChild(std::unique_ptr<Node> child) {
        children.push_back(std::move(child));
    }
    
    // Problem: Deep copying is expensive
    std::unique_ptr<Node> clone() const {
        auto newNode = std::make_unique<Node>(name);
        for (const auto& child : children) {
            newNode->addChild(child->clone());
        }
        return newNode;
    }
    
    // Problem: Inefficient search
    Node* find(const std::string& targetName) {
        if (name == targetName) return this;
        for (const auto& child : children) {
            if (auto found = child->find(targetName)) {
                return found;
            }
        }
        return nullptr;
    }
    
    // Problem: Depth-first only
    void traverse(std::function<void(const Node&)> visitor) const {
        visitor(*this);
        for (const auto& child : children) {
            child->traverse(visitor);
        }
    }
};

class Graph {
private:
    std::unique_ptr<Node> root_;
    
public:
    explicit Graph(std::unique_ptr<Node> root) 
        : root_(std::move(root)) {}
    
    // Many traversals - very inefficient!
    int countNodes() const {
        int count = 0;
        root_->traverse([&count](const Node&) { count++; });
        return count;
    }
    
    int maxDepth() const {
        // Implement and optimize
        return 0;
    }
    
    std::vector<std::string> getAllNames() const {
        std::vector<std::string> names;
        root_->traverse([&names](const Node& n) {
            names.push_back(n.name);
        });
        return names;
    }
};

int main() {
    // Create large graph
    auto root = std::make_unique<Node>("root");
    for (int i = 0; i < 1000; ++i) {
        auto child = std::make_unique<Node>("child_" + std::to_string(i));
        for (int j = 0; j < 100; ++j) {
            child->addChild(
                std::make_unique<Node>("grandchild_" + std::to_string(j))
            );
        }
        root->addChild(std::move(child));
    }
    
    Graph g(std::move(root));
    
    // These operations are slow!
    auto count = g.countNodes();
    auto depth = g.maxDepth();
    auto names = g.getAllNames();
    
    return 0;
}
```

**Your Task:**
1. **Profile the code** - Identify bottlenecks
2. **Add caching** - Cache computed values (node count, depth, etc.)
3. **Optimize find** - Add hash map for O(1) lookup by name
4. **Optimize clone** - Consider copy-on-write or shared structure for immutable parts
5. **Add iterators** - Provide efficient iterators instead of collecting all names
6. **Memory pool** - Consider using a memory pool for node allocation
7. **Benchmark** - Compare before/after performance

**Performance Targets:**
- `find()` should be O(1) average case
- `countNodes()` should be O(1) after first call
- Cloning should share immutable data where possible
- Memory usage should be reasonable

**Optimization Strategies to Consider:**
```cpp
class OptimizedGraph {
private:
    std::unique_ptr<Node> root_;
    std::unordered_map<std::string, Node*> nameIndex_;  // Fast lookup
    mutable std::optional<int> cachedNodeCount_;        // Cached count
    mutable bool dirty_ = true;                         // Invalidation flag
    
    void rebuildIndex();
    void invalidateCache();
    
public:
    // Optimized methods here
};
```

---

### Exercise 5: Real-World Scenario - Smart Pointer PIMPL 🏗️

**Difficulty:** Advanced  
**Objective:** Implement the PIMPL (Pointer to Implementation) idiom using `unique_ptr`.

```cpp
// Widget.h - Header file
#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <string>

// Forward declaration only - no implementation details leaked!
class WidgetImpl;

class Widget {
public:
    Widget();
    ~Widget();  // Must be defined in .cpp where WidgetImpl is complete
    
    // Rule of 5 - need special handling for unique_ptr to incomplete type
    Widget(Widget&& other) noexcept;
    Widget& operator=(Widget&& other) noexcept;
    Widget(const Widget& other);
    Widget& operator=(const Widget& other);
    
    // Public interface
    void setName(const std::string& name);
    std::string getName() const;
    void doWork();
    
private:
    std::unique_ptr<WidgetImpl> pImpl_;
};

#endif // WIDGET_H
```

```cpp
// Widget.cpp - Implementation file
#include "Widget.h"
#include <iostream>
#include <vector>

// Now we can define the implementation
class WidgetImpl {
public:
    std::string name_;
    std::vector<int> data_;
    // ... other complex private members ...
    
    void doComplexWork() {
        std::cout << "Doing complex work for " << name_ << "\n";
        // ... complex implementation ...
    }
};

// Your task: Implement all these correctly!

Widget::Widget() : pImpl_(std::make_unique<WidgetImpl>()) {}

Widget::~Widget() = default;  // Must be in .cpp!

Widget::Widget(Widget&& other) noexcept = default;

Widget& Widget::operator=(Widget&& other) noexcept = default;

// Copy operations need special handling - implement these!
Widget::Widget(const Widget& other) {
    // TODO: How to deep copy the unique_ptr?
}

Widget& Widget::operator=(const Widget& other) {
    // TODO: How to deep copy the unique_ptr?
    return *this;
}

void Widget::setName(const std::string& name) {
    pImpl_->name_ = name;
}

std::string Widget::getName() const {
    return pImpl_->name_;
}

void Widget::doWork() {
    pImpl_->doComplexWork();
}
```

**Your Task:**
1. **Complete the implementation** of copy constructor and copy assignment
2. **Explain why** the destructor must be defined in the .cpp file
3. **Explain why** the default move operations work but default copy operations don't
4. **Add more methods** that demonstrate the PIMPL benefits:
   - `addData(int value)` - adds to internal vector
   - `getData()` - returns copy of data
   - `optimize()` - complex internal operation
5. **Create a test program** that shows:
   - Moving widgets (cheap)
   - Copying widgets (more expensive but works)
   - Recompilation: Change WidgetImpl without recompiling client code
6. **Measure compilation time** benefits of PIMPL

**Key Questions to Answer:**
- Why does `unique_ptr<WidgetImpl>` require complete type for destruction?
- What happens if you define destructor as `~Widget() = default;` in the header?
- How does PIMPL reduce compilation dependencies?
- What's the performance cost of PIMPL (indirection)?

---

### Exercise 6: Code Review - Design Patterns 🎨

**Difficulty:** Advanced  
**Objective:** Review and improve this implementation of multiple design patterns using `unique_ptr`.

```cpp
#include <memory>
#include <vector>
#include <iostream>

// Singleton with unique_ptr - Is this correct?
class Logger {
private:
    static std::unique_ptr<Logger> instance_;
    Logger() = default;
    
public:
    static Logger& getInstance() {
        if (!instance_) {
            instance_ = std::make_unique<Logger>();  // Thread-safe?
        }
        return *instance_;
    }
    
    void log(const std::string& msg) {
        std::cout << "Log: " << msg << "\n";
    }
};
std::unique_ptr<Logger> Logger::instance_ = nullptr;

// Factory pattern - Is this the best approach?
class Product {
public:
    virtual ~Product() = default;
    virtual void use() = 0;
};

class ConcreteProductA : public Product {
public:
    void use() override { std::cout << "Using Product A\n"; }
};

class ConcreteProductB : public Product {
public:
    void use() override { std::cout << "Using Product B\n"; }
};

class Factory {
public:
    static std::unique_ptr<Product> create(const std::string& type) {
        if (type == "A") {
            return std::unique_ptr<Product>(new ConcreteProductA());
        } else if (type == "B") {
            return std::unique_ptr<Product>(new ConcreteProductB());
        }
        return nullptr;
    }
};

// Observer pattern - Issues here?
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(int value) = 0;
};

class Subject {
private:
    std::vector<Observer*> observers_;  // Should this be unique_ptr?
    int value_;
    
public:
    void attach(Observer* obs) {
        observers_.push_back(obs);
    }
    
    void detach(Observer* obs) {
        // How to remove safely?
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), obs),
            observers_.end()
        );
    }
    
    void setValue(int val) {
        value_ = val;
        notify();
    }
    
private:
    void notify() {
        for (auto obs : observers_) {
            obs->update(value_);
        }
    }
};

int main() {
    // Test code
    return 0;
}
```

**Your Task:**
1. **Identify design issues** in each pattern
2. **Rewrite each pattern** with proper `unique_ptr` usage
3. **Discuss ownership**: Who owns what? When should unique_ptr vs raw pointer be used?
4. **Fix the Singleton**: Make it thread-safe and consider if unique_ptr is appropriate
5. **Fix the Factory**: Use `make_unique` and consider exception safety
6. **Fix Observer pattern**: Decide on ownership model (Should Subject own Observers?)
7. **Add proper tests** for each pattern

**Specific Questions:**
- Should Singleton use unique_ptr or just a static instance?
- In Observer pattern, should Subject own its Observers?
- What if an Observer needs to observe multiple Subjects?
- How to handle circular dependencies with unique_ptr?

---

## 🎯 Solutions Checklist

For each exercise, ensure your solution includes:

- [ ] **Compiles without warnings** (use `-Wall -Wextra -Werror`)
- [ ] **No memory leaks** (verified with valgrind/AddressSanitizer)
- [ ] **Exception safety** considered
- [ ] **Proper const correctness**
- [ ] **Move semantics** where appropriate
- [ ] **Comments explaining** ownership decisions
- [ ] **Test cases** covering edge cases
- [ ] **Performance** considerations documented

## 📊 Answer Key for MCQs

1. **C** - Automatic memory management
2. **B** - Move-only
3. **C** - Same size as raw pointer
4. **C** - Compilation error
5. **B** - `std::make_unique`
6. **B** - Raw pointer without ownership transfer
7. **B** - `p1` becomes nullptr
8. **B** - `unique_ptr<T[]>` for arrays
9. **B** - Via move operation
10. **B** - Special cleanup beyond delete
11. **C** - Type includes `decltype(deleter)`
12. **B** - Only one object (value 20)
13. **C** - No reference counting
14. **B** - Returns pointer, relinquishes ownership
15. **C** - Pass by value for ownership transfer

## 📚 Additional Resources

- **Run with sanitizers:**
  ```bash
  g++ -std=c++17 -fsanitize=address -fsanitize=undefined -g program.cpp
  ```

- **Check for memory leaks:**
  ```bash
  valgrind --leak-check=full ./program
  ```

- **Performance profiling:**
  ```bash
  g++ -std=c++17 -O2 -pg program.cpp
  ./a.out
  gprof a.out gmon.out
  ```

