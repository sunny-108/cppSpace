# Item 18 Programming Assignments: std::unique_ptr

## 📚 Assignment Overview
These assignments progress from medium to advanced difficulty, covering real-world scenarios where `std::unique_ptr` is essential.

---

## Assignment 1: Resource Manager with Custom Deleters (Medium)

### 🎯 Objective
Implement a resource management system that handles different types of resources (files, sockets, database connections) using `unique_ptr` with custom deleters.

### 📋 Requirements

1. Create a base `Resource` class with virtual destructor
2. Implement three derived classes:
   - `FileResource` - manages FILE* handles
   - `SocketResource` - manages socket descriptors (use int to simulate)
   - `DatabaseResource` - manages DB connections (use mock object)

3. Create custom deleters for each resource type that print cleanup messages

4. Implement a `ResourceManager` class that:
   - Stores resources in a `std::vector<std::unique_ptr<Resource>>`
   - Provides methods to add/remove resources
   - Automatically cleans up all resources on destruction
   - Returns resource count and can list all active resources

5. Demonstrate exception safety by throwing exceptions during resource creation

### 💻 Skeleton Code
```cpp
#include <memory>
#include <vector>
#include <iostream>
#include <string>

// TODO: Implement base Resource class

// TODO: Implement FileResource class

// TODO: Implement SocketResource class

// TODO: Implement DatabaseResource class

// TODO: Implement ResourceManager class

int main() {
    // Test your implementation
    return 0;
}
```

### ✅ Expected Output
```
Creating File Resource: data.txt
Creating Socket Resource on port: 8080
Creating Database Resource: UserDB
Resources active: 3
Listing resources:
  - File: data.txt
  - Socket: 8080
  - Database: UserDB
Closing File: data.txt
Closing Socket: 8080
Closing Database: UserDB
Resources cleaned up
```

### 🎓 Learning Outcomes
- Custom deleters with `unique_ptr`
- Managing different resource types polymorphically
- RAII and automatic resource cleanup
- Exception safety with smart pointers

---

## Assignment 2: Abstract Factory Pattern with unique_ptr (Medium-Advanced)

### 🎯 Objective
Implement the Abstract Factory pattern using `unique_ptr` for a GUI component system that supports multiple platforms.

### 📋 Requirements

1. Create abstract base classes:
   - `Button` with pure virtual `render()` and `click()`
   - `TextBox` with pure virtual `render()` and `getText()`
   - `GUIFactory` with pure virtual methods to create buttons and textboxes

2. Implement two concrete factories:
   - `WindowsFactory` - creates Windows-style components
   - `LinuxFactory` - creates Linux-style components

3. Implement concrete component classes for each platform (4 classes total)

4. Create an `Application` class that:
   - Takes a `unique_ptr<GUIFactory>` in constructor
   - Stores created GUI components
   - Has a `createUI()` method that creates multiple components
   - Has a `renderAll()` method that renders all components
   - Properly transfers ownership of components

5. Demonstrate creating applications for both platforms

### 💻 Key Design Points
```cpp
class Application {
private:
    std::unique_ptr<GUIFactory> factory_;
    std::vector<std::unique_ptr<Button>> buttons_;
    std::vector<std::unique_ptr<TextBox>> textboxes_;
    
public:
    // Constructor takes ownership of factory
    explicit Application(std::unique_ptr<GUIFactory> factory);
    
    void createUI();
    void renderAll();
};
```

### ✅ Expected Behavior
```cpp
auto winApp = std::make_unique<Application>(
    std::make_unique<WindowsFactory>()
);
winApp->createUI();
winApp->renderAll();

// Output:
// [Windows Button] Rendering...
// [Windows TextBox] Rendering...
// ...
```

### 🎓 Learning Outcomes
- Abstract Factory pattern with smart pointers
- Transferring ownership through factory methods
- Storing polymorphic objects in containers
- Move semantics in constructors

---

## Assignment 3: Thread-Safe Object Pool (Advanced)

### 🎯 Objective
Implement a thread-safe object pool that manages expensive-to-create objects using `unique_ptr` to ensure exclusive access.

### 📋 Requirements

1. Create a `PooledObject` class template that wraps an object and returns it to the pool on destruction

2. Implement an `ObjectPool<T>` class template that:
   - Pre-creates a configurable number of objects
   - Provides `acquire()` method returning `std::unique_ptr<PooledObject<T>>`
   - Automatically reclaims objects when `unique_ptr` is destroyed
   - Is thread-safe (use mutex)
   - Tracks available vs. in-use objects
   - Blocks or returns nullptr when pool is exhausted (configurable)

3. Create a `DatabaseConnection` class to pool:
   - Simulates connection time (sleep)
   - Has methods: `executeQuery()`, `getConnectionId()`

4. Demonstrate multi-threaded usage with 5 threads trying to get 3 pooled connections

### 💻 Advanced Template Design
```cpp
template<typename T>
class PooledObject {
private:
    T* object_;
    std::function<void(T*)> returnToPool_;
    
public:
    // Constructor, destructor, accessors
    // Automatically returns object to pool on destruction
};

template<typename T>
class ObjectPool {
private:
    std::vector<std::unique_ptr<T>> pool_;
    std::vector<T*> available_;
    std::mutex mutex_;
    
public:
    explicit ObjectPool(size_t size);
    std::unique_ptr<PooledObject<T>> acquire();
    size_t availableCount() const;
    size_t totalSize() const;
    
private:
    void returnObject(T* obj);
};
```

### 🎓 Learning Outcomes
- Custom deleters with captured state
- RAII for resource management
- Thread safety with smart pointers
- Template metaprogramming
- Object pool pattern

---

## Assignment 4: Expression Tree Builder (Advanced)

### 🎯 Objective
Build an expression tree evaluator that uses `unique_ptr` for tree node ownership and supports various operations.

### 📋 Requirements

1. Create an `Expr` abstract base class with:
   - Pure virtual `eval()` method returning `double`
   - Pure virtual `toString()` method returning `std::string`
   - Virtual destructor

2. Implement concrete expression node types:
   - `NumberExpr` - holds a number
   - `BinaryExpr` - holds operator and two child expressions
   - `UnaryExpr` - holds operator and one child expression
   - `VariableExpr` - holds variable name, looks up value in context

3. Implement an `ExprBuilder` class with fluent interface:
   ```cpp
   auto expr = ExprBuilder()
       .add(
           ExprBuilder().number(5),
           ExprBuilder().multiply(
               ExprBuilder().variable("x"),
               ExprBuilder().number(3)
           )
       )
       .build();
   ```

4. Implement an `ExpressionContext` class that stores variable values

5. Support operations: +, -, *, /, unary -, sqrt

6. Implement expression simplification (e.g., x * 0 = 0, x + 0 = x)

7. Implement expression cloning (deep copy)

### ✅ Example Usage
```cpp
ExpressionContext ctx;
ctx.setVariable("x", 4.0);
ctx.setVariable("y", 2.0);

auto expr = ExprBuilder()
    .multiply(
        ExprBuilder().add(
            ExprBuilder().variable("x"),
            ExprBuilder().variable("y")
        ),
        ExprBuilder().number(3)
    )
    .build();

std::cout << expr->toString() << " = " << expr->eval(ctx) << "\n";
// Output: ((x + y) * 3) = 18

auto simplified = expr->simplify();
auto cloned = expr->clone();
```

### 🎓 Learning Outcomes
- Tree data structures with smart pointers
- Recursive algorithms with unique_ptr
- Builder pattern with move semantics
- Deep copying polymorphic hierarchies
- Visitor pattern (for simplification)

---

## Assignment 5: Plugin System with Dynamic Loading (Advanced)

### 🎯 Objective
Create a plugin system that dynamically loads and manages plugins using `unique_ptr`, simulating a real-world extensible application architecture.

### 📋 Requirements

1. Define a `Plugin` interface:
   ```cpp
   class Plugin {
   public:
       virtual ~Plugin() = default;
       virtual std::string getName() const = 0;
       virtual std::string getVersion() const = 0;
       virtual bool initialize() = 0;
       virtual void shutdown() = 0;
       virtual void execute(const std::string& command) = 0;
   };
   ```

2. Implement at least 3 concrete plugin types:
   - `LoggingPlugin` - logs messages to file/console
   - `DataProcessorPlugin` - processes data with various algorithms
   - `NetworkPlugin` - simulates network operations

3. Create a `PluginManager` class that:
   - Stores plugins in `std::map<std::string, std::unique_ptr<Plugin>>`
   - Loads plugins (simulated, not actual DLL loading)
   - Initializes plugins in dependency order
   - Unloads plugins safely
   - Executes commands on specific plugins
   - Handles plugin dependencies (e.g., NetworkPlugin depends on LoggingPlugin)
   - Broadcasts events to all plugins

4. Implement a `PluginFactory` that creates plugins based on type string

5. Add plugin dependency resolution:
   - Plugins can specify dependencies
   - Manager initializes in correct order
   - Manager prevents circular dependencies

6. Implement plugin versioning and compatibility checking

### 💻 Advanced Features
```cpp
class PluginManager {
private:
    std::map<std::string, std::unique_ptr<Plugin>> plugins_;
    std::map<std::string, std::vector<std::string>> dependencies_;
    
public:
    bool loadPlugin(std::unique_ptr<Plugin> plugin);
    std::unique_ptr<Plugin> unloadPlugin(const std::string& name);
    bool executePluginCommand(const std::string& pluginName, 
                             const std::string& command);
    void broadcastEvent(const std::string& event);
    bool hasPlugin(const std::string& name) const;
    
private:
    bool initializeWithDependencies();
    std::vector<std::string> resolveDependencyOrder();
    bool detectCircularDependencies();
};
```

### ✅ Example Usage
```cpp
PluginManager manager;

// Load plugins
manager.loadPlugin(std::make_unique<LoggingPlugin>());
manager.loadPlugin(std::make_unique<NetworkPlugin>());  // Depends on Logging
manager.loadPlugin(std::make_unique<DataProcessorPlugin>());

// Plugins initialized in dependency order automatically

// Execute commands
manager.executePluginCommand("Network", "connect:example.com");
manager.broadcastEvent("system_startup");

// Unload specific plugin
auto removed = manager.unloadPlugin("DataProcessor");
```

### 🎓 Learning Outcomes
- Plugin architecture design
- Dependency resolution algorithms
- Topological sorting for initialization order
- Transferring ownership in complex systems
- Factory pattern with smart pointers
- Circular dependency detection
- Resource management in extensible systems

---

## Assignment 6: Memory-Efficient Trie with Smart Pointers (Advanced)

### 🎯 Objective
Implement a memory-efficient Trie (prefix tree) data structure using `unique_ptr` for automatic node management, with advanced features.

### 📋 Requirements

1. Create a `TrieNode` class that:
   - Uses `std::array<std::unique_ptr<TrieNode>, 26>` for children (lowercase a-z)
   - Stores whether it's end of word
   - Optionally stores word frequency/metadata

2. Implement a `Trie` class with methods:
   - `insert(const std::string& word)` - inserts word
   - `search(const std::string& word)` - exact match
   - `startsWith(const std::string& prefix)` - prefix search
   - `remove(const std::string& word)` - removes word, cleans up empty nodes
   - `getAllWords()` - returns all words in trie
   - `autocomplete(const std::string& prefix)` - returns suggestions
   - `longestCommonPrefix()` - finds longest common prefix
   - `countWords()` - total words in trie
   - `countNodes()` - total nodes in trie

3. Implement advanced features:
   - **Lazy deletion**: mark as deleted without immediately removing nodes
   - **Compression**: compress single-child chains into one node
   - **Serialization**: save/load trie to/from file
   - **Statistics**: memory usage, average depth, etc.

4. Add support for:
   - Case-insensitive search (option)
   - Wildcards in search (e.g., "c?t" matches "cat", "cot")
   - Fuzzy search (allow N mistakes)

### 💻 Implementation Hints
```cpp
class TrieNode {
private:
    std::array<std::unique_ptr<TrieNode>, 26> children_;
    bool isEndOfWord_;
    int frequency_;
    
public:
    TrieNode() : isEndOfWord_(false), frequency_(0) {}
    
    // Methods to access/modify children
    TrieNode* getChild(char c);
    TrieNode* addChild(char c);
    bool hasChildren() const;
    size_t childrenCount() const;
};

class Trie {
private:
    std::unique_ptr<TrieNode> root_;
    size_t wordCount_;
    
    // Helper methods
    void collectWords(TrieNode* node, const std::string& prefix,
                     std::vector<std::string>& words);
    bool removeHelper(TrieNode* node, const std::string& word, size_t depth);
    
public:
    Trie() : root_(std::make_unique<TrieNode>()), wordCount_(0) {}
    
    // Public interface
    void insert(const std::string& word);
    bool search(const std::string& word) const;
    bool remove(const std::string& word);
    std::vector<std::string> autocomplete(const std::string& prefix) const;
    // ... more methods
};
```

### ✅ Test Cases
```cpp
Trie dictionary;

// Insert words
dictionary.insert("cat");
dictionary.insert("catch");
dictionary.insert("car");
dictionary.insert("card");
dictionary.insert("dog");

// Search
assert(dictionary.search("cat") == true);
assert(dictionary.search("ca") == false);
assert(dictionary.startsWith("ca") == true);

// Autocomplete
auto suggestions = dictionary.autocomplete("ca");
// Returns: ["cat", "catch", "car", "card"]

// Remove
dictionary.remove("cat");
assert(dictionary.search("cat") == false);
assert(dictionary.search("catch") == true);  // Still exists

// Statistics
std::cout << "Total words: " << dictionary.countWords() << "\n";
std::cout << "Total nodes: " << dictionary.countNodes() << "\n";
```

### 🎓 Learning Outcomes
- Tree structures with smart pointers
- Recursive deletion and cleanup
- Memory-efficient data structures
- Prefix algorithms
- Complex ownership patterns
- Node compression techniques

---

## Assignment 7: Smart Pointer Intrusive List (Advanced/Expert)

### 🎯 Objective
Implement an intrusive doubly-linked list that uses `unique_ptr` for forward links and raw pointers for backward links, demonstrating advanced ownership patterns.

### 📋 Requirements

1. Create a `ListNode<T>` class template:
   - Holds data of type T
   - Has `unique_ptr<ListNode<T>>` for next
   - Has raw pointer `ListNode<T>*` for prev
   - Move-only (no copy)

2. Implement an `IntrusiveList<T>` class template:
   - `push_front(T value)` - adds to front
   - `push_back(T value)` - adds to back
   - `pop_front()` - returns `unique_ptr<ListNode<T>>`
   - `pop_back()` - returns `unique_ptr<ListNode<T>>`
   - `insert_after(ListNode<T>* node, T value)`
   - `remove(ListNode<T>* node)` - returns `unique_ptr<ListNode<T>>`
   - `size()`, `empty()`, `clear()`
   - Iterator support (forward and reverse)

3. Implement proper move semantics for the list

4. Add methods for:
   - Splicing lists together
   - Reversing the list
   - Sorting the list (merge sort)
   - Finding elements

5. Ensure exception safety in all operations

6. Write comprehensive tests including:
   - Memory leak tests
   - Exception safety tests
   - Performance comparisons with std::list

### 💻 Key Challenge
```cpp
template<typename T>
class ListNode {
public:
    T data;
    std::unique_ptr<ListNode<T>> next;
    ListNode<T>* prev;  // Raw pointer to avoid circular ownership
    
    explicit ListNode(T value) 
        : data(std::move(value)), next(nullptr), prev(nullptr) {}
};

template<typename T>
class IntrusiveList {
private:
    std::unique_ptr<ListNode<T>> head_;
    ListNode<T>* tail_;  // Raw pointer
    size_t size_;
    
public:
    IntrusiveList() : head_(nullptr), tail_(nullptr), size_(0) {}
    
    // Challenge: Implement these without memory leaks!
    void push_back(T value);
    std::unique_ptr<ListNode<T>> pop_front();
    
    // More challenging: implement sorting while maintaining ownership
    void sort();
};
```

### 🎓 Learning Outcomes
- Advanced ownership patterns (unique + raw pointer mix)
- Intrusive data structures
- Move semantics in containers
- Exception safety with smart pointers
- Algorithm implementation with ownership transfer
- Understanding when to use raw pointers safely

---

## 🎯 Grading Rubric

Each assignment is evaluated on:

| Criteria | Points | Description |
|----------|--------|-------------|
| **Correctness** | 40% | Code compiles and passes all test cases |
| **Smart Pointer Usage** | 20% | Proper use of unique_ptr, no memory leaks |
| **Design** | 15% | Clean architecture, good abstractions |
| **Exception Safety** | 10% | Handles errors gracefully |
| **Code Quality** | 10% | Readable, well-commented, follows best practices |
| **Performance** | 5% | Efficient implementation |

## 📚 Additional Challenges

For each assignment, try these extra challenges:

1. **Add comprehensive unit tests** using a testing framework
2. **Measure memory usage** and compare with raw pointers
3. **Add performance benchmarks** 
4. **Implement move assignment operator** where applicable
5. **Add debug logging** to track object lifetimes
6. **Create visualization** of object relationships
7. **Add thread safety** where appropriate

## 🚀 Bonus Assignment Ideas

1. **Smart Pointer Debugging Tool**: Create a wrapper around unique_ptr that tracks all allocations/deallocations
2. **Custom Allocator Integration**: Implement unique_ptr with custom allocators
3. **Intrusive Smart Pointer**: Implement your own intrusive smart pointer similar to boost::intrusive_ptr
4. **ABA Problem Solution**: Use unique_ptr to solve the ABA problem in lock-free data structures

