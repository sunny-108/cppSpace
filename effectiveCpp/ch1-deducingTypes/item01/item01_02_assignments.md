# Programming Assignments: Template and Auto Type Deduction

## Difficulty Level: Advanced Patterns and Real-World Scenarios

---

## Assignment 1: Type Deduction Logger with Perfect Forwarding

### Objective
Create a sophisticated logging system that uses template type deduction to capture and display the exact types being passed, including cv-qualifiers and reference types.

### Requirements

1. **Implement a TypeLogger class template** that:
   - Uses template type deduction to capture the exact parameter type
   - Displays whether the argument is an lvalue or rvalue
   - Shows cv-qualifiers (const, volatile)
   - Indicates reference types (lvalue ref, rvalue ref, or none)
   - Uses `std::type_info` or custom type traits to display human-readable type names

2. **Create three overloaded log functions:**
   ```cpp
   template<typename T>
   void logByValue(T param);
   
   template<typename T>
   void logByReference(T& param);
   
   template<typename T>
   void logByUniversalReference(T&& param);
   ```

3. **Test with various scenarios:**
   - Primitive types (int, double, char)
   - Const and non-const objects
   - Lvalue and rvalue references
   - Pointers and const pointers
   - Arrays
   - Function pointers
   - Custom class objects

4. **Advanced Challenge:**
   - Implement a compile-time type information extractor using template metaprogramming
   - Create a macro or constexpr function that can display type information at compile time
   - Handle perfect forwarding scenarios where types must be preserved through multiple function calls

### Expected Output Format
```
Calling logByValue with int: Type=int, Value Category=rvalue, CV=none
Calling logByReference with const int&: Type=int, Value Category=lvalue, CV=const
Calling logByUniversalReference with int&&: Type=int, Value Category=rvalue, CV=none
```

### Real-World Application
This pattern is used in frameworks like:
- Logging libraries (spdlog, glog)
- Debugging tools
- Serialization frameworks
- Type-safe variadic function implementations

---

## Assignment 2: Smart Configuration System with Auto Deduction

### Objective
Build a configuration management system that leverages auto type deduction to create a flexible, type-safe configuration API.

### Requirements

1. **Create a Config class** that:
   - Stores configuration values of different types
   - Uses auto deduction for type inference
   - Supports method chaining with auto return types
   - Handles braced initializers for list-like configurations

2. **Implement these features:**
   ```cpp
   class Config {
   public:
       template<typename T>
       auto set(const std::string& key, T&& value);
       
       template<typename T = void>
       auto get(const std::string& key) const;
       
       auto getAll() const;
       
       template<typename... Args>
       auto setMultiple(Args&&... args);
   };
   ```

3. **Usage scenarios to support:**
   ```cpp
   Config cfg;
   
   // Auto deduction for different types
   auto port = cfg.set("port", 8080);
   auto host = cfg.set("host", "localhost");
   auto enabled = cfg.set("enabled", true);
   
   // Handle initializer lists
   auto servers = cfg.set("servers", {"server1", "server2", "server3"});
   
   // Auto with trailing return type
   auto validate() -> decltype(auto);
   ```

4. **Advanced Challenge:**
   - Implement type-safe retrieval with default values
   - Support nested configuration objects
   - Create a fluent API using auto return types
   - Handle array vs. initializer_list correctly
   - Add compile-time type checking

### Real-World Application
Similar patterns are used in:
- Application configuration libraries (boost::program_options)
- JSON/YAML parsers (nlohmann::json)
- Database ORM systems
- GUI framework property systems

---

## Assignment 3: Generic Algorithm Library with Template Type Deduction

### Objective
Develop a library of generic algorithms that demonstrates mastery of template type deduction in complex scenarios.

### Requirements

1. **Implement the following algorithms:**
   ```cpp
   // Transform with perfect forwarding
   template<typename Container, typename Func>
   auto transform(Container&& container, Func&& func);
   
   // Filter with predicate deduction
   template<typename Container, typename Predicate>
   auto filter(Container&& container, Predicate&& pred);
   
   // Reduce/fold with type deduction
   template<typename Container, typename T, typename BinaryOp>
   auto reduce(Container&& container, T&& init, BinaryOp&& op);
   
   // Zip multiple containers with varying types
   template<typename... Containers>
   auto zip(Containers&&... containers);
   
   // Flatten nested containers
   template<typename Container>
   auto flatten(Container&& container);
   ```

2. **Handle these edge cases:**
   - Const containers and const iterators
   - Temporary containers (rvalues)
   - Mixed lvalue/rvalue arguments
   - Return type optimization
   - Move semantics where appropriate

3. **Type deduction challenges:**
   - Deduce result type from function return
   - Handle lambdas with auto parameters
   - Support member function pointers
   - Deal with function objects and functors

4. **Advanced Challenge:**
   - Implement lazy evaluation with iterator adaptors
   - Create expression templates for optimization
   - Add SFINAE constraints for type safety
   - Support ranges (C++20 style)

### Test Cases
```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};

// Test transform
auto squared = transform(nums, [](auto x) { return x * x; });

// Test filter
auto evens = filter(nums, [](auto x) { return x % 2 == 0; });

// Test reduce
auto sum = reduce(nums, 0, [](auto a, auto b) { return a + b; });

// Test zip
std::vector<std::string> names = {"one", "two", "three"};
auto zipped = zip(nums, names);

// Test with temporary
auto result = transform(std::vector{1, 2, 3}, [](int x) { return x * 2; });
```

### Real-World Application
These patterns appear in:
- Range libraries (ranges-v3, C++20 ranges)
- Reactive programming (RxCpp)
- Parallel algorithm libraries (Intel TBB)
- Functional programming libraries

---

## Assignment 4: Perfect Forwarding Wrapper Framework

### Objective
Create a wrapper framework that demonstrates advanced understanding of universal references and perfect forwarding in real-world scenarios.

### Requirements

1. **Build a CallWrapper system** that:
   - Wraps arbitrary function calls
   - Preserves value categories (lvalue/rvalue)
   - Maintains const-correctness
   - Supports member functions and free functions
   - Handles variadic arguments

2. **Implement these wrapper types:**
   ```cpp
   // Performance measurement wrapper
   template<typename Func, typename... Args>
   auto measurePerformance(Func&& func, Args&&... args);
   
   // Exception safety wrapper
   template<typename Func, typename... Args>
   auto safeCall(Func&& func, Args&&... args) noexcept;
   
   // Retry wrapper with exponential backoff
   template<typename Func, typename... Args>
   auto retryCall(int maxAttempts, Func&& func, Args&&... args);
   
   // Cache wrapper with memoization
   template<typename Func>
   class Memoize {
       // Implement call operator with perfect forwarding
   };
   
   // Async wrapper
   template<typename Func, typename... Args>
   auto asyncCall(Func&& func, Args&&... args) 
       -> std::future<decltype(func(std::forward<Args>(args)...))>;
   ```

3. **Demonstrate type deduction in:**
   - Return type deduction (using decltype(auto))
   - Parameter pack deduction
   - Member function pointer deduction
   - Lambda deduction

4. **Advanced Challenge:**
   - Implement move-only type support (like unique_ptr)
   - Handle reference parameters correctly
   - Support overloaded functions
   - Add compile-time checks for callable types
   - Implement transparent wrapper (no overhead in release builds)

### Test Scenarios
```cpp
// Test with free function
int add(int a, int b) { return a + b; }
auto result1 = measurePerformance(add, 5, 10);

// Test with lambda
auto lambda = [](auto&& x) { return x * 2; };
auto result2 = safeCall(lambda, 42);

// Test with member function
struct Calculator {
    int multiply(int a, int b) const { return a * b; }
};
Calculator calc;
auto result3 = retryCall(3, &Calculator::multiply, calc, 6, 7);

// Test with move-only type
auto result4 = asyncCall([](std::unique_ptr<int> p) { 
    return *p * 2; 
}, std::make_unique<int>(42));

// Test memoization
Memoize<int(int)> fib = fibonacci;
auto result5 = fib(20);  // Computed
auto result6 = fib(20);  // Cached
```

### Real-World Application
Similar patterns are found in:
- AOP (Aspect-Oriented Programming) frameworks
- Performance profiling tools
- Dependency injection containers
- RPC (Remote Procedure Call) frameworks
- Testing/mocking frameworks

---

## Assignment 5: Type-Safe Event System with Auto and Template Deduction

### Objective
Design a modern, type-safe event/observer system that demonstrates sophisticated use of both template and auto type deduction.

### Requirements

1. **Core Event System Components:**
   ```cpp
   // Event class with auto deduction
   template<typename... Args>
   class Event {
   public:
       using Callback = std::function<void(Args...)>;
       
       template<typename Func>
       auto subscribe(Func&& callback);
       
       template<typename... CallArgs>
       void emit(CallArgs&&... args);
       
       auto getSubscriberCount() const;
   };
   
   // EventBus with type deduction
   class EventBus {
   public:
       template<typename EventType, typename Func>
       auto on(Func&& callback);
       
       template<typename EventType, typename... Args>
       void emit(Args&&... args);
       
       template<typename EventType>
       auto getEvent();
   };
   ```

2. **Implement these features:**
   - Type-safe event registration and dispatch
   - Auto deduction for callback types
   - Support for lambdas with auto parameters
   - Event filtering based on type deduction
   - Unsubscribe mechanisms with RAII handles

3. **Advanced Features:**
   ```cpp
   // Async event handling with auto
   template<typename EventType>
   auto onAsync(auto&& callback);
   
   // Event transformation pipeline
   template<typename EventType>
   auto map(auto&& transformer);
   
   template<typename EventType>
   auto filter(auto&& predicate);
   
   // Event aggregation
   template<typename... EventTypes>
   auto combineLatest(EventTypes&... events);
   ```

4. **Type Deduction Challenges:**
   - Deduce event types from callbacks
   - Handle generic lambdas correctly
   - Support move-only event data
   - Preserve const-correctness through dispatch
   - Implement type erasure where necessary

### Test Scenarios
```cpp
// Define events
struct MouseEvent { int x, y; };
struct KeyEvent { char key; };
struct NetworkEvent { std::string data; };

EventBus bus;

// Subscribe with auto deduction
auto handle1 = bus.on<MouseEvent>([](const auto& event) {
    std::cout << "Mouse at: " << event.x << ", " << event.y << "\n";
});

auto handle2 = bus.on<KeyEvent>([](auto key_event) {
    std::cout << "Key pressed: " << key_event.key << "\n";
});

// Emit events
bus.emit<MouseEvent>(100, 200);
bus.emit<KeyEvent>('A');

// Advanced: Event pipeline
auto filteredMouse = bus.getEvent<MouseEvent>()
    .filter([](const auto& e) { return e.x > 50; })
    .map([](const auto& e) { return std::make_pair(e.x, e.y); });

// Combined events
auto combined = combineLatest(
    bus.getEvent<MouseEvent>(),
    bus.getEvent<KeyEvent>()
);
```

### Real-World Application
Event systems like this are used in:
- GUI frameworks (Qt signals/slots, wxWidgets)
- Game engines (Unity, Unreal)
- Reactive frameworks (RxCpp, React.js patterns in C++)
- Message passing systems
- Observer pattern implementations

---

## Assignment 6: Generic Resource Manager with RAII and Type Deduction

### Objective
Create a sophisticated resource management system that combines RAII with template and auto type deduction.

### Requirements

1. **Implement ResourceManager:**
   ```cpp
   template<typename Resource, typename Deleter = std::default_delete<Resource>>
   class ResourceManager {
   public:
       // Auto deduction for resource factories
       template<typename Factory>
       auto acquire(Factory&& factory);
       
       // Template deduction for custom deleters
       template<typename CustomDeleter>
       auto acquireWithDeleter(auto&& factory, CustomDeleter&& deleter);
       
       // Auto return type for resource retrieval
       auto get(const std::string& key) const;
       
       // Perfect forwarding for resource creation
       template<typename... Args>
       auto create(Args&&... args);
   };
   ```

2. **Support various resource types:**
   - File handles (with auto close)
   - Network connections
   - Database connections
   - Memory buffers
   - Thread pools
   - GPU resources

3. **Type deduction scenarios:**
   - Deduce resource type from factory function
   - Auto deduce deleter type from custom cleanup
   - Handle move-only resources
   - Support shared and unique ownership

4. **Advanced Features:**
   ```cpp
   // Resource pool with auto sizing
   template<typename Resource>
   class ResourcePool {
       auto acquire() -> /* deduce RAII handle type */;
       auto size() const;
       auto available() const;
   };
   
   // Scoped resource with auto deduction
   auto withResource(auto&& factory, auto&& action);
   ```

### Test Scenarios
```cpp
// File resource
auto fileManager = ResourceManager<FILE, decltype(&fclose)>();
auto file = fileManager.acquire(
    []() { return fopen("test.txt", "r"); }
);

// Network connection with custom deleter
auto netManager = ResourceManager<Connection>();
auto conn = netManager.acquireWithDeleter(
    []() { return new Connection("localhost", 8080); },
    [](Connection* c) { c->close(); delete c; }
);

// Database connection pool
ResourcePool<DatabaseConnection> dbPool(10);
{
    auto conn = dbPool.acquire();
    // Use connection
} // Auto returned to pool

// Scoped resource usage
auto result = withResource(
    []() { return openFile("data.txt"); },
    [](auto& file) { return processFile(file); }
);
```

### Real-World Application
Resource management patterns like these are critical in:
- Database connection pooling
- Graphics API resource management (OpenGL, Vulkan, DirectX)
- Network programming
- Operating system resource management
- Embedded systems programming

---

## Bonus Assignment 7: Compile-Time Type Deduction Analysis Tool

### Objective
Build a compile-time tool that analyzes and reports type deduction results using template metaprogramming.

### Requirements

1. **Create type traits:**
   ```cpp
   // Detect reference types
   template<typename T>
   struct is_lvalue_reference : /* implementation */;
   
   template<typename T>
   struct is_rvalue_reference : /* implementation */;
   
   // Detect cv qualifiers
   template<typename T>
   struct cv_qualifiers : /* implementation */;
   
   // Deduce array size
   template<typename T>
   struct array_size : /* implementation */;
   
   // Function signature analyzer
   template<typename F>
   struct function_traits : /* implementation */;
   ```

2. **Implement compile-time type printer:**
   ```cpp
   template<typename T>
   constexpr auto type_name();
   
   #define PRINT_TYPE(x) print_type_info<decltype(x)>()
   ```

3. **Create test framework:**
   - Static assertions for type deduction
   - Compile-time unit tests
   - Type deduction verification macros

### Real-World Application
These techniques are used in:
- Template metaprogramming libraries (Boost.MPL)
- Type trait libraries (type_traits)
- Compile-time testing frameworks
- Static analysis tools

---

## Submission Guidelines

For each assignment:

1. **Code Files:**
   - Well-organized header and implementation files
   - Comprehensive comments explaining type deduction behavior
   - Clear naming conventions

2. **Test Suite:**
   - Unit tests covering all scenarios
   - Edge case testing
   - Performance benchmarks (where applicable)

3. **Documentation:**
   - README explaining the design
   - Type deduction behavior documentation
   - Usage examples

4. **Analysis Report:**
   - Explain the type deduction happening in your code
   - Discuss any surprising or counterintuitive behavior
   - Performance implications of your type deduction choices
   - Comparison with alternative approaches

---

## Evaluation Criteria

- **Correctness** (30%): Code works as specified
- **Type Deduction Understanding** (25%): Demonstrates deep understanding of Items 1 & 2
- **Code Quality** (20%): Clean, maintainable, idiomatic C++
- **Testing** (15%): Comprehensive test coverage
- **Documentation** (10%): Clear explanations and examples

---

## Tips for Success

1. **Use compiler explorers:** https://godbolt.org to see actual type deduction
2. **Enable all warnings:** `-Wall -Wextra -Wpedantic`
3. **Use static_assert:** Verify type deduction at compile time
4. **Test with different compilers:** GCC, Clang, MSVC
5. **Profile your code:** Ensure zero-cost abstractions
6. **Read compiler errors carefully:** They reveal type deduction results
7. **Use tools:** cppinsights.io shows what the compiler actually generates

Happy coding! 🚀
