# Item 19 (Part 2): Use std::shared_ptr - Advanced Topics

## 🚀 std::make_shared - The Preferred Way

### Why Use make_shared?

```cpp
// ❌ NOT PREFERRED: Two allocations
std::shared_ptr<Widget> sp1(new Widget());
// Allocation 1: Widget object
// Allocation 2: Control block

// ✅ PREFERRED: Single allocation
auto sp2 = std::make_shared<Widget>();
// Allocation 1: Widget + Control block together!
```

### Performance Comparison

```
Traditional construction (new + shared_ptr):
┌──────────┐        ┌──────────────┐
│  Widget  │        │ Control      │
│  Object  │        │ Block        │
└──────────┘        └──────────────┘
   Heap 1              Heap 2

make_shared construction:
┌────────────────────────────┐
│ Control Block │   Widget   │
│               │   Object   │
└────────────────────────────┘
      Single Heap Allocation
```

### Complete Example

```cpp
#include <memory>
#include <iostream>
#include <chrono>

class Data {
private:
    int values_[1000];
public:
    Data() { std::cout << "Data created\n"; }
    ~Data() { std::cout << "Data destroyed\n"; }
};

void testPerformance() {
    using namespace std::chrono;
    
    // Test make_shared
    auto start1 = high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        auto sp = std::make_shared<Data>();
    }
    auto end1 = high_resolution_clock::now();
    auto time1 = duration_cast<milliseconds>(end1 - start1).count();
    
    // Test new + shared_ptr
    auto start2 = high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        std::shared_ptr<Data> sp(new Data());
    }
    auto end2 = high_resolution_clock::now();
    auto time2 = duration_cast<milliseconds>(end2 - start2).count();
    
    std::cout << "make_shared: " << time1 << "ms\n";
    std::cout << "new + shared_ptr: " << time2 << "ms\n";
}
```

### Benefits of make_shared

1. **Performance**: Single allocation (faster, better cache locality)
2. **Exception Safety**: Prevents potential leaks
3. **Conciseness**: Less typing, no type repetition
4. **Consistency**: Matches `make_unique` style

### Exception Safety Example

```cpp
void processWidget(std::shared_ptr<Widget> sp, int priority);

// ❌ POTENTIAL LEAK (unlikely but possible)
processWidget(std::shared_ptr<Widget>(new Widget()), computePriority());
// If computePriority() throws after new but before shared_ptr construction
// the Widget leaks!

// ✅ EXCEPTION SAFE
processWidget(std::make_shared<Widget>(), computePriority());
// make_shared is atomic - either succeeds completely or fails cleanly
```

### When NOT to Use make_shared

```cpp
// 1. Custom deleters
auto deleter = [](Widget* p) { 
    cleanup(p); 
    delete p; 
};
std::shared_ptr<Widget> sp(new Widget(), deleter);  // Need this form

// 2. Adopting existing raw pointer
Widget* existing = getWidget();
std::shared_ptr<Widget> sp(existing);  // Need this form

// 3. Very large objects with weak_ptr
// Control block won't be freed until all weak_ptrs are gone
// May want separate allocation if object is huge
```

## 🎭 Custom Deleters with shared_ptr

Unlike `unique_ptr`, the deleter type is NOT part of `shared_ptr`'s type!

### Basic Custom Deleter

```cpp
#include <memory>
#include <iostream>

class FileHandle {
private:
    FILE* file_;
    std::string name_;
    
public:
    FileHandle(const char* filename) : name_(filename) {
        file_ = fopen(filename, "w");
        if (file_) {
            std::cout << "File opened: " << name_ << "\n";
        }
    }
    
    void write(const std::string& data) {
        if (file_) {
            fputs(data.c_str(), file_);
        }
    }
    
    ~FileHandle() {
        std::cout << "FileHandle destroyed (but file NOT closed yet)\n";
    }
    
    FILE* get() { return file_; }
    const std::string& name() const { return name_; }
};

// Custom deleter function
void fileCloser(FileHandle* fh) {
    if (fh && fh->get()) {
        fclose(fh->get());
        std::cout << "File closed: " << fh->name() << "\n";
    }
    delete fh;
}

int main() {
    // shared_ptr with custom deleter
    std::shared_ptr<FileHandle> file(
        new FileHandle("data.txt"),
        fileCloser  // Custom deleter
    );
    
    file->write("Hello, World!\n");
    
    // Can copy - deleter is stored in control block
    auto file2 = file;
    std::cout << "Reference count: " << file.use_count() << "\n";
    
    // Both file and file2 share the same deleter
    return 0;  // fileCloser called when last shared_ptr destroyed
}
```

### Lambda Deleters

```cpp
auto logDeleter = [](Widget* w) {
    std::cout << "Deleting widget at: " << w << "\n";
    delete w;
};

std::shared_ptr<Widget> sp1(new Widget(), logDeleter);
std::shared_ptr<Widget> sp2(new Widget(), logDeleter);

// sp1 and sp2 have THE SAME TYPE!
// This is different from unique_ptr where deleter is part of type
sp1 = sp2;  // ✅ OK! Types are compatible
```

### Comparison: shared_ptr vs unique_ptr Deleters

```cpp
// unique_ptr - deleter is part of TYPE
auto del = [](Widget* p) { delete p; };
std::unique_ptr<Widget, decltype(del)> up1(new Widget(), del);
std::unique_ptr<Widget, decltype(del)> up2(new Widget(), del);
// Type: unique_ptr<Widget, lambda_type>

// shared_ptr - deleter stored in control block
std::shared_ptr<Widget> sp1(new Widget(), del);
std::shared_ptr<Widget> sp2(new Widget(), del);
// Type: just shared_ptr<Widget>

// Result:
up1 = std::move(up2);  // ✅ OK - same type
sp1 = sp2;             // ✅ OK - same type (always!)
```

## 🔗 std::enable_shared_from_this - Getting shared_ptr from `this`

### The Problem

```cpp
class Widget {
public:
    std::shared_ptr<Widget> getShared() {
        // ❌ WRONG! Creates separate control block
        return std::shared_ptr<Widget>(this);
    }
};

auto sp1 = std::make_shared<Widget>();
auto sp2 = sp1->getShared();  
// DISASTER! sp1 and sp2 have different control blocks!
// Both will try to delete the Widget -> double delete!
```

### The Solution: enable_shared_from_this

```cpp
#include <memory>
#include <iostream>

class Widget : public std::enable_shared_from_this<Widget> {
private:
    int id_;
    
public:
    Widget(int id) : id_(id) {
        std::cout << "Widget " << id_ << " created\n";
    }
    
    ~Widget() {
        std::cout << "Widget " << id_ << " destroyed\n";
    }
    
    // ✅ CORRECT: Use shared_from_this()
    std::shared_ptr<Widget> getShared() {
        return shared_from_this();
    }
    
    void doAsyncWork() {
        // Capture shared_ptr to keep object alive during async operation
        auto self = shared_from_this();
        
        // Simulate async work
        async_operation([self]() {
            std::cout << "Async work using Widget " << self->id_ << "\n";
        });
    }
};

int main() {
    auto sp1 = std::make_shared<Widget>(1);
    std::cout << "sp1 count: " << sp1.use_count() << "\n";  // 1
    
    auto sp2 = sp1->getShared();  // ✅ Shares same control block
    std::cout << "sp1 count: " << sp1.use_count() << "\n";  // 2
    std::cout << "sp2 count: " << sp2.use_count() << "\n";  // 2
    
    return 0;
}
```

### Important Rules for enable_shared_from_this

```cpp
class MyClass : public std::enable_shared_from_this<MyClass> {
public:
    void test() {
        // ⚠️ IMPORTANT: Only call shared_from_this() when
        // a shared_ptr to this object already exists!
        
        try {
            auto sp = shared_from_this();
        } catch (const std::bad_weak_ptr& e) {
            std::cout << "Error: No shared_ptr exists yet!\n";
        }
    }
};

// ❌ WRONG: Calling before shared_ptr exists
MyClass obj;
obj.test();  // Throws std::bad_weak_ptr!

// ✅ CORRECT: Call after shared_ptr created
auto sp = std::make_shared<MyClass>();
sp->test();  // Works fine!
```

### Real-World Example: Async Operations

```cpp
#include <memory>
#include <iostream>
#include <vector>
#include <functional>

class DataProcessor : public std::enable_shared_from_this<DataProcessor> {
private:
    std::string name_;
    std::vector<int> data_;
    
public:
    DataProcessor(std::string name) : name_(std::move(name)) {
        std::cout << "DataProcessor '" << name_ << "' created\n";
    }
    
    ~DataProcessor() {
        std::cout << "DataProcessor '" << name_ << "' destroyed\n";
    }
    
    void startProcessing() {
        // Schedule multiple async operations
        for (int i = 0; i < 3; ++i) {
            scheduleWork(i);
        }
    }
    
private:
    void scheduleWork(int taskId) {
        // Capture shared_ptr to keep object alive
        auto self = shared_from_this();
        
        // Simulate async work (in real code, this would be actual async)
        std::cout << "Scheduling task " << taskId 
                  << " for " << name_ << "\n";
        
        // Lambda keeps self alive
        auto callback = [self, taskId]() {
            std::cout << "Executing task " << taskId 
                      << " on " << self->name_ << "\n";
        };
        
        // In real code: submit callback to thread pool, event loop, etc.
        callback();  // Execute immediately for demonstration
    }
};

int main() {
    std::cout << "Creating processor...\n";
    {
        auto processor = std::make_shared<DataProcessor>("MyProcessor");
        processor->startProcessing();
        // processor goes out of scope here, but object may still be alive
        // if async operations are holding references
    }
    std::cout << "Processor variable destroyed\n";
    
    return 0;
}
```

## 🔓 std::weak_ptr - Breaking Circular References

### The Circular Reference Problem

```cpp
#include <memory>
#include <iostream>

class B;  // Forward declaration

class A {
public:
    std::shared_ptr<B> b_ptr;
    ~A() { std::cout << "A destroyed\n"; }
};

class B {
public:
    std::shared_ptr<A> a_ptr;  // ❌ PROBLEM!
    ~B() { std::cout << "B destroyed\n"; }
};

int main() {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();
    
    a->b_ptr = b;  // A holds shared_ptr to B
    b->a_ptr = a;  // B holds shared_ptr to A
    
    // Circular reference! Neither destructor will be called!
    // Memory leak!
    return 0;
}
// NO OUTPUT - memory leaked!
```

### Solution: Use weak_ptr

```cpp
class A {
public:
    std::shared_ptr<B> b_ptr;
    ~A() { std::cout << "A destroyed\n"; }
};

class B {
public:
    std::weak_ptr<A> a_ptr;  // ✅ Use weak_ptr to break cycle!
    ~B() { std::cout << "B destroyed\n"; }
    
    void accessA() {
        // Convert weak_ptr to shared_ptr to use it
        if (auto a_shared = a_ptr.lock()) {
            std::cout << "A is still alive!\n";
        } else {
            std::cout << "A has been destroyed\n";
        }
    }
};

int main() {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();
    
    a->b_ptr = b;
    b->a_ptr = a;  // weak_ptr doesn't increase ref count
    
    std::cout << "a ref count: " << a.use_count() << "\n";  // 1 (not 2!)
    
    return 0;
}
// OUTPUT:
// a ref count: 1
// A destroyed
// B destroyed
```

### weak_ptr Operations

```cpp
auto sp = std::make_shared<Widget>();
std::weak_ptr<Widget> wp = sp;  // Create weak_ptr from shared_ptr

std::cout << "sp count: " << sp.use_count() << "\n";  // 1
std::cout << "wp count: " << wp.use_count() << "\n";  // 1 (same)
std::cout << "wp expired: " << wp.expired() << "\n";  // false

// To use weak_ptr, convert to shared_ptr
if (auto sp2 = wp.lock()) {
    std::cout << "Object still alive\n";
    sp2->doSomething();
} else {
    std::cout << "Object destroyed\n";
}

// After all shared_ptrs are gone
sp.reset();

std::cout << "wp expired: " << wp.expired() << "\n";  // true
if (auto sp2 = wp.lock()) {
    // Won't execute
} else {
    std::cout << "Object destroyed\n";  // This executes
}
```

### Real-World Example: Observer Pattern

```cpp
#include <memory>
#include <vector>
#include <iostream>

class Subject;

class Observer {
public:
    virtual ~Observer() {
        std::cout << "Observer destroyed\n";
    }
    virtual void update(int value) = 0;
};

class ConcreteObserver : public Observer {
private:
    int id_;
    
public:
    ConcreteObserver(int id) : id_(id) {
        std::cout << "Observer " << id_ << " created\n";
    }
    
    ~ConcreteObserver() {
        std::cout << "Observer " << id_ << " destroyed\n";
    }
    
    void update(int value) override {
        std::cout << "Observer " << id_ << " received: " << value << "\n";
    }
};

class Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;  // weak_ptr!
    int value_;
    
public:
    void attach(std::shared_ptr<Observer> obs) {
        observers_.push_back(obs);  // Store as weak_ptr
    }
    
    void setValue(int val) {
        value_ = val;
        notify();
    }
    
private:
    void notify() {
        // Remove expired observers while notifying
        auto it = observers_.begin();
        while (it != observers_.end()) {
            if (auto obs = it->lock()) {
                obs->update(value_);
                ++it;
            } else {
                // Observer destroyed, remove from list
                it = observers_.erase(it);
            }
        }
    }
};

int main() {
    Subject subject;
    
    {
        auto obs1 = std::make_shared<ConcreteObserver>(1);
        auto obs2 = std::make_shared<ConcreteObserver>(2);
        
        subject.attach(obs1);
        subject.attach(obs2);
        
        subject.setValue(42);  // Both observers notified
        
        // obs2 goes out of scope
    }
    
    std::cout << "\nAfter observer 2 destroyed:\n";
    subject.setValue(99);  // Only observer 1 notified
    
    return 0;
}
```

## 🏷️ Aliasing Constructor

The aliasing constructor allows a `shared_ptr` to point to one object while owning another.

```cpp
#include <memory>
#include <iostream>

struct Data {
    int value;
    double score;
    
    Data() : value(42), score(3.14) {
        std::cout << "Data created\n";
    }
    ~Data() {
        std::cout << "Data destroyed\n";
    }
};

int main() {
    auto data_ptr = std::make_shared<Data>();
    
    // Aliasing: points to 'value' but shares ownership of entire Data
    std::shared_ptr<int> value_ptr(data_ptr, &data_ptr->value);
    
    std::cout << "data_ptr count: " << data_ptr.use_count() << "\n";   // 2
    std::cout << "value_ptr count: " << value_ptr.use_count() << "\n"; // 2
    
    std::cout << "Value: " << *value_ptr << "\n";  // 42
    
    data_ptr.reset();  // Release our reference
    
    std::cout << "\nAfter resetting data_ptr:\n";
    std::cout << "value_ptr count: " << value_ptr.use_count() << "\n"; // 1
    std::cout << "Value still accessible: " << *value_ptr << "\n";     // 42
    
    // Data destroyed when value_ptr goes out of scope
    return 0;
}
```

### Practical Use: Returning Member Pointers

```cpp
class Container {
private:
    std::string name_;
    std::vector<int> data_;
    
public:
    Container(std::string name) : name_(std::move(name)), data_{1, 2, 3} {
        std::cout << "Container created\n";
    }
    
    ~Container() {
        std::cout << "Container destroyed\n";
    }
    
    // Return shared_ptr to member, but keep container alive
    std::shared_ptr<const std::vector<int>> getData(
        std::shared_ptr<Container> self) {
        // Aliasing: points to data_, but owns entire Container
        return std::shared_ptr<const std::vector<int>>(self, &data_);
    }
};

int main() {
    std::shared_ptr<const std::vector<int>> data;
    
    {
        auto container = std::make_shared<Container>("MyContainer");
        data = container->getData(container);
        // container goes out of scope, but Container object stays alive!
    }
    
    std::cout << "Container variable gone, but data still valid:\n";
    for (int val : *data) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    data.reset();  // NOW Container is destroyed
    
    return 0;
}
```

## ⚡ Performance Considerations

### Memory Overhead

```cpp
struct Tiny {
    char c;  // 1 byte
};

sizeof(Tiny);                          // 1 byte
sizeof(std::unique_ptr<Tiny>);         // 8 bytes (just pointer)
sizeof(std::shared_ptr<Tiny>);         // 16 bytes (2 pointers)
// Plus control block: ~32 bytes minimum

// For small objects, overhead is significant!
```

### Atomic Operations Cost

```cpp
#include <chrono>
#include <iostream>
#include <memory>

void testCopyPerformance() {
    const int iterations = 10000000;
    
    // Test shared_ptr copying (atomic ops)
    auto sp = std::make_shared<int>(42);
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto copy = sp;  // Atomic increment + decrement
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    
    // Test raw pointer copying (no atomic ops)
    int* raw = new int(42);
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        int* copy = raw;  // Just pointer copy
        (void)copy;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    
    auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
    auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    
    std::cout << "shared_ptr: " << time1 << "ms\n";
    std::cout << "raw pointer: " << time2 << "ms\n";
    
    delete raw;
}
```

### Optimization Tips

```cpp
// ❌ SLOW: Many copies, many atomic ops
void process(std::shared_ptr<Widget> sp) {
    for (int i = 0; i < 1000; ++i) {
        auto local = sp;  // Copy!
        local->doWork();
    }
}

// ✅ FASTER: Pass by const reference
void process(const std::shared_ptr<Widget>& sp) {
    for (int i = 0; i < 1000; ++i) {
        sp->doWork();  // No copy!
    }
}

// ✅ EVEN BETTER: If you don't need to extend lifetime
void process(Widget& w) {
    for (int i = 0; i < 1000; ++i) {
        w.doWork();  // Direct access!
    }
}

// Call like:
auto sp = std::make_shared<Widget>();
process(*sp);  // Pass by reference
```

## 🎯 Advanced Patterns

### Pattern 1: Shared Caching

```cpp
#include <memory>
#include <map>
#include <string>

class ResourceCache {
private:
    std::map<std::string, std::weak_ptr<Resource>> cache_;
    
public:
    std::shared_ptr<Resource> getResource(const std::string& key) {
        // Check if resource exists in cache
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            // Try to lock weak_ptr
            if (auto sp = it->second.lock()) {
                std::cout << "Cache hit: " << key << "\n";
                return sp;
            } else {
                // Resource expired, remove from cache
                cache_.erase(it);
            }
        }
        
        // Load resource
        std::cout << "Cache miss, loading: " << key << "\n";
        auto resource = std::make_shared<Resource>(key);
        cache_[key] = resource;  // Store as weak_ptr
        return resource;
    }
};
```

### Pattern 2: Double-checked Locking with shared_ptr

```cpp
#include <memory>
#include <mutex>
#include <atomic>

class Singleton {
private:
    static std::atomic<Singleton*> instance_;
    static std::mutex mutex_;
    
    Singleton() = default;
    
public:
    static std::shared_ptr<Singleton> getInstance() {
        static std::shared_ptr<Singleton> shared_instance;
        static std::once_flag flag;
        
        std::call_once(flag, []() {
            shared_instance = std::shared_ptr<Singleton>(new Singleton());
        });
        
        return shared_instance;
    }
};
```

## 📋 Best Practices Summary

### ✅ DO:

1. **Use `make_shared`** whenever possible
2. **Pass by const reference** to avoid unnecessary copies
3. **Use `weak_ptr`** to break circular references
4. **Use `enable_shared_from_this`** when you need `shared_ptr` from `this`
5. **Consider `unique_ptr` first**, convert to `shared_ptr` if needed
6. **Store `weak_ptr` in caches** and observer lists

### ❌ DON'T:

1. **Never create multiple `shared_ptr`s from same raw pointer**
2. **Don't use `shared_ptr` when ownership is clearly exclusive**
3. **Don't pass `shared_ptr` by value** when you don't need to extend lifetime
4. **Don't call `shared_from_this()` before a `shared_ptr` exists**
5. **Don't create circular references** with `shared_ptr`
6. **Don't use `shared_ptr<void>`** unless you have a very good reason

## 💡 Final Summary

### shared_ptr Key Points:
1. Implements **shared ownership** through reference counting
2. Use **`make_shared`** for efficiency and safety
3. **Deleter type is NOT part of shared_ptr type** (unlike unique_ptr)
4. Use **`enable_shared_from_this`** to safely get shared_ptr from this
5. Use **`weak_ptr`** to break circular references
6. **Reference counting is thread-safe**, object access is not
7. Has **overhead**: control block + atomic operations
8. **Aliasing constructor** allows pointing to member while owning whole object

### Decision Tree:

```
Do you need shared ownership?
├─ No ──────────────────────────────── Use unique_ptr (or stack object)
└─ Yes
   ├─ Simple shared ownership ──────── Use shared_ptr with make_shared
   ├─ Need to break cycles ─────────── Use weak_ptr
   ├─ Need shared_from_this() ───────── Inherit from enable_shared_from_this
   ├─ Custom cleanup ───────────────── Use shared_ptr with custom deleter
   └─ Point to member while owning whole ─ Use aliasing constructor
```

### Performance Hierarchy (Fast → Slow):
1. Stack objects (no heap allocation)
2. `unique_ptr` (single allocation, no ref counting)
3. `shared_ptr` (overhead from ref counting)
4. Raw pointers with manual management (fast but dangerous)

> **Remember:** `shared_ptr` is powerful but has cost. Use it when you truly need shared ownership. When in doubt, start with `unique_ptr` and convert to `shared_ptr` if needed!

