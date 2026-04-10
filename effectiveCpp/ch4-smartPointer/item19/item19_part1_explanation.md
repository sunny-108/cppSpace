# Item 19 (Part 1): Use std::shared_ptr for Shared-Ownership Resource Management

## 📌 Core Concept

`std::shared_ptr` is a smart pointer that implements **shared ownership**. Multiple `shared_ptr` instances can point to the same object, and the object is automatically deleted when the **last** `shared_ptr` owning it is destroyed.

Think of it as: "We all share this toy, and when the last person is done with it, we throw it away."

## 🎯 The Fundamental Difference

### unique_ptr vs shared_ptr

```cpp
// unique_ptr: EXCLUSIVE ownership
std::unique_ptr<Widget> uw = std::make_unique<Widget>();
// auto uw2 = uw;  // ❌ ERROR! Can't copy!

// shared_ptr: SHARED ownership
std::shared_ptr<Widget> sw1 = std::make_shared<Widget>();
auto sw2 = sw1;  // ✅ OK! Both share ownership
auto sw3 = sw1;  // ✅ OK! Now 3 shared_ptrs share ownership
// Widget deleted only when sw1, sw2, AND sw3 are all destroyed
```

## 🔢 Reference Counting: The Magic Behind shared_ptr

`shared_ptr` uses **reference counting** to track how many `shared_ptr` instances point to the same object.

### How Reference Counting Works

```cpp
#include <memory>
#include <iostream>

class Widget {
public:
    Widget() { std::cout << "Widget created\n"; }
    ~Widget() { std::cout << "Widget destroyed\n"; }
};

int main() {
    std::cout << "Creating sp1...\n";
    std::shared_ptr<Widget> sp1 = std::make_shared<Widget>();
    std::cout << "Reference count: " << sp1.use_count() << "\n";  // 1
    
    {
        std::cout << "\nCreating sp2 from sp1...\n";
        auto sp2 = sp1;
        std::cout << "sp1 count: " << sp1.use_count() << "\n";  // 2
        std::cout << "sp2 count: " << sp2.use_count() << "\n";  // 2
        
        {
            std::cout << "\nCreating sp3 from sp1...\n";
            auto sp3 = sp1;
            std::cout << "Reference count: " << sp1.use_count() << "\n";  // 3
        }
        std::cout << "\nsp3 destroyed, count: " << sp1.use_count() << "\n";  // 2
    }
    std::cout << "\nsp2 destroyed, count: " << sp1.use_count() << "\n";  // 1
    
    std::cout << "\nAbout to exit main...\n";
    return 0;  // Widget destroyed here when sp1 goes out of scope
}
```

**Output:**
```
Creating sp1...
Widget created
Reference count: 1

Creating sp2 from sp1...
sp1 count: 2
sp2 count: 2

Creating sp3 from sp1...
Reference count: 3

sp3 destroyed, count: 2

sp2 destroyed, count: 1

About to exit main...
Widget destroyed
```

## 🏗️ The Control Block

Each `shared_ptr` pointing to an object shares a **control block** that contains:

1. **Reference count** - Number of `shared_ptr`s pointing to the object
2. **Weak count** - Number of `weak_ptr`s pointing to the object (more in Part 2)
3. **Deleter** - Custom deleter if provided
4. **Allocator** - Custom allocator if provided
5. **Other data** - For internal bookkeeping

### Visual Representation

```
┌─────────────┐         ┌──────────────────┐         ┌─────────┐
│  shared_ptr │────────▶│  Control Block   │────────▶│ Widget  │
│     sp1     │         │                  │         │ Object  │
└─────────────┘         │ Ref Count: 3     │         └─────────┘
                        │ Weak Count: 0    │              ▲
┌─────────────┐         │ Deleter: default │              │
│  shared_ptr │────────▶│ Allocator: ...   │              │
│     sp2     │         └──────────────────┘              │
└─────────────┘                                           │
                                                          │
┌─────────────┐                                           │
│  shared_ptr │───────────────────────────────────────────┘
│     sp3     │
└─────────────┘
```

## 📖 Basic Examples

### Example 1: Simple Shared Ownership

```cpp
#include <memory>
#include <iostream>
#include <vector>

class Document {
private:
    std::string name_;
    
public:
    Document(std::string name) : name_(std::move(name)) {
        std::cout << "Document '" << name_ << "' created\n";
    }
    
    ~Document() {
        std::cout << "Document '" << name_ << "' destroyed\n";
    }
    
    void display() const {
        std::cout << "Document: " << name_ << "\n";
    }
};

class User {
private:
    std::string name_;
    std::shared_ptr<Document> doc_;
    
public:
    User(std::string name, std::shared_ptr<Document> doc)
        : name_(std::move(name)), doc_(std::move(doc)) {
        std::cout << "User '" << name_ << "' accessing document\n";
    }
    
    void viewDocument() const {
        if (doc_) {
            std::cout << name_ << " is viewing: ";
            doc_->display();
        }
    }
};

int main() {
    // Create a document
    auto document = std::make_shared<Document>("Report.pdf");
    std::cout << "Initial ref count: " << document.use_count() << "\n\n";
    
    // Multiple users can share the same document
    User alice("Alice", document);
    std::cout << "After Alice: ref count = " << document.use_count() << "\n";
    
    User bob("Bob", document);
    std::cout << "After Bob: ref count = " << document.use_count() << "\n";
    
    User charlie("Charlie", document);
    std::cout << "After Charlie: ref count = " << document.use_count() << "\n\n";
    
    alice.viewDocument();
    bob.viewDocument();
    charlie.viewDocument();
    
    std::cout << "\nAll users and document going out of scope...\n";
    return 0;
    // Document destroyed only after ALL users are destroyed
}
```

### Example 2: Factory Functions Returning shared_ptr

```cpp
#include <memory>
#include <iostream>
#include <string>

class Connection {
private:
    std::string serverName_;
    int connectionId_;
    static int nextId_;
    
public:
    Connection(std::string server) 
        : serverName_(std::move(server)), connectionId_(nextId_++) {
        std::cout << "Connected to " << serverName_ 
                  << " (ID: " << connectionId_ << ")\n";
    }
    
    ~Connection() {
        std::cout << "Disconnected from " << serverName_ 
                  << " (ID: " << connectionId_ << ")\n";
    }
    
    void sendData(const std::string& data) const {
        std::cout << "Sending to " << serverName_ << ": " << data << "\n";
    }
};

int Connection::nextId_ = 1;

// Factory function - multiple clients can share connection
std::shared_ptr<Connection> connectToServer(const std::string& server) {
    return std::make_shared<Connection>(server);
}

int main() {
    // Create connection
    auto conn = connectToServer("database.example.com");
    
    // Share connection with multiple components
    auto conn2 = conn;  // Copy - increases ref count
    auto conn3 = conn;  // Another copy
    
    std::cout << "Active connections: " << conn.use_count() << "\n\n";
    
    conn->sendData("SELECT * FROM users");
    conn2->sendData("INSERT INTO logs...");
    conn3->sendData("UPDATE settings...");
    
    std::cout << "\nConnection auto-closed when all shared_ptrs destroyed\n";
    return 0;
}
```

### Example 3: Storing in Containers

```cpp
#include <memory>
#include <vector>
#include <iostream>

class Task {
private:
    int id_;
    std::string description_;
    
public:
    Task(int id, std::string desc) 
        : id_(id), description_(std::move(desc)) {
        std::cout << "Task " << id_ << " created\n";
    }
    
    ~Task() {
        std::cout << "Task " << id_ << " destroyed\n";
    }
    
    void execute() const {
        std::cout << "Executing Task " << id_ << ": " << description_ << "\n";
    }
    
    int getId() const { return id_; }
};

int main() {
    std::vector<std::shared_ptr<Task>> taskQueue;
    std::vector<std::shared_ptr<Task>> activeTasksBackup;
    
    // Create tasks and add to queue
    for (int i = 1; i <= 3; ++i) {
        auto task = std::make_shared<Task>(i, "Important work #" + std::to_string(i));
        taskQueue.push_back(task);
        
        // Also keep backup reference
        activeTasksBackup.push_back(task);
    }
    
    std::cout << "\nTask 1 ref count: " << taskQueue[0].use_count() << "\n\n";
    
    // Execute and remove from queue
    std::cout << "Processing queue:\n";
    while (!taskQueue.empty()) {
        taskQueue.back()->execute();
        taskQueue.pop_back();
    }
    
    std::cout << "\nQueue empty, but tasks still alive in backup!\n";
    std::cout << "Backup size: " << activeTasksBackup.size() << "\n";
    
    std::cout << "\nClearing backup...\n";
    activeTasksBackup.clear();  // Now tasks are destroyed
    
    return 0;
}
```

## 🔄 Copying vs Moving shared_ptr

### Copying - Increases Reference Count
```cpp
std::shared_ptr<Widget> sp1 = std::make_shared<Widget>();
std::cout << "sp1 count: " << sp1.use_count() << "\n";  // 1

std::shared_ptr<Widget> sp2 = sp1;  // COPY
std::cout << "sp1 count: " << sp1.use_count() << "\n";  // 2
std::cout << "sp2 count: " << sp2.use_count() << "\n";  // 2
```

### Moving - Does NOT Increase Reference Count
```cpp
std::shared_ptr<Widget> sp1 = std::make_shared<Widget>();
std::cout << "sp1 count: " << sp1.use_count() << "\n";  // 1

std::shared_ptr<Widget> sp2 = std::move(sp1);  // MOVE
std::cout << "sp1 count: " << (sp1 ? sp1.use_count() : 0) << "\n";  // 0 (sp1 is null)
std::cout << "sp2 count: " << sp2.use_count() << "\n";  // 1 (same count)
```

### Performance Implication

```cpp
// ❌ SLOWER - atomic increment/decrement on each copy
void processWidget(std::shared_ptr<Widget> sp) {
    // sp copied here - ref count incremented
    sp->doSomething();
}  // sp destroyed - ref count decremented

// ✅ FASTER - no ref count changes, but can't extend lifetime
void processWidget(Widget* w) {
    w->doSomething();
}

// ✅ GOOD COMPROMISE - no ref count changes, safer than raw pointer
void processWidget(const std::shared_ptr<Widget>& sp) {
    sp->doSomething();
}
```

## 🎓 When to Use shared_ptr

### ✅ Use shared_ptr when:

1. **Multiple owners** need access to the same resource
   ```cpp
   // GUI: Multiple widgets share same texture
   std::shared_ptr<Texture> texture = loadTexture("icon.png");
   button1->setTexture(texture);
   button2->setTexture(texture);
   ```

2. **Ownership is unclear or dynamic**
   ```cpp
   // Not sure which component will outlive others
   std::shared_ptr<DatabaseConnection> db = connect();
   cache->setConnection(db);
   logger->setConnection(db);
   // Connection lives until last user is done
   ```

3. **Callbacks and async operations**
   ```cpp
   void startAsyncOperation() {
       auto data = std::make_shared<LargeData>();
       
       // Lambda captures shared_ptr, extending lifetime
       asyncTask([data]() {
           // Use data... even if startAsyncOperation() has returned
           processData(*data);
       });
   }
   ```

4. **Caching shared resources**
   ```cpp
   // Resource cache - multiple clients share cached items
   std::map<std::string, std::shared_ptr<Resource>> cache;
   ```

### ❌ Don't use shared_ptr when:

1. **Ownership is clearly exclusive**
   ```cpp
   // Use unique_ptr instead
   std::unique_ptr<Widget> widget = std::make_unique<Widget>();
   ```

2. **Performance is critical and copying happens frequently**
   ```cpp
   // Ref counting has overhead - use raw pointer or reference
   void hotPath(Widget& w) {  // Not shared_ptr!
       w.fastOperation();
   }
   ```

3. **Objects don't outlive their scope**
   ```cpp
   // Just use stack allocation
   void processData() {
       Widget w;  // No heap allocation needed
       w.doWork();
   }
   ```

## ⚠️ Common Pitfalls (Part 1)

### Pitfall 1: Creating shared_ptr from Same Raw Pointer Twice

```cpp
// ❌ DANGER! Double delete!
Widget* raw = new Widget();
std::shared_ptr<Widget> sp1(raw);
std::shared_ptr<Widget> sp2(raw);  // BAD! Two control blocks!
// Both will try to delete raw -> crash!
```

**Solution:**
```cpp
// ✅ CORRECT: Create once, copy the shared_ptr
auto sp1 = std::make_shared<Widget>();
auto sp2 = sp1;  // Share the same control block
```

### Pitfall 2: Mixing shared_ptr and unique_ptr Incorrectly

```cpp
// ❌ Can't convert shared_ptr to unique_ptr
std::shared_ptr<Widget> sp = std::make_shared<Widget>();
// std::unique_ptr<Widget> up = std::move(sp);  // ERROR!

// ✅ But can convert unique_ptr to shared_ptr
std::unique_ptr<Widget> up = std::make_unique<Widget>();
std::shared_ptr<Widget> sp = std::move(up);  // OK!
```

### Pitfall 3: Creating shared_ptr from `this`

```cpp
class Widget {
public:
    std::shared_ptr<Widget> getShared() {
        // ❌ WRONG! Creates new control block!
        return std::shared_ptr<Widget>(this);
    }
};

// When you do this:
auto sp1 = std::make_shared<Widget>();
auto sp2 = sp1->getShared();  // BAD! Two control blocks!
```

**Solution in Part 2: `std::enable_shared_from_this`**

## 📊 shared_ptr vs unique_ptr Comparison

| Feature | unique_ptr | shared_ptr |
|---------|------------|------------|
| Ownership | Exclusive | Shared |
| Copy | ❌ No | ✅ Yes |
| Move | ✅ Yes | ✅ Yes |
| Size | 1 pointer | 2 pointers* |
| Overhead | None (default deleter) | Control block + atomic ops |
| Thread-safe ref counting | N/A | ✅ Yes |
| Use case | Single clear owner | Multiple owners |

*One pointer to object, one to control block

## 🔍 Understanding Reference Count Operations

### Thread Safety

```cpp
// Reference counting is THREAD-SAFE
std::shared_ptr<Widget> global_sp;

// Thread 1
void thread1() {
    auto local_sp = global_sp;  // Safe: atomic increment
}

// Thread 2
void thread2() {
    auto local_sp = global_sp;  // Safe: atomic increment
}

// But the object itself is NOT automatically thread-safe!
void thread3() {
    global_sp->modify();  // NOT SAFE without synchronization!
}
```

### Cost of Reference Counting

```cpp
// Each copy/destroy involves atomic operations
for (int i = 0; i < 1000000; ++i) {
    auto sp2 = sp1;  // Atomic increment
}  // Atomic decrement 1 million times

// More efficient: use reference to avoid copies
void processMany(const std::shared_ptr<Widget>& sp) {
    for (int i = 0; i < 1000000; ++i) {
        sp->doWork();  // No ref count changes!
    }
}
```

## 💡 Summary (Part 1)

1. **`shared_ptr` implements shared ownership** through reference counting
2. **Object deleted when last shared_ptr is destroyed** (ref count reaches 0)
3. **Copy increases ref count**, move transfers ownership without changing count
4. **Control block manages ref count** and other metadata
5. **Reference counting is thread-safe**, but the object itself isn't
6. **Has overhead**: two pointers + atomic operations
7. **Use when multiple owners needed**, otherwise prefer `unique_ptr`
8. **Never create multiple shared_ptrs from same raw pointer**

## 🎯 Coming in Part 2

- `std::make_shared` and why it's important
- Custom deleters with `shared_ptr`
- `std::enable_shared_from_this` - safely getting `shared_ptr` from `this`
- `std::weak_ptr` - breaking circular references
- Aliasing constructor
- Performance optimizations
- Advanced patterns and best practices

