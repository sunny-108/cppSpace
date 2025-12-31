# Programming Assignment - Item 38: Be Aware of Varying Thread Handle Destructor Behavior

## Learning Objectives
- Understand when future destructors block
- Master shared state management
- Distinguish between different future sources
- Avoid accidental blocking in destructors

---

## Exercise 1: Future Destructor Behavior Detection (Easy)

### Problem
Create a tool that detects whether a future will block on destruction.

### Requirements
```cpp
class FutureAnalyzer {
public:
    enum class DestructorBehavior {
        Blocks,
        DoesNotBlock,
        Unknown
    };
    
    template<typename T>
    static DestructorBehavior analyzeFuture(const std::future<T>& f);
    
    static void demonstrateBlocking();
    static void demonstrateNonBlocking();
};
```

### Example
```cpp
auto fut1 = std::async(std::launch::async, task);
auto behavior = FutureAnalyzer::analyzeFuture(fut1);
// Returns DestructorBehavior::Blocks

std::packaged_task<int()> pt(task);
auto fut2 = pt.get_future();
auto behavior2 = FutureAnalyzer::analyzeFuture(fut2);
// Returns DestructorBehavior::DoesNotBlock
```

### Hints
- Cannot directly detect from future API
- Use test scenarios
- Document known behaviors

---

## Exercise 2: Timing Future Destructors (Medium)

### Problem
Measure and demonstrate blocking behavior of different future types.

### Requirements
```cpp
class FutureDestructorTimer {
public:
    struct TimingResult {
        std::string futureSource;
        std::chrono::milliseconds destructorTime;
        bool didBlock;
    };
    
    TimingResult timeAsyncFuture(int taskDurationMs);
    TimingResult timePackagedTaskFuture(int taskDurationMs);
    TimingResult timePromiseFuture(int taskDurationMs);
    TimingResult timeDeferredFuture(int taskDurationMs);
    
    void printAllResults();
};
```

### Example Output
```
async future:         Destructor blocked for 2000ms ✓
packaged_task future: Destructor returned in 0ms
promise future:       Destructor returned in 0ms
deferred future:      Destructor returned in 0ms
```

### Hints
- Use std::chrono to time destructor
- Create futures in inner scopes
- Compare with task duration

---

## Exercise 3: Shared State Management (Medium)

### Problem
Demonstrate shared state reference counting and last-reference blocking.

### Requirements
```cpp
class SharedStateDemo {
public:
    static void demonstrateLastReferencBlocks();
    static void demonstrateSharedFuture();
    static void demonstrateMovedFuture();
    
    struct StateInfo {
        int referenceCount;
        bool willBlockOnDestruction;
    };
    
    // Note: Can't actually access shared state,
    // but can demonstrate behavior
    static void explainSharedState();
};
```

### Example
```cpp
SharedStateDemo::demonstrateLastReferenceBlocks();
// Shows:
// - First future copied: doesn't block
// - Last future destroyed: BLOCKS
```

### Hints
- Use std::shared_future for multiple references
- Only last reference blocks
- Move empties the source

---

## Exercise 4: Safe Async Wrapper (Hard)

### Problem
Create a wrapper that makes blocking behavior explicit and predictable.

### Requirements
```cpp
template<typename T>
class ExplicitFuture {
public:
    enum class BlockingPolicy {
        BlockOnDestruction,
        NoBlockOnDestruction
    };
    
    ExplicitFuture(std::future<T>&& f, BlockingPolicy policy);
    ~ExplicitFuture();
    
    T get();
    void wait();
    
    template<typename Rep, typename Period>
    std::future_status wait_for(
        const std::chrono::duration<Rep, Period>& timeout);
    
private:
    std::future<T> future;
    BlockingPolicy policy;
    bool resultRetrieved = false;
};
```

### Example
```cpp
// Explicit blocking
ExplicitFuture<int> fut1(
    std::async(std::launch::async, task),
    ExplicitFuture<int>::BlockingPolicy::BlockOnDestruction
);
// Destructor will wait

// Explicit non-blocking (must get() result first)
ExplicitFuture<int> fut2(
    std::async(std::launch::async, task),
    ExplicitFuture<int>::BlockingPolicy::NoBlockOnDestruction
);
int result = fut2.get();  // Must call before destruction
```

### Hints
- Check policy in destructor
- If NoBlock, assert get() was called
- Document requirements

---

## Exercise 5: Future Container (Medium)

### Problem
Create a container for futures that handles destruction properly.

### Requirements
```cpp
template<typename T>
class FutureContainer {
public:
    void add(std::future<T>&& future);
    
    std::vector<T> getAllResults();  // Blocks until all ready
    
    void waitAll();  // Explicit wait
    
    size_t readyCount() const;
    size_t pendingCount() const;
    
    ~FutureContainer();  // May block!
    
private:
    std::vector<std::future<T>> futures;
    mutable std::mutex mutex;
};
```

### Example
```cpp
FutureContainer<int> container;

for (int i = 0; i < 10; ++i) {
    container.add(std::async(std::launch::async, compute, i));
}

// Explicit wait before destruction
container.waitAll();

// Or let destructor block (but document it!)
```

### Hints
- Store futures in vector
- Destructor blocks until all complete
- Provide explicit wait method

---

## Exercise 6: Async Task Chain (Hard)

### Problem
Create a task chain where blocking behavior is controlled.

### Requirements
```cpp
template<typename T>
class TaskChain {
public:
    TaskChain(std::function<T()> initialTask);
    
    template<typename F>
    auto then(F&& f) -> TaskChain<std::invoke_result_t<F, T>>;
    
    T get();  // Force evaluation
    
    void detach();  // Fire and forget (no blocking)
    
    ~TaskChain();
    
private:
    std::future<T> future;
    bool detached = false;
};
```

### Example
```cpp
auto chain = TaskChain<int>([]() { return 42; })
    .then([](int x) { return x * 2; })
    .then([](int x) { return std::to_string(x); });

std::string result = chain.get();  // "84"

// Or detach for fire-and-forget
chain.detach();  // Won't block on destruction
```

### Hints
- Chain futures using continuation pattern
- Detach sets flag to prevent blocking
- Get() forces immediate evaluation

---

## Exercise 7: Packaged Task vs Async Comparison (Medium)

### Problem
Demonstrate the difference in destructor behavior between packaged_task and async futures.

### Requirements
```cpp
class FutureComparisonDemo {
public:
    struct Comparison {
        std::string source;
        bool blocksOnDestruction;
        std::string explanation;
    };
    
    static Comparison analyzeAsyncFuture();
    static Comparison analyzePackagedTaskFuture();
    static Comparison analyzePromiseFuture();
    
    static void runAllComparisons();
};
```

### Example Output
```
async future:
  Blocks: YES
  Reason: Last reference to shared state from std::async

packaged_task future:
  Blocks: NO
  Reason: Not created by std::async

promise future:
  Blocks: NO
  Reason: Not created by std::async
```

### Hints
- Time each destructor
- Document the three conditions for blocking
- Show examples of each

---

## Exercise 8: Future Registry (Hard)

### Problem
Track all futures and ensure clean shutdown.

### Requirements
```cpp
class FutureRegistry {
public:
    template<typename T>
    std::shared_ptr<std::future<T>> registerFuture(
        std::future<T>&& future,
        const std::string& name);
    
    void waitAll();
    void waitFor(const std::string& name);
    
    struct FutureStats {
        size_t total;
        size_t ready;
        size_t pending;
        std::vector<std::string> pendingNames;
    };
    
    FutureStats getStats() const;
    
    ~FutureRegistry();  // Wait for all futures
    
private:
    struct Entry {
        std::string name;
        std::function<void()> waiter;
        std::function<bool()> isReady;
    };
    
    std::vector<Entry> entries;
    mutable std::mutex mutex;
};
```

### Example
```cpp
FutureRegistry registry;

auto f1 = registry.registerFuture(
    std::async(std::launch::async, task1), 
    "Task1"
);

auto f2 = registry.registerFuture(
    std::async(std::launch::async, task2),
    "Task2"
);

// Check status
auto stats = registry.getStats();
std::cout << stats.pending << " tasks pending\n";

// Clean shutdown
registry.waitAll();
```

### Hints
- Type-erase futures for storage
- Use shared_ptr for reference counting
- Provide statistics method

---

## Exercise 9: Implicit vs Explicit Blocking (Medium)

### Problem
Create examples showing implicit blocking and how to make it explicit.

### Requirements
```cpp
class BlockingStyleDemo {
public:
    // Bad: Implicit blocking
    static void implicitBlockingExample();
    
    // Good: Explicit blocking
    static void explicitBlockingExample();
    
    // Best: No blocking needed
    static void noBlockingExample();
    
    static void showBestPractices();
};
```

### Example
```cpp
// Implicit (BAD)
void implicitBlockingExample() {
    auto fut = std::async(std::launch::async, longTask);
    if (condition) {
        return;  // ← Blocks here! Not obvious
    }
}

// Explicit (GOOD)
void explicitBlockingExample() {
    auto fut = std::async(std::launch::async, longTask);
    if (condition) {
        fut.wait();  // ← Explicit wait
        return;
    }
}
```

### Hints
- Show implicit blocking surprises
- Make waits explicit with comments
- Prefer getting results when needed

---

## Exercise 10: Future Lifetime Analyzer (Advanced)

### Problem
Create a debugging tool that tracks future lifetimes and warns about blocking.

### Requirements
```cpp
class FutureLifetimeAnalyzer {
public:
    template<typename T>
    class TrackedFuture {
    public:
        TrackedFuture(std::future<T>&& f, const std::string& name);
        ~TrackedFuture();
        
        T get();
        void wait();
        
        std::string getName() const;
        bool willBlock() const;
        
    private:
        std::future<T> future;
        std::string name;
        std::chrono::system_clock::time_point creationTime;
        bool isFromAsync;
    };
    
    static void printReport();
    static void enableWarnings();
};
```

### Example
```cpp
FutureLifetimeAnalyzer::enableWarnings();

{
    auto f = FutureLifetimeAnalyzer::TrackedFuture(
        std::async(std::launch::async, task),
        "MyTask"
    );
    
    // Warning printed: "MyTask will block on destruction!"
}  // Blocks here

FutureLifetimeAnalyzer::printReport();
// Shows all futures, lifetimes, blocking behavior
```

### Hints
- Track creation source (async, packaged_task, etc.)
- Warn if destroying async future without get()
- Generate report of all futures

---

## Testing Your Solutions

### Test Scenarios
```cpp
void testBlockingBehavior() {
    auto start = std::chrono::steady_clock::now();
    
    {
        auto fut = std::async(std::launch::async, []() {
            std::this_thread::sleep_for(2s);
        });
    }  // Should block for ~2 seconds
    
    auto elapsed = std::chrono::steady_clock::now() - start;
    assert(elapsed >= 2s);
}

void testNonBlockingBehavior() {
    auto start = std::chrono::steady_clock::now();
    
    {
        std::packaged_task<void()> pt([]() {
            std::this_thread::sleep_for(2s);
        });
        auto fut = pt.get_future();
        std::thread t(std::move(pt));
        t.detach();
    }  // Should return immediately
    
    auto elapsed = std::chrono::steady_clock::now() - start;
    assert(elapsed < 100ms);
}
```

### Verification Checklist
- [ ] Understand all three blocking conditions
- [ ] Can predict blocking behavior
- [ ] Make blocking explicit in code
- [ ] Avoid accidental blocking
- [ ] Document future sources
- [ ] Handle shared_future correctly

---

## Common Pitfalls

1. **Forgetting async futures block**
   ```cpp
   void processData() {
       auto fut = std::async(std::launch::async, process);
       if (error) return;  // ← BLOCKS HERE!
   }
   ```

2. **Assuming all futures block**
   ```cpp
   std::packaged_task<int()> pt(task);
   auto fut = pt.get_future();
   std::thread t(std::move(pt));
   t.detach();
   // fut destructor does NOT block
   ```

3. **Not understanding shared_future**
   ```cpp
   auto fut = std::async(std::launch::async, task);
   auto sf1 = fut.share();
   auto sf2 = sf1;
   // Only when last shared_future destroyed does it block
   ```

---

## Grading Rubric

| Criteria | Points |
|----------|--------|
| Understanding blocking rules | 40 |
| Code correctness | 25 |
| Documentation | 15 |
| Testing | 12 |
| Best practices | 8 |
| **Total** | **100** |
