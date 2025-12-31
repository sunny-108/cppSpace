# Assignments 08-12: Advanced Concurrency Topics

## Assignment 08: Thread-Safe Data Structures

### Overview
Design and implement thread-safe versions of common data structures with fine-grained locking and lock-free alternatives.

**Key Topics:**
- Thread-safe queue, stack, map, list
- Fine-grained locking strategies
- Concurrent skip lists
- B-tree concurrency control

### MCQs (15 questions covering concurrent data structure design patterns)

### Exercises:
1. **Thread-Safe Unordered Map**: Implement with lock striping
2. **Concurrent Skip List**: Lock-free or fine-grained locking
3. **Wait-Free Queue**: Single producer, single consumer
4. **Concurrent B+ Tree**: Multi-reader, multi-writer
5. **Benchmark Suite**: Compare all implementations

---

## Assignment 09: Performance & Optimization

### Overview
Profile, measure, and optimize concurrent code. Understand cache effects, false sharing, and scalability bottlenecks.

**Key Topics:**
- Cache coherence and false sharing
- NUMA awareness
- Lock contention measurement
- Profiling with perf/VTune
- Scalability analysis

### MCQs (15 questions on performance measurement and optimization)

### Exercises:
1. **False Sharing Detection**: Identify and fix in real code
2. **NUMA-Aware Allocator**: Implement thread-local caching
3. **Lock Contention Analysis**: Instrument code to measure
4. **Scalability Study**: Amdahl's vs Gustafson's Law analysis
5. **CPU Cache Optimization**: Align data structures, prefetching

### Implementations:
```cpp
// Example: NUMA-aware thread pool
class NUMAThreadPool {
    std::vector<std::unique_ptr<WorkerGroup>> numaNodes;
    
public:
    NUMAThreadPool();
    
    // Submit task to specific NUMA node
    template<typename F>
    auto submit(size_t numaNode, F&& f) -> std::future<...>;
    
    // Submit with automatic NUMA locality
    template<typename F>
    auto submitLocal(F&& f) -> std::future<...>;
};

// Benchmark framework
class PerformanceBenchmark {
public:
    struct Metrics {
        double throughput;
        double latency;
        uint64_t cacheMisses;
        uint64_t branchMispredictions;
        double scalability;
    };
    
    Metrics measureWithPerfCounters();
    void generateScalabilityGraph();
    void detectBottlenecks();
};
```

---

## Assignment 10: Advanced Concurrency Patterns

### Overview
Implement complex concurrency patterns: producer-consumer variants, work stealing, fork-join, actors.

**Key Topics:**
- Multiple producer, multiple consumer
- Work stealing schedulers
- Fork-join parallelism
- Actor model in C++
- Pipeline parallelism

### MCQs (15 questions on concurrency patterns)

### Exercises:
1. **Work Stealing Scheduler**: Implement Cilk-style deque
2. **Actor System**: Message passing with mailboxes
3. **Fork-Join Framework**: Divide and conquer parallelism
4. **Pipeline Pattern**: Staged processing with backpressure
5. **Reactive Streams**: Observer pattern for async data

### Implementations:
```cpp
// Work stealing deque
template<typename T>
class WorkStealingDeque {
public:
    void push(T item);           // Owner only
    std::optional<T> pop();      // Owner only
    std::optional<T> steal();    // Other threads
    
private:
    std::atomic<size_t> top{0};
    std::atomic<size_t> bottom{0};
    std::vector<T> buffer;
};

// Actor model
class Actor {
public:
    virtual ~Actor() = default;
    
    template<typename Message>
    void send(Message&& msg);
    
protected:
    virtual void receive(const Message& msg) = 0;
    
private:
    ConcurrentQueue<Message> mailbox;
    std::thread worker;
};

// Fork-join task
template<typename T>
class ForkJoinTask {
public:
    virtual T compute() = 0;
    T fork();
    void join();
    
    static void invokeAll(std::vector<ForkJoinTask*> tasks);
};
```

---

## Assignment 11: Debugging & Testing Concurrent Code

### Overview
Master tools and techniques for finding and fixing concurrency bugs. Systematic testing approaches.

**Key Topics:**
- ThreadSanitizer, Helgrind
- Race detection strategies  
- Stress testing
- Deadlock detection
- Model checking

### MCQs (15 questions on debugging techniques)

### Exercises:
1. **Race Condition Hunt**: Given buggy code, find all races
2. **Stress Test Framework**: Generate random interleavings
3. **Deadlock Detector**: Runtime cycle detection
4. **Sanitizer Integration**: Use TSan on large codebase
5. **Model Checker**: Use CHESS or similar tool

### Implementations:
```cpp
// Stress tester
class ConcurrencyStressTester {
public:
    struct Config {
        size_t numThreads;
        size_t iterations;
        std::chrono::milliseconds duration;
        bool randomDelays;
    };
    
    template<typename TestFunc>
    void run(TestFunc test, const Config& config);
    
    // Report race conditions found
    std::vector<RaceCondition> getDetectedRaces() const;
};

// Deadlock detector
class RuntimeDeadlockDetector {
public:
    void registerLockAcquire(void* lock, std::thread::id tid);
    void registerLockRelease(void* lock, std::thread::id tid);
    
    bool detectCycle();
    std::vector<std::thread::id> getDeadlockedThreads();
    void printLockGraph();
};

// Property-based testing
template<typename T>
class ConcurrentPropertyTester {
public:
    // Check linearizability
    bool checkLinearizability(
        std::function<void(T&)> operations,
        size_t numThreads
    );
    
    // Check for memory leaks
    bool checkMemoryLeaks();
    
    // Generate test report
    void generateReport(const std::string& filename);
};
```

---

## Assignment 12: Real-World Concurrent Systems

### Overview
Design and implement complete concurrent systems: web server, task scheduler, database cache.

**Key Topics:**
- HTTP server with thread pool
- Job scheduler with priorities
- Distributed cache
- Real-time event processor
- Concurrent game server

### MCQs (15 questions on system design)

### Project Options (Choose 2):

### Project 1: Multi-threaded Web Server
```cpp
class HTTPServer {
public:
    HTTPServer(uint16_t port, size_t numThreads);
    
    void registerHandler(const std::string& path, Handler handler);
    void start();
    void stop();
    
private:
    class ConnectionHandler;
    class ThreadPool;
    class RequestRouter;
    
    // Your implementation
};

// Requirements:
// - Handle 10,000+ concurrent connections
// - Support keep-alive
// - Request routing
// - Graceful shutdown
// - Load balancing across threads
// - Performance: >100K requests/sec
```

### Project 2: Task Scheduler
```cpp
class DistributedTaskScheduler {
public:
    // Submit task with constraints
    TaskID submitTask(Task task, const Constraints& constraints);
    
    // Schedule periodic tasks
    TaskID scheduleRecurring(Task task, std::chrono::milliseconds interval);
    
    // Cancel task
    void cancelTask(TaskID id);
    
    // Task dependencies
    void addDependency(TaskID task, TaskID dependency);
    
    // Resource limits
    void setResourceLimits(const ResourceLimits& limits);
    
private:
    // Implement:
    // - Priority queues
    // - Dependency resolution
    // - Resource management
    // - Failure handling
    // - Metrics and monitoring
};
```

### Project 3: In-Memory Cache
```cpp
class ConcurrentCache {
public:
    // Basic operations
    void put(const Key& key, const Value& value);
    std::optional<Value> get(const Key& key);
    void erase(const Key& key);
    
    // Eviction policies
    void setEvictionPolicy(EvictionPolicy policy);
    void setMaxSize(size_t size);
    
    // Batch operations
    void putBatch(const std::vector<std::pair<Key, Value>>& items);
    std::vector<Value> getBatch(const std::vector<Key>& keys);
    
    // Statistics
    CacheStats getStats() const;
    
private:
    // Implement:
    // - LRU/LFU eviction
    // - Lock striping
    // - TTL support
    // - Memory management
    // - Hit/miss tracking
};
```

### Project 4: Game Server
```cpp
class MultiplayerGameServer {
public:
    // Player management
    PlayerID connectPlayer(const std::string& name);
    void disconnectPlayer(PlayerID id);
    
    // Game state
    void updatePlayerState(PlayerID id, const PlayerState& state);
    GameState getGameState() const;
    
    // Messaging
    void broadcastMessage(const Message& msg);
    void sendToPlayer(PlayerID id, const Message& msg);
    
    // Tick system
    void startGameLoop(std::chrono::milliseconds tickRate);
    
private:
    // Requirements:
    // - 60 tick/sec simulation
    // - Handle 1000+ concurrent players
    // - State synchronization
    // - Lag compensation
    // - Cheat detection
};
```

### Evaluation Criteria:
- **Design (30%)**: Architecture and component interaction
- **Correctness (25%)**: Thread safety, no races
- **Performance (25%)**: Meets performance targets
- **Robustness (20%)**: Error handling, graceful degradation

### Deliverables:
1. Complete implementation
2. Design document with diagrams
3. Test suite with stress tests
4. Performance benchmarks
5. User documentation

---

## Overall Assignment Series Summary

### Progression:
1. **01-03**: Fundamentals (threads, locks, deadlocks)
2. **04-05**: Synchronization (CVs, atomics, memory model)
3. **06-07**: Advanced techniques (lock-free, async)
4. **08-09**: Optimization (data structures, performance)
5. **10-12**: Integration (patterns, debugging, systems)

### Skills Developed:
- ✅ Thread management and lifecycle
- ✅ Synchronization primitives mastery
- ✅ Lock-free programming
- ✅ Memory model understanding
- ✅ Performance optimization
- ✅ Debugging concurrent code
- ✅ System design

### Total Time: 75-90 hours

### Final Project Presentation:
- 30-minute presentation of Assignment 12 project
- Code walkthrough
- Performance demo
- Lessons learned

---

## Additional Resources

### Books:
- C++ Concurrency in Action, 2nd Edition
- The Art of Multiprocessor Programming
- Programming with POSIX Threads

### Papers:
- Maurice Herlihy's work on lock-free data structures
- Michael-Scott queue paper
- Hazard pointers paper

### Tools:
- ThreadSanitizer
- Valgrind/Helgrind
- Intel VTune
- perf (Linux)

### Online:
- CppCon talks on concurrency
- Herb Sutter's blog
- Jeff Preshing's blog
- CppReference concurrency section

---

Good luck completing all assignments!
