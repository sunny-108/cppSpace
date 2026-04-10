# Item 19 Exercises: MCQs and Practical Tasks

## 📝 Multiple Choice Questions (15 Questions)

### Question 1
What is the primary mechanism used by `std::shared_ptr` to manage shared ownership?

A) Garbage collection  
B) Reference counting  
C) Manual tracking  
D) Static allocation  

**Answer: B**  
**Explanation:** `shared_ptr` uses reference counting to track how many `shared_ptr` instances point to the same object. The object is deleted when the count reaches zero.

---

### Question 2
What is the size overhead of `std::shared_ptr` compared to a raw pointer?

A) No overhead - same size  
B) One additional pointer (for control block)  
C) Two times the size (object pointer + control block pointer)  
D) Three times the size  

**Answer: C**  
**Explanation:** `shared_ptr` stores two pointers: one to the managed object and one to the control block, making it typically 16 bytes on 64-bit systems.

---

### Question 3
What happens when you copy a `std::shared_ptr`?

A) The reference count stays the same  
B) The reference count is atomically incremented  
C) A new object is created  
D) Compilation error  

**Answer: B**  
**Explanation:** Copying a `shared_ptr` atomically increments the reference count because multiple `shared_ptr` instances now share ownership of the object.

---

### Question 4
Why is `std::make_shared` preferred over directly using `new` with `shared_ptr`?

A) It's the only way to create shared_ptr  
B) It performs a single allocation (object + control block) and is exception-safe  
C) It's slower but safer  
D) It automatically uses a custom deleter  

**Answer: B**  
**Explanation:** `make_shared` combines the object and control block into a single heap allocation, improving performance and cache locality. It's also exception-safe.

---

### Question 5
What is the main difference between `shared_ptr` and `unique_ptr` deleters?

A) `shared_ptr` doesn't support custom deleters  
B) `unique_ptr` deleter is part of the type, `shared_ptr` deleter is stored in control block  
C) They work exactly the same way  
D) `shared_ptr` deleters are faster  

**Answer: B**  
**Explanation:** With `unique_ptr`, the deleter type is part of the pointer type. With `shared_ptr`, the deleter is stored in the control block, so all `shared_ptr<T>` have the same type regardless of deleter.

---

### Question 6
What is the purpose of `std::weak_ptr`?

A) To provide a faster version of shared_ptr  
B) To break circular references and observe without owning  
C) To replace unique_ptr  
D) To make shared_ptr thread-safe  

**Answer: B**  
**Explanation:** `weak_ptr` provides a non-owning reference to an object managed by `shared_ptr`. It doesn't increase the reference count and helps break circular dependencies.

---

### Question 7
What does `std::enable_shared_from_this` allow you to do?

A) Create multiple shared_ptrs from the same raw pointer  
B) Safely obtain a shared_ptr to `this` from within a member function  
C) Convert unique_ptr to shared_ptr  
D) Make shared_ptr thread-safe  

**Answer: B**  
**Explanation:** `enable_shared_from_this` provides `shared_from_this()` method that returns a `shared_ptr` to the current object, sharing the same control block.

---

### Question 8
What is the result of this code?

```cpp
Widget* raw = new Widget();
std::shared_ptr<Widget> sp1(raw);
std::shared_ptr<Widget> sp2(raw);
```

A) sp1 and sp2 safely share ownership  
B) Undefined behavior - double delete  
C) Compilation error  
D) Memory leak  

**Answer: B**  
**Explanation:** Creating two `shared_ptr` instances from the same raw pointer creates two separate control blocks, leading to double deletion when both go out of scope.

---

### Question 9
How do you check if a `weak_ptr` still points to a valid object?

A) `if (wp) { ... }`  
B) `if (wp.valid()) { ... }`  
C) `if (!wp.expired()) { ... }` or `if (auto sp = wp.lock()) { ... }`  
D) `if (wp.use_count() > 0) { ... }`  

**Answer: C**  
**Explanation:** Use `expired()` to check, or better yet, use `lock()` which atomically checks and creates a `shared_ptr` if the object still exists.

---

### Question 10
What happens when you move a `std::shared_ptr`?

A) Reference count increases  
B) Reference count decreases  
C) Reference count stays the same, ownership transfers  
D) Object is copied  

**Answer: C**  
**Explanation:** Moving a `shared_ptr` transfers ownership without changing the reference count. The moved-from `shared_ptr` becomes null.

---

### Question 11
Can you convert a `std::unique_ptr` to a `std::shared_ptr`?

A) No, they are incompatible  
B) Yes, by moving the unique_ptr  
C) Yes, by copying the unique_ptr  
D) Only with explicit cast  

**Answer: B**  
**Explanation:** You can move a `unique_ptr` to a `shared_ptr`: `std::shared_ptr<T> sp = std::move(up);`. This is a one-way conversion.

---

### Question 12
What is stored in the control block of a `shared_ptr`?

A) Only the reference count  
B) Only the object  
C) Reference count, weak count, deleter, allocator, and other metadata  
D) Only the pointer to the object  

**Answer: C**  
**Explanation:** The control block contains reference count, weak count, custom deleter (if any), custom allocator (if any), and other bookkeeping data.

---

### Question 13
Is reference counting in `shared_ptr` thread-safe?

A) No, you must use mutex  
B) Yes, reference count operations are atomic  
C) Only on some platforms  
D) Only if you use make_shared  

**Answer: B**  
**Explanation:** Reference count operations (increment/decrement) are atomic and thread-safe. However, the object itself is not automatically thread-safe.

---

### Question 14
When should you NOT use `std::make_shared`?

A) Never - always use make_shared  
B) When you need a custom deleter or adopting existing pointer  
C) When performance matters  
D) When using with unique_ptr  

**Answer: B**  
**Explanation:** You cannot use `make_shared` with custom deleters or when adopting an existing raw pointer. Also, with very large objects and weak_ptr, the memory won't be freed until weak_ptrs are gone.

---

### Question 15
What is the aliasing constructor used for?

A) Creating multiple shared_ptrs to the same object  
B) Pointing to a sub-object while sharing ownership of the whole object  
C) Converting between different shared_ptr types  
D) Improving performance  

**Answer: B**  
**Explanation:** The aliasing constructor allows a `shared_ptr` to point to one object (e.g., a member) while sharing ownership of a different object (e.g., the containing object).

---

## 💻 Practical Exercises

### Exercise 1: Code Review - Find the Bugs and Design Issues 🐛

**Difficulty:** Medium  
**Objective:** Identify memory leaks, undefined behavior, and design problems in this shared_ptr code.

```cpp
#include <memory>
#include <iostream>
#include <vector>
#include <thread>

// Bug-ridden code for review

class Node {
public:
    int value;
    std::shared_ptr<Node> parent;    // Issue 1: Circular reference!
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    
    Node(int v) : value(v) {
        std::cout << "Node " << value << " created\n";
    }
    
    ~Node() {
        std::cout << "Node " << value << " destroyed\n";
    }
    
    // Issue 2: Returning shared_ptr to this incorrectly
    std::shared_ptr<Node> getShared() {
        return std::shared_ptr<Node>(this);
    }
};

class ResourceManager {
private:
    std::vector<std::shared_ptr<Resource>> resources_;
    
public:
    void addResource(Resource* res) {  // Issue 3: Taking raw pointer
        resources_.push_back(std::shared_ptr<Resource>(res));
    }
    
    std::shared_ptr<Resource> getResource(int index) {
        return resources_[index];  // Issue 4: No bounds checking
    }
};

// Issue 5: Not thread-safe shared state
class Counter {
private:
    int count_ = 0;
    
public:
    void increment() {
        ++count_;  // Not thread-safe!
    }
    
    int get() const { return count_; }
};

void threadFunc(std::shared_ptr<Counter> counter) {
    for (int i = 0; i < 1000; ++i) {
        counter->increment();
    }
}

// Issue 6: Exception safety problem
void processWidget(std::shared_ptr<Widget> widget, int value) {
    widget->process(value);
}

int computeExpensive() {
    throw std::runtime_error("Error");
}

// Issue 7: Inefficient passing
void drawShape(std::shared_ptr<Shape> shape) {  // Passed by value!
    for (int i = 0; i < 10000; ++i) {
        auto temp = shape;  // Unnecessary copies and atomic ops
        temp->draw();
    }
}

// Issue 8: Creating shared_ptr in constructor
class Widget {
private:
    std::shared_ptr<Widget> self_;
    
public:
    Widget() {
        self_ = std::shared_ptr<Widget>(this);  // WRONG!
    }
};

// Issue 9: Memory leak with make_shared and large objects
class HugeData {
    char data[1024 * 1024 * 100];  // 100MB
};

void leakyFunction() {
    auto sp = std::make_shared<HugeData>();
    std::weak_ptr<HugeData> wp = sp;
    
    sp.reset();  // Object deleted, but control block remains!
    // Memory not fully freed while wp exists
}

// Issue 10: Storing shared_ptr when weak_ptr is better
class Subject {
private:
    std::vector<std::shared_ptr<Observer>> observers_;  // Should be weak_ptr!
    
public:
    void attach(std::shared_ptr<Observer> obs) {
        observers_.push_back(obs);
    }
    
    void notify() {
        for (auto& obs : observers_) {
            obs->update();
        }
    }
};

int main() {
    // Test circular reference
    {
        auto parent = std::make_shared<Node>(1);
        auto child = std::make_shared<Node>(2);
        
        parent->left = child;
        child->parent = parent;  // Circular reference!
    }
    std::cout << "Nodes should be destroyed here...\n";
    // But they won't be! Memory leak!
    
    // Test incorrect shared_ptr from this
    auto node = std::make_shared<Node>(3);
    auto node2 = node->getShared();  // Creates second control block!
    // Both will try to delete the same object!
    
    // Test exception safety
    try {
        processWidget(std::shared_ptr<Widget>(new Widget()), 
                     computeExpensive());
        // If computeExpensive() throws, Widget leaks!
    } catch (...) {}
    
    // Test thread safety
    auto counter = std::make_shared<Counter>();
    std::thread t1(threadFunc, counter);
    std::thread t2(threadFunc, counter);
    t1.join();
    t2.join();
    std::cout << "Counter: " << counter->get() << "\n";  // Not 2000!
    
    return 0;
}
```

**Your Task:**
1. **Identify all 10+ bugs/issues** in the code above
2. **Explain why each is a problem** (memory leak, undefined behavior, performance, etc.)
3. **Provide corrected code** for each issue
4. **Write test code** to verify the fixes work correctly

**Specific Issues to Address:**
- Circular reference with parent pointer
- Incorrect `getShared()` implementation
- Raw pointer to `shared_ptr` conversion
- Exception safety in function calls
- Thread safety issues
- Inefficient parameter passing
- Control block memory retention
- Observer pattern memory leaks

---

### Exercise 2: Debugging - Memory Leak Detective 🔍

**Difficulty:** Advanced  
**Objective:** Find and fix memory leaks in a complex application using `shared_ptr` and `weak_ptr`.

```cpp
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <functional>

// Complex system with subtle memory leaks

class EventListener {
public:
    virtual ~EventListener() {
        std::cout << "EventListener destroyed\n";
    }
    virtual void onEvent(const std::string& event) = 0;
};

class EventSystem {
private:
    // Leak 1: Should use weak_ptr!
    std::map<std::string, std::vector<std::shared_ptr<EventListener>>> listeners_;
    
public:
    void subscribe(const std::string& eventType, 
                   std::shared_ptr<EventListener> listener) {
        listeners_[eventType].push_back(listener);
    }
    
    void unsubscribe(const std::string& eventType,
                    std::shared_ptr<EventListener> listener) {
        auto& vec = listeners_[eventType];
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&listener](const std::shared_ptr<EventListener>& l) {
                return l.get() == listener.get();
            }), vec.end());
    }
    
    void emit(const std::string& eventType) {
        for (auto& listener : listeners_[eventType]) {
            listener->onEvent(eventType);
        }
    }
};

class Component : public EventListener {
private:
    std::string name_;
    std::shared_ptr<EventSystem> eventSystem_;  // Leak 2: Creates cycle!
    std::vector<std::shared_ptr<Component>> children_;
    
public:
    Component(std::string name, std::shared_ptr<EventSystem> es) 
        : name_(std::move(name)), eventSystem_(es) {
        std::cout << "Component '" << name_ << "' created\n";
        eventSystem_->subscribe("update", shared_from_this());  // Leak 3!
    }
    
    ~Component() {
        std::cout << "Component '" << name_ << "' destroyed\n";
    }
    
    void onEvent(const std::string& event) override {
        std::cout << name_ << " received: " << event << "\n";
    }
    
    void addChild(std::shared_ptr<Component> child) {
        children_.push_back(child);
    }
};

// Leak 4: Lambda captures creating cycles
class TaskScheduler {
private:
    std::vector<std::function<void()>> tasks_;
    
public:
    void scheduleTask(std::shared_ptr<Component> comp) {
        // Lambda captures shared_ptr, creates cycle
        tasks_.push_back([comp]() {
            comp->onEvent("scheduled");
        });
    }
    
    void executeTasks() {
        for (auto& task : tasks_) {
            task();
        }
    }
};

// Leak 5: Static shared_ptr
class Singleton {
private:
    static std::shared_ptr<Singleton> instance_;  // Never deleted!
    
    Singleton() {
        std::cout << "Singleton created\n";
    }
    
public:
    static std::shared_ptr<Singleton> getInstance() {
        if (!instance_) {
            instance_ = std::shared_ptr<Singleton>(new Singleton());
        }
        return instance_;
    }
    
    ~Singleton() {
        std::cout << "Singleton destroyed\n";  // Never called!
    }
};

std::shared_ptr<Singleton> Singleton::instance_;

// Leak 6: Self-referencing shared_ptr
class Node {
public:
    int value;
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> self;  // Why would you do this?!
    
    Node(int v) : value(v) {
        std::cout << "Node " << value << " created\n";
    }
    
    ~Node() {
        std::cout << "Node " << value << " destroyed\n";
    }
    
    void initialize(std::shared_ptr<Node> ptr) {
        self = ptr;  // Creates extra reference
    }
};

// Leak 7: Not cleaning up expired weak_ptrs
class Cache {
private:
    std::map<int, std::weak_ptr<Resource>> cache_;
    int nextId_ = 0;
    
public:
    int add(std::shared_ptr<Resource> res) {
        cache_[nextId_] = res;
        return nextId_++;
    }
    
    std::shared_ptr<Resource> get(int id) {
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            return it->second.lock();
        }
        return nullptr;
    }
    
    // Missing: cleanup of expired entries!
    // Map keeps growing even for deleted resources
};

int main() {
    // Leak 1 & 2: Event system + Component cycle
    {
        auto eventSys = std::make_shared<EventSystem>();
        auto comp = std::make_shared<Component>("Main", eventSys);
        
        eventSys->emit("update");
        // Component subscribes to EventSystem
        // EventSystem holds shared_ptr to Component
        // Component holds shared_ptr to EventSystem
        // Circular reference! Neither destroyed!
    }
    std::cout << "Should be destroyed here...\n\n";
    
    // Leak 3: enable_shared_from_this used incorrectly
    // (Component uses shared_from_this in constructor)
    
    // Leak 4: Lambda captures
    {
        auto scheduler = std::make_shared<TaskScheduler>();
        auto comp = std::make_shared<Component>("Task", 
                    std::make_shared<EventSystem>());
        
        scheduler->scheduleTask(comp);
        // Lambda captures shared_ptr to comp
        // scheduler holds lambda
        // If comp held scheduler, circular reference!
    }
    
    // Leak 5: Singleton
    auto singleton = Singleton::getInstance();
    // singleton never destroyed
    
    // Leak 6: Self reference
    {
        auto node = std::make_shared<Node>(42);
        node->initialize(node);  // Extra reference to self!
    }
    std::cout << "Node should be destroyed here...\n\n";
    
    // Leak 7: Cache never cleans up
    Cache cache;
    for (int i = 0; i < 1000; ++i) {
        auto res = std::make_shared<Resource>();
        cache.add(res);
        // res goes out of scope, but weak_ptr entry remains
    }
    // Cache map has 1000 expired entries!
    
    std::cout << "End of main\n";
    return 0;
}
```

**Your Task:**
1. **Run the code with memory leak detection** (Valgrind, AddressSanitizer)
   ```bash
   g++ -std=c++17 -fsanitize=address -g program.cpp -o program
   ./program
   ```

2. **Identify all memory leaks** and explain the root cause

3. **Fix each leak** by:
   - Using `weak_ptr` where appropriate
   - Breaking circular references
   - Cleaning up expired entries
   - Fixing `enable_shared_from_this` usage
   - Removing unnecessary self-references

4. **Verify no leaks remain** with tools

5. **Add smart cleanup mechanisms**:
   - Auto-cleanup for expired weak_ptrs
   - Proper subscriber management
   - Correct lambda captures

**Expected Fixes:**
```cpp
// Fix 1: Use weak_ptr for listeners
std::map<std::string, std::vector<std::weak_ptr<EventListener>>> listeners_;

// Fix 2: Use weak_ptr for EventSystem reference
std::weak_ptr<EventSystem> eventSystem_;

// Fix 3: Call shared_from_this AFTER construction
// Move subscription to separate init() method

// Fix 4: Capture weak_ptr in lambdas
tasks_.push_back([weak = std::weak_ptr<Component>(comp)]() {
    if (auto comp = weak.lock()) {
        comp->onEvent("scheduled");
    }
});

// Fix 5: Use static instance, not static shared_ptr
// Or implement proper cleanup

// Fix 6: Remove self reference entirely

// Fix 7: Implement periodic cleanup
void Cache::cleanup() {
    for (auto it = cache_.begin(); it != cache_.end();) {
        if (it->second.expired()) {
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }
}
```

---

### Exercise 3: Performance Optimization - Minimize Overhead ⚡

**Difficulty:** Advanced  
**Objective:** Optimize this code that uses `shared_ptr` inefficiently, causing performance problems.

```cpp
#include <memory>
#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>

// Inefficient code - optimize this!

class Point3D {
public:
    double x, y, z;
    
    Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    double distanceFrom(const Point3D& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        double dz = z - other.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

class Mesh {
private:
    std::vector<std::shared_ptr<Point3D>> vertices_;
    
public:
    // Issue 1: Passing shared_ptr by value - expensive copies!
    void addVertex(std::shared_ptr<Point3D> vertex) {
        vertices_.push_back(vertex);
    }
    
    // Issue 2: Returning shared_ptr by value for read-only access
    std::shared_ptr<Point3D> getVertex(size_t index) const {
        return vertices_[index];  // Unnecessary ref count increment
    }
    
    // Issue 3: Creating temporary shared_ptrs in hot loop
    double computeAverageDistance() const {
        double total = 0.0;
        for (size_t i = 0; i < vertices_.size(); ++i) {
            for (size_t j = i + 1; j < vertices_.size(); ++j) {
                // Creates 2 shared_ptr copies per iteration!
                auto v1 = getVertex(i);
                auto v2 = getVertex(j);
                total += v1->distanceFrom(*v2);
            }
        }
        return total / (vertices_.size() * (vertices_.size() - 1) / 2);
    }
    
    // Issue 4: Unnecessary shared_ptr for temporary calculations
    std::shared_ptr<Point3D> computeCentroid() const {
        double sumX = 0, sumY = 0, sumZ = 0;
        
        for (const auto& vertex : vertices_) {
            sumX += vertex->x;
            sumY += vertex->y;
            sumZ += vertex->z;
        }
        
        // Creating shared_ptr for return value that caller immediately uses
        return std::make_shared<Point3D>(
            sumX / vertices_.size(),
            sumY / vertices_.size(),
            sumZ / vertices_.size()
        );
    }
    
    // Issue 5: Copying shared_ptrs in algorithm
    void sortVerticesByX() {
        // Copies all shared_ptrs during sort!
        std::sort(vertices_.begin(), vertices_.end(),
            [](std::shared_ptr<Point3D> a, std::shared_ptr<Point3D> b) {
                return a->x < b->x;
            });
    }
    
    size_t vertexCount() const { return vertices_.size(); }
};

class Scene {
private:
    std::vector<std::shared_ptr<Mesh>> meshes_;
    
public:
    // Issue 6: Storing shared_ptr when ownership not needed
    void addMesh(std::shared_ptr<Mesh> mesh) {
        meshes_.push_back(mesh);
    }
    
    // Issue 7: Passing shared_ptr through deep call chain
    void processAllMeshes(std::shared_ptr<Processor> processor) {
        for (auto& mesh : meshes_) {
            processMesh(mesh, processor);  // Copies at each level
        }
    }
    
private:
    void processMesh(std::shared_ptr<Mesh> mesh, 
                    std::shared_ptr<Processor> processor) {
        analyzeGeometry(mesh, processor);  // More copies!
    }
    
    void analyzeGeometry(std::shared_ptr<Mesh> mesh,
                        std::shared_ptr<Processor> processor) {
        for (size_t i = 0; i < mesh->vertexCount(); ++i) {
            auto vertex = mesh->getVertex(i);  // Copy!
            processor->process(*vertex);
        }
    }
};

// Issue 8: Creating shared_ptrs in tight loops
class ParticleSystem {
private:
    std::vector<std::shared_ptr<Point3D>> particles_;
    
public:
    void update(double dt) {
        // Creating thousands of shared_ptrs per frame!
        for (auto& particle : particles_) {
            auto temp = particle;  // Unnecessary copy
            temp->x += dt;
            temp->y += dt * 0.5;
            temp->z += dt * 0.25;
        }
    }
    
    // Issue 9: Storing copies instead of references
    std::vector<std::shared_ptr<Point3D>> getActiveParticles() const {
        std::vector<std::shared_ptr<Point3D>> active;
        for (const auto& p : particles_) {
            if (p->x > 0) {
                active.push_back(p);  // Copy shared_ptr
            }
        }
        return active;  // Move, but still copied into caller
    }
};

// Issue 10: make_shared not used
void createManyPoints() {
    std::vector<std::shared_ptr<Point3D>> points;
    
    for (int i = 0; i < 100000; ++i) {
        // Two allocations per point!
        points.push_back(std::shared_ptr<Point3D>(
            new Point3D(i, i*2, i*3)
        ));
    }
}

// Performance test
void benchmark() {
    auto start = std::chrono::high_resolution_clock::now();
    
    auto mesh = std::make_shared<Mesh>();
    
    // Add 1000 vertices
    for (int i = 0; i < 1000; ++i) {
        mesh->addVertex(std::make_shared<Point3D>(i, i, i));
    }
    
    // Compute average distance (very slow!)
    double avgDist = mesh->computeAverageDistance();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();
    
    std::cout << "Average distance: " << avgDist << "\n";
    std::cout << "Time: " << duration << "ms\n";
}

int main() {
    benchmark();
    return 0;
}
```

**Your Task:**

1. **Profile the code** to identify bottlenecks:
   ```bash
   g++ -std=c++17 -O2 -pg program.cpp -o program
   ./program
   gprof program gmon.out
   ```

2. **Identify all performance issues**:
   - Unnecessary `shared_ptr` copies
   - Atomic operations in hot loops
   - Inefficient parameter passing
   - Wrong choice of smart pointer

3. **Optimize each issue**:

**Optimization 1: Pass by const reference**
```cpp
// Before
void addVertex(std::shared_ptr<Point3D> vertex)

// After
void addVertex(const std::shared_ptr<Point3D>& vertex)
```

**Optimization 2: Return const reference or raw pointer**
```cpp
// Before
std::shared_ptr<Point3D> getVertex(size_t index) const

// After
const Point3D& getVertex(size_t index) const {
    return *vertices_[index];
}
```

**Optimization 3: Avoid creating shared_ptrs in loops**
```cpp
// Before
auto v1 = getVertex(i);
auto v2 = getVertex(j);
total += v1->distanceFrom(*v2);

// After
total += getVertex(i).distanceFrom(getVertex(j));
```

**Optimization 4: Return by value for small objects**
```cpp
// Before
std::shared_ptr<Point3D> computeCentroid() const

// After
Point3D computeCentroid() const
```

**Optimization 5: Use const reference in lambda**
```cpp
// Before
[](std::shared_ptr<Point3D> a, std::shared_ptr<Point3D> b)

// After
[](const std::shared_ptr<Point3D>& a, const std::shared_ptr<Point3D>& b)
```

**Optimization 6: Consider unique_ptr or direct storage**
```cpp
// If no sharing needed:
std::vector<std::unique_ptr<Point3D>> vertices_;

// Or even better if possible:
std::vector<Point3D> vertices_;  // Direct storage!
```

**Optimization 7: Pass through by reference**
```cpp
void processMesh(const std::shared_ptr<Mesh>& mesh, 
                const std::shared_ptr<Processor>& processor)
```

**Optimization 8: Eliminate unnecessary temps**
```cpp
// Before
for (auto& particle : particles_) {
    auto temp = particle;
    temp->x += dt;
}

// After
for (auto& particle : particles_) {
    particle->x += dt;
}
```

**Optimization 9: Return view or indices**
```cpp
// Return indices instead of copies
std::vector<size_t> getActiveParticleIndices() const
```

**Optimization 10: Use make_shared**
```cpp
points.push_back(std::make_shared<Point3D>(i, i*2, i*3));
```

4. **Measure improvement** after each optimization

5. **Document the performance gains**:
   - Baseline: X ms
   - After optimization 1: Y ms
   - Final: Z ms
   - Total speedup: baseline/final

**Expected Results:**
- Original: ~5000ms for 1000 vertices
- Optimized: ~100ms (50x speedup!)

**Bonus Challenges:**
- Profile with cachegrind to see cache misses
- Compare memory usage before/after
- Add benchmarks with varying data sizes
- Consider using object pools for frequent allocations

---

### Exercise 4: Design Review - Choosing the Right Smart Pointer 🎨

**Difficulty:** Advanced  
**Objective:** Review these design patterns and determine if `shared_ptr` is the right choice or if alternatives would be better.

```cpp
#include <memory>
#include <vector>
#include <map>
#include <string>

// Design 1: GUI Widget Hierarchy
class Widget {
protected:
    std::shared_ptr<Widget> parent_;  // Question: shared_ptr correct here?
    std::vector<std::shared_ptr<Widget>> children_;
    
public:
    void setParent(std::shared_ptr<Widget> parent) {
        parent_ = parent;  // Potential circular reference!
    }
    
    void addChild(std::shared_ptr<Widget> child) {
        children_.push_back(child);
        child->setParent(shared_from_this());
    }
};

// Design 2: Resource Cache
class ResourceCache {
private:
    // Question: Is shared_ptr the right choice for cache?
    std::map<std::string, std::shared_ptr<Resource>> cache_;
    
public:
    std::shared_ptr<Resource> get(const std::string& key) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }
        
        auto resource = std::make_shared<Resource>(key);
        cache_[key] = resource;
        return resource;
    }
    
    // Problem: Resources never removed from cache!
};

// Design 3: Factory Pattern
class ObjectFactory {
public:
    // Question: Should factory return shared_ptr or unique_ptr?
    static std::shared_ptr<Object> create(const std::string& type) {
        if (type == "TypeA") {
            return std::make_shared<TypeA>();
        }
        return std::make_shared<TypeB>();
    }
};

// Design 4: Singleton
class ConfigManager {
private:
    static std::shared_ptr<ConfigManager> instance_;
    
public:
    // Question: shared_ptr or something else?
    static std::shared_ptr<ConfigManager> getInstance() {
        if (!instance_) {
            instance_ = std::make_shared<ConfigManager>();
        }
        return instance_;
    }
};

// Design 5: Callback System
class Button {
private:
    // Question: How to store callbacks without memory leaks?
    std::vector<std::function<void()>> callbacks_;
    
public:
    void onClick(std::function<void()> callback) {
        callbacks_.push_back(callback);
    }
    
    void click() {
        for (auto& cb : callbacks_) {
            cb();
        }
    }
};

class Controller {
private:
    std::shared_ptr<Button> button_;
    
public:
    Controller(std::shared_ptr<Button> btn) : button_(btn) {
        // Captures 'this' - danger if Controller destroyed!
        button_->onClick([this]() {
            this->handleClick();
        });
    }
    
    void handleClick() {
        // ...
    }
};

// Design 6: Reference Counted Base Class
class RefCounted {
protected:
    std::atomic<int> refCount_{0};
    
public:
    void addRef() { ++refCount_; }
    void release() {
        if (--refCount_ == 0) {
            delete this;
        }
    }
};

// Question: Should we use shared_ptr instead of manual ref counting?
```

**Your Task:**

For each design pattern above:

1. **Analyze the current design**
   - Is `shared_ptr` the right choice?
   - What problems exist?
   - Are there circular references?
   - Is there ownership confusion?

2. **Propose improvements**
   - Which smart pointer should be used? (`unique_ptr`, `shared_ptr`, `weak_ptr`, or raw pointer/reference)
   - How to break circular references?
   - How to improve performance?
   - How to improve clarity?

3. **Rewrite with correct smart pointer usage**

4. **Explain your reasoning**

**Example Analysis:**

**Design 1 - Widget Hierarchy:**

**Problems:**
- Parent using `shared_ptr` creates circular reference
- Parent owns children, children reference parent = cycle
- Memory leak: widgets never destroyed

**Solution:**
```cpp
class Widget {
protected:
    Widget* parent_;  // Raw pointer (non-owning) or weak_ptr
    std::vector<std::unique_ptr<Widget>> children_;  // Exclusive ownership
    
public:
    void setParent(Widget* parent) {
        parent_ = parent;
    }
    
    void addChild(std::unique_ptr<Widget> child) {
        child->setParent(this);
        children_.push_back(std::move(child));
    }
};
```

**Reasoning:**
- Parent owns children exclusively → `unique_ptr`
- Children don't own parent → raw pointer or `weak_ptr`
- No circular references
- Clear ownership model

Now complete the analysis for Designs 2-6!

---

### Exercise 5: Implementation Challenge - Undo/Redo System 🔨

**Difficulty:** Advanced  
**Objective:** Implement a complete undo/redo system using `shared_ptr` and `weak_ptr` correctly.

```cpp
#include <memory>
#include <stack>
#include <vector>
#include <string>

// Your task: Complete this implementation

class Document : public std::enable_shared_from_this<Document> {
private:
    std::string content_;
    int version_;
    
public:
    Document() : version_(0) {}
    
    void setContent(const std::string& content) {
        content_ = content;
        ++version_;
    }
    
    std::string getContent() const { return content_; }
    int getVersion() const { return version_; }
};

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual bool canExecute() const = 0;  // Check if document still exists
};

class InsertTextCommand : public Command {
private:
    std::weak_ptr<Document> document_;  // Don't own document
    std::string text_;
    size_t position_;
    std::string previousContent_;
    
public:
    InsertTextCommand(std::shared_ptr<Document> doc, 
                     const std::string& text,
                     size_t pos)
        : document_(doc), text_(text), position_(pos) {}
    
    void execute() override {
        // TODO: Implement
        // 1. Lock weak_ptr to get shared_ptr
        // 2. Save previous content
        // 3. Insert text
        // 4. Handle if document was destroyed
    }
    
    void undo() override {
        // TODO: Implement
        // 1. Lock weak_ptr
        // 2. Restore previous content
        // 3. Handle if document was destroyed
    }
    
    bool canExecute() const override {
        // TODO: Check if document still exists
        return false;
    }
};

class DeleteTextCommand : public Command {
    // TODO: Implement similar to InsertTextCommand
};

class CommandManager {
private:
    std::stack<std::unique_ptr<Command>> undoStack_;
    std::stack<std::unique_ptr<Command>> redoStack_;
    size_t maxHistorySize_;
    
public:
    explicit CommandManager(size_t maxSize = 100) 
        : maxHistorySize_(maxSize) {}
    
    void executeCommand(std::unique_ptr<Command> cmd) {
        // TODO: Implement
        // 1. Check if command can execute
        // 2. Execute command
        // 3. Push to undo stack
        // 4. Clear redo stack
        // 5. Maintain max history size
    }
    
    bool undo() {
        // TODO: Implement
        // 1. Check if undo stack not empty
        // 2. Pop from undo stack
        // 3. Undo the command
        // 4. Push to redo stack
        // 5. Return success/failure
        return false;
    }
    
    bool redo() {
        // TODO: Implement
        return false;
    }
    
    void clearHistory() {
        // TODO: Clear both stacks
    }
    
    size_t undoCount() const {
        return undoStack_.size();
    }
    
    size_t redoCount() const {
        return redoStack_.size();
    }
};

// Test your implementation
int main() {
    auto doc = std::make_shared<Document>();
    CommandManager cmdManager;
    
    // Execute commands
    cmdManager.executeCommand(
        std::make_unique<InsertTextCommand>(doc, "Hello", 0)
    );
    
    cmdManager.executeCommand(
        std::make_unique<InsertTextCommand>(doc, " World", 5)
    );
    
    std::cout << "Content: " << doc->getContent() << "\n";
    std::cout << "Undo count: " << cmdManager.undoCount() << "\n";
    
    // Undo
    cmdManager.undo();
    std::cout << "After undo: " << doc->getContent() << "\n";
    
    // Redo
    cmdManager.redo();
    std::cout << "After redo: " << doc->getContent() << "\n";
    
    // Test with document destruction
    {
        auto tempDoc = std::make_shared<Document>();
        cmdManager.executeCommand(
            std::make_unique<InsertTextCommand>(tempDoc, "Temp", 0)
        );
    }  // tempDoc destroyed
    
    // Undo should handle gracefully
    bool success = cmdManager.undo();
    std::cout << "Undo after doc destroyed: " << (success ? "Success" : "Failed") << "\n";
    
    return 0;
}
```

**Requirements:**

1. **Complete all TODO sections**
2. **Use `weak_ptr` for document references in commands**
3. **Handle document destruction gracefully**
4. **Implement history size limit**
5. **Add more command types**: DeleteTextCommand, ReplaceTextCommand
6. **Add command merging**: Merge consecutive insert commands
7. **Add command groups**: Execute multiple commands as one
8. **Thread safety**: Make CommandManager thread-safe
9. **Add tests**: Comprehensive test suite
10. **Memory leak check**: Verify no leaks with AddressSanitizer

**Bonus Features:**
- Command description for undo menu
- Checkpoint system (save/restore states)
- Command compression for large history
- Async command execution
- Command validation before execution

---

## 🎯 Solutions Checklist

For each exercise, ensure your solution includes:

- [ ] **Compiles without warnings** (`-Wall -Wextra -Werror`)
- [ ] **No memory leaks** (verified with Valgrind/AddressSanitizer)
- [ ] **Proper smart pointer usage** (right pointer for right job)
- [ ] **Thread safety** where required
- [ ] **Exception safety** considered
- [ ] **Performance optimized** (no unnecessary copies)
- [ ] **Well documented** (comments explaining ownership)
- [ ] **Comprehensive tests** covering edge cases
- [ ] **Handles object destruction** gracefully

## 📊 Answer Key for MCQs

1. **B** - Reference counting
2. **C** - Two pointers (16 bytes)
3. **B** - Atomically incremented
4. **B** - Single allocation, exception-safe
5. **B** - Deleter in type vs. control block
6. **B** - Break circular references
7. **B** - Safely get shared_ptr from this
8. **B** - Double delete (UB)
9. **C** - expired() or lock()
10. **C** - Ref count unchanged, ownership transfers
11. **B** - Move unique_ptr to shared_ptr
12. **C** - All metadata in control block
13. **B** - Atomic operations thread-safe
14. **B** - Custom deleter or existing pointer
15. **B** - Point to sub-object, own whole

## 🔧 Testing Commands

**Memory leak detection:**
```bash
# AddressSanitizer
g++ -std=c++17 -fsanitize=address -fsanitize=undefined -g program.cpp -o program
./program

# Valgrind
g++ -std=c++17 -g program.cpp -o program
valgrind --leak-check=full --show-leak-kinds=all ./program
```

**Performance profiling:**
```bash
# gprof
g++ -std=c++17 -O2 -pg program.cpp -o program
./program
gprof program gmon.out > analysis.txt

# perf (Linux)
g++ -std=c++17 -O2 -g program.cpp -o program
perf record ./program
perf report
```

**Thread sanitizer:**
```bash
g++ -std=c++17 -fsanitize=thread -g program.cpp -o program -pthread
./program
```

## 📚 Additional Resources

- **CppReference**: https://en.cppreference.com/w/cpp/memory/shared_ptr
- **Boost Smart Pointers**: For additional smart pointer types
- **C++ Core Guidelines**: F.7, R.20-R.37 for smart pointer guidelines
- **Herb Sutter's Blog**: GotW series on smart pointers

