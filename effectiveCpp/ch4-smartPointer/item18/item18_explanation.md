# Item 18: Use std::unique_ptr for Exclusive-Ownership Resource Management

## 📌 Core Concept

`std::unique_ptr` is a smart pointer that **exclusively owns** the object it points to. When the `unique_ptr` is destroyed, it automatically deletes the object it owns. Think of it as a "I'm the only owner" pointer.

## 🎯 Why Use std::unique_ptr?

### The Problem with Raw Pointers
```cpp
void processWidget() {
    Widget* pw = new Widget();
    // ... do something ...
    // Oops! If exception occurs here, memory leaks!
    delete pw;  // May never execute
}
```

### The Solution with std::unique_ptr
```cpp
void processWidget() {
    std::unique_ptr<Widget> pw(new Widget());
    // ... do something ...
    // pw automatically deletes Widget when it goes out of scope
    // Even if exception occurs!
}
```

## 🔑 Key Characteristics

### 1. **Exclusive Ownership**
Only ONE `unique_ptr` can own a resource at a time.

```cpp
auto pw1 = std::make_unique<Widget>();
// auto pw2 = pw1;  // ❌ ERROR! Cannot copy!
auto pw2 = std::move(pw1);  // ✅ OK! Transfer ownership
// Now pw1 is nullptr, pw2 owns the Widget
```

### 2. **Zero Overhead**
`unique_ptr` is the same size as a raw pointer (when using default deleter)!

```cpp
sizeof(Widget*) == sizeof(std::unique_ptr<Widget>)  // Usually true
```

### 3. **Automatic Cleanup**
No need to manually call `delete`. RAII (Resource Acquisition Is Initialization) in action!

```cpp
{
    auto pw = std::make_unique<Widget>();
    // Use pw...
}  // Widget automatically deleted here
```

## 📖 Complete Examples

### Example 1: Basic Usage
```cpp
#include <memory>
#include <iostream>

class Investment {
public:
    Investment(std::string name) : name_(name) {
        std::cout << "Creating " << name_ << "\n";
    }
    ~Investment() {
        std::cout << "Destroying " << name_ << "\n";
    }
    void describe() const {
        std::cout << "Investment: " << name_ << "\n";
    }
private:
    std::string name_;
};

int main() {
    // Create using make_unique (preferred)
    auto stock = std::make_unique<Investment>("Apple Stock");
    stock->describe();
    
    // Automatic cleanup when stock goes out of scope
    return 0;
}

// Output:
// Creating Apple Stock
// Investment: Apple Stock
// Destroying Apple Stock
```

### Example 2: Custom Deleters
Sometimes you need custom cleanup logic (not just `delete`).

```cpp
#include <memory>
#include <iostream>

class FileHandle {
public:
    FileHandle(const char* filename) {
        std::cout << "Opening file: " << filename << "\n";
    }
};

// Custom deleter
auto fileDeleter = [](FileHandle* fh) {
    std::cout << "Closing file with custom deleter\n";
    delete fh;
};

int main() {
    // unique_ptr with custom deleter
    std::unique_ptr<FileHandle, decltype(fileDeleter)> file(
        new FileHandle("data.txt"),
        fileDeleter
    );
    
    // File will be closed with custom deleter when 'file' is destroyed
    return 0;
}
```

### Example 3: Factory Functions (Real-World Pattern)
```cpp
#include <memory>
#include <iostream>
#include <string>

class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() const = 0;
};

class Dog : public Animal {
public:
    void speak() const override {
        std::cout << "Woof!\n";
    }
};

class Cat : public Animal {
public:
    void speak() const override {
        std::cout << "Meow!\n";
    }
};

// Factory function returning unique_ptr
std::unique_ptr<Animal> makeAnimal(const std::string& type) {
    if (type == "dog") {
        return std::make_unique<Dog>();
    } else if (type == "cat") {
        return std::make_unique<Cat>();
    }
    return nullptr;
}

int main() {
    auto pet1 = makeAnimal("dog");
    auto pet2 = makeAnimal("cat");
    
    if (pet1) pet1->speak();  // Woof!
    if (pet2) pet2->speak();  // Meow!
    
    // Both animals automatically deleted
    return 0;
}
```

### Example 4: Arrays with unique_ptr
```cpp
#include <memory>
#include <iostream>

int main() {
    // For arrays, use unique_ptr<T[]>
    auto arr = std::make_unique<int[]>(5);
    
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
    
    // Array automatically deleted with delete[] (not delete)
    return 0;
}
```

### Example 5: Transferring Ownership
```cpp
#include <memory>
#include <iostream>
#include <vector>

class Task {
public:
    Task(int id) : id_(id) {
        std::cout << "Task " << id_ << " created\n";
    }
    ~Task() {
        std::cout << "Task " << id_ << " destroyed\n";
    }
    int getId() const { return id_; }
private:
    int id_;
};

std::vector<std::unique_ptr<Task>> createTasks() {
    std::vector<std::unique_ptr<Task>> tasks;
    
    for (int i = 0; i < 3; ++i) {
        tasks.push_back(std::make_unique<Task>(i));
    }
    
    return tasks;  // Move semantics automatically applied
}

int main() {
    auto taskList = createTasks();
    
    std::cout << "\nProcessing tasks...\n";
    for (const auto& task : taskList) {
        std::cout << "Task ID: " << task->getId() << "\n";
    }
    
    std::cout << "\nEnding program...\n";
    return 0;
}
```

## 🔄 Conversion to shared_ptr

`unique_ptr` can be **easily converted** to `shared_ptr`, but not vice versa!

```cpp
std::unique_ptr<Widget> uw = std::make_unique<Widget>();

// Easy conversion
std::shared_ptr<Widget> sw = std::move(uw);  // ✅ OK!
// Now uw is nullptr, sw owns the Widget

// But you CANNOT convert shared_ptr to unique_ptr
// std::unique_ptr<Widget> uw2 = std::move(sw);  // ❌ ERROR!
```

## ⚖️ unique_ptr vs Raw Pointer vs shared_ptr

| Feature | Raw Pointer | unique_ptr | shared_ptr |
|---------|-------------|------------|------------|
| Automatic cleanup | ❌ No | ✅ Yes | ✅ Yes |
| Ownership semantics | ❌ Unclear | ✅ Exclusive | ✅ Shared |
| Size overhead | None | None* | Yes (control block) |
| Copy | ✅ Yes | ❌ No | ✅ Yes |
| Move | ✅ Yes | ✅ Yes | ✅ Yes |
| Performance | Fastest | Nearly same | Slower (ref counting) |

*With default deleter

## 🎓 Best Practices

### ✅ DO:
```cpp
// 1. Use make_unique (C++14 and later)
auto pw = std::make_unique<Widget>();

// 2. Use unique_ptr for factory functions
std::unique_ptr<Base> createObject();

// 3. Store unique_ptr in containers
std::vector<std::unique_ptr<Widget>> widgets;

// 4. Use custom deleters when needed
std::unique_ptr<FILE, decltype(&fclose)> file(fopen("f.txt", "r"), &fclose);
```

### ❌ DON'T:
```cpp
// 1. Don't use raw new with unique_ptr (prefer make_unique)
std::unique_ptr<Widget> pw(new Widget());  // Works, but not preferred

// 2. Don't try to copy
auto pw2 = pw1;  // ERROR!

// 3. Don't create unique_ptr from same raw pointer twice
Widget* raw = new Widget();
std::unique_ptr<Widget> pw1(raw);
std::unique_ptr<Widget> pw2(raw);  // ❌ Double delete! Undefined behavior!
```

## 🔍 Common Pitfalls

### Pitfall 1: Dangling Raw Pointer
```cpp
Widget* raw;
{
    auto uw = std::make_unique<Widget>();
    raw = uw.get();  // Get raw pointer
}  // uw destroyed, Widget deleted
// raw is now dangling!
```

### Pitfall 2: Exception Safety in Constructors
```cpp
// ❌ BAD: Not exception-safe
processWidget(std::unique_ptr<Widget>(new Widget()),
              computePriority());

// If computePriority() throws, Widget may leak!

// ✅ GOOD: Exception-safe
auto uw = std::make_unique<Widget>();
processWidget(std::move(uw), computePriority());
```

## 📊 When to Use What?

```
┌─────────────────────────────────────────────────┐
│ Do you need to share ownership?                 │
└─────────────┬───────────────────────────────────┘
              │
       No ────┼──── Yes
              │       │
              ▼       ▼
      unique_ptr   shared_ptr
     (exclusive)   (shared)
```

## 🎯 Summary

1. **Use `unique_ptr` by default** for dynamic memory management
2. **Zero overhead** compared to raw pointers (with default deleter)
3. **Exception-safe** and prevents memory leaks
4. **Move-only** semantics enforce exclusive ownership
5. **Can convert to `shared_ptr`** if needed later
6. **Perfect for factory functions** and PIMPL idiom
7. **Supports custom deleters** for flexible resource management
8. **Use `make_unique`** for exception safety and cleaner code

## 💡 Remember

> "When in doubt, use `unique_ptr`. It's as efficient as raw pointers but far safer. You can always convert to `shared_ptr` later if you discover shared ownership is actually needed."

