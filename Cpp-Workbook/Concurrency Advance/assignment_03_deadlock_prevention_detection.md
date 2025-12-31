# Assignment 03: Deadlock Prevention & Detection

## Overview
This assignment covers deadlock scenarios, detection, prevention strategies, and recovery mechanisms in concurrent C++ applications. You'll learn to identify, debug, and prevent deadlocks in complex real-world systems.

**Estimated Time:** 6-8 hours

---

## Learning Objectives
- Understand the four conditions necessary for deadlock
- Detect deadlocks using runtime analysis and tools
- Implement deadlock prevention strategies
- Apply lock-free alternatives where appropriate
- Design deadlock-free systems

---

## Part 1: Multiple Choice Questions (MCQs)

### Q1. What are the four necessary conditions for deadlock (Coffman conditions)?
A) Mutual exclusion, hold and wait, no preemption, circular wait  
B) Race condition, data race, atomicity, visibility  
C) Liveness, safety, fairness, progress  
D) Acquire, release, synchronize, fence  

**Answer:** A

---

### Q2. Which strategy breaks the "hold and wait" condition?
A) Always acquire locks in the same order  
B) Require all locks to be acquired at once  
C) Use timeouts on lock acquisition  
D) Make locks preemptible  

**Answer:** B

---

### Q3. `std::lock()` prevents deadlock by:
A) Using a timeout  
B) Acquiring multiple locks atomically without deadlock risk  
C) Detecting cycles in the lock graph  
D) Using priority inversion  

**Answer:** B

---

### Q4. A livelock differs from a deadlock in that:
A) Threads are blocked waiting indefinitely  
B) Threads are active but making no progress  
C) Only one thread is affected  
D) Resources are released automatically  

**Answer:** B

---

### Q5. Which is NOT a deadlock prevention strategy?
A) Lock ordering  
B) Lock timeout  
C) Increasing thread priority  
D) Atomic multi-lock acquisition  

**Answer:** C

---

### Q6. The "Dining Philosophers" problem illustrates:
A) Producer-consumer pattern  
B) Resource allocation deadlock  
C) Memory ordering issues  
D) Cache coherence problems  

**Answer:** B

---

### Q7. Try-lock with backoff is useful for:
A) Guaranteeing deadlock freedom  
B) Reducing contention when acquiring multiple locks  
C) Improving single-lock performance  
D) Preventing race conditions  

**Answer:** B

---

### Q8. A resource allocation graph with a cycle indicates:
A) Definite deadlock  
B) Possible deadlock  
C) No deadlock  
D) Race condition  

**Answer:** B

---

### Q9. The "wait-for" graph in deadlock detection tracks:
A) Which threads are waiting for which resources  
B) Lock acquisition order  
C) Memory dependencies  
D) Thread priorities  

**Answer:** A

---

### Q10. To detect deadlock at runtime, you can:
A) Use static analysis only  
B) Build and analyze the resource allocation graph  
C) Count the number of locks  
D) Check thread priorities  

**Answer:** B

---

### Q11. Priority inversion occurs when:
A) High priority thread waits for low priority thread holding a lock  
B) Locks are acquired in reverse order  
C) Two threads have the same priority  
D) A deadlock is detected  

**Answer:** A

---

### Q12. The Banker's Algorithm is used for:
A) Deadlock detection  
B) Deadlock avoidance by ensuring safe states  
C) Lock-free programming  
D) Race condition prevention  

**Answer:** B

---

### Q13. Which tool can help detect deadlocks in C++ programs?
A) Valgrind's Helgrind  
B) cppcheck  
C) clang-format  
D) gdb alone (without extensions)  

**Answer:** A

---

### Q14. A "safe state" in deadlock avoidance means:
A) No threads can acquire locks  
B) System can allocate resources without entering deadlock  
C) All threads have the same priority  
D) No locks are held  

**Answer:** B

---

### Q15. Breaking circular wait can be achieved by:
A) Imposing a total ordering on lock acquisition  
B) Using more threads  
C) Increasing lock timeout  
D) Making threads sleep longer  

**Answer:** A

---

## Part 2: Code Review Exercises

### Exercise 2.1: Classic Deadlock Scenario

Analyze this code for deadlock vulnerabilities:

```cpp
class Resource {
    std::mutex mtx;
    int value;
    
public:
    Resource(int v) : value(v) {}
    
    void transferTo(Resource& other, int amount) {
        std::lock_guard<std::mutex> lock1(mtx);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
        std::lock_guard<std::mutex> lock2(other.mtx);
        
        value -= amount;
        other.value += amount;
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
};

void scenario1() {
    Resource r1(100), r2(200);
    
    std::thread t1([&] {
        for (int i = 0; i < 1000; ++i) {
            r1.transferTo(r2, 1);
        }
    });
    
    std::thread t2([&] {
        for (int i = 0; i < 1000; ++i) {
            r2.transferTo(r1, 1);
        }
    });
    
    t1.join();
    t2.join();
    
    std::cout << "r1: " << r1.getValue() << ", r2: " << r2.getValue() << "\n";
}

void scenario2() {
    std::vector<Resource> resources;
    for (int i = 0; i < 10; ++i) {
        resources.emplace_back(i * 100);
    }
    
    auto worker = [&](int id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 9);
        
        for (int i = 0; i < 100; ++i) {
            int from = dis(gen);
            int to = dis(gen);
            if (from != to) {
                resources[from].transferTo(resources[to], 10);
            }
        }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

**Questions:**
1. Explain the deadlock in scenario1 step-by-step
2. Why is scenario2 even more prone to deadlock?
3. Draw the resource allocation graph for scenario1
4. Provide at least 3 different solutions
5. Implement each solution and benchmark performance

---

### Exercise 2.2: Hidden Circular Dependency

```cpp
class Node {
    std::mutex mtx;
    int data;
    Node* next;
    
public:
    Node(int d) : data(d), next(nullptr) {}
    
    void setNext(Node* n) {
        std::lock_guard<std::mutex> lock(mtx);
        next = n;
    }
    
    void updateBoth(int newData) {
        std::lock_guard<std::mutex> lock(mtx);
        data = newData;
        
        if (next) {
            std::lock_guard<std::mutex> nextLock(next->mtx);
            next->data = newData + 1;
        }
    }
    
    void propagateUpdate(int delta) {
        std::lock_guard<std::mutex> lock(mtx);
        data += delta;
        
        if (next) {
            next->propagateUpdate(delta);  // Recursive call
        }
    }
    
    int sumChain() {
        std::lock_guard<std::mutex> lock(mtx);
        int sum = data;
        
        if (next) {
            sum += next->sumChain();
        }
        
        return sum;
    }
};

void testCircularList() {
    Node n1(1), n2(2), n3(3);
    
    n1.setNext(&n2);
    n2.setNext(&n3);
    n3.setNext(&n1);  // Creates cycle
    
    std::thread t1([&] { n1.propagateUpdate(10); });
    std::thread t2([&] { n2.propagateUpdate(20); });
    
    t1.join();
    t2.join();
}
```

**Questions:**
1. Identify all deadlock scenarios
2. What makes `propagateUpdate` dangerous with cycles?
3. How does `sumChain` handle cycles? (Hint: it doesn't)
4. Design a deadlock-free version
5. Should locks be held during recursive calls?

---

### Exercise 2.3: Subtle Lock Ordering Issue

```cpp
class DatabaseConnection {
    std::mutex mtx;
    bool connected = false;
    
public:
    void connect() {
        std::lock_guard<std::mutex> lock(mtx);
        connected = true;
    }
    
    void disconnect() {
        std::lock_guard<std::mutex> lock(mtx);
        connected = false;
    }
    
    bool isConnected() const {
        std::lock_guard<std::mutex> lock(mtx);
        return connected;
    }
};

class ConnectionPool {
    std::mutex poolMtx;
    std::vector<std::unique_ptr<DatabaseConnection>> connections;
    DatabaseConnection* primaryConnection;
    
public:
    ConnectionPool(size_t size) : primaryConnection(nullptr) {
        for (size_t i = 0; i < size; ++i) {
            connections.push_back(std::make_unique<DatabaseConnection>());
        }
        if (!connections.empty()) {
            primaryConnection = connections[0].get();
        }
    }
    
    DatabaseConnection* acquire() {
        std::lock_guard<std::mutex> lock(poolMtx);
        for (auto& conn : connections) {
            if (!conn->isConnected()) {  // Nested lock acquisition
                conn->connect();
                return conn.get();
            }
        }
        return nullptr;
    }
    
    void release(DatabaseConnection* conn) {
        std::lock_guard<std::mutex> lock(poolMtx);
        if (conn) {
            conn->disconnect();  // Nested lock acquisition
        }
    }
    
    void resetPrimary() {
        primaryConnection->disconnect();  // Lock 1
        
        std::lock_guard<std::mutex> lock(poolMtx);  // Lock 2
        primaryConnection = connections[0].get();
        primaryConnection->connect();  // Nested lock
    }
    
    void updateAllConnections() {
        std::lock_guard<std::mutex> lock(poolMtx);  // Lock 1
        
        for (auto& conn : connections) {
            if (conn->isConnected()) {  // Lock 2
                conn->disconnect();  // Lock 2
                conn->connect();  // Lock 2
            }
        }
    }
};
```

**Questions:**
1. What lock ordering issues exist?
2. Can `acquire()` and `resetPrimary()` deadlock?
3. What's wrong with nested locking in `acquire()`?
4. Redesign to eliminate nested locking
5. Consider lock hierarchy - what levels would you assign?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Deadlock Detection System

Implement a runtime deadlock detector:

```cpp
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <vector>

class DeadlockDetector {
public:
    using ThreadId = std::thread::id;
    using LockId = void*;
    
    // Notify when a thread attempts to acquire a lock
    void acquireAttempt(ThreadId thread, LockId lock);
    
    // Notify when a thread successfully acquires a lock
    void acquireSuccess(ThreadId thread, LockId lock);
    
    // Notify when a thread releases a lock
    void release(ThreadId thread, LockId lock);
    
    // Check if there's a potential deadlock
    bool detectDeadlock();
    
    // Get threads involved in deadlock
    std::vector<ThreadId> getDeadlockedThreads();
    
    // Print the wait-for graph
    void printWaitForGraph();
    
private:
    std::mutex detectorMtx;
    
    // Lock ownership: lock -> thread
    std::map<LockId, ThreadId> lockOwners;
    
    // Wait-for relationships: thread -> locks it's waiting for
    std::map<ThreadId, std::set<LockId>> waitingFor;
    
    // Locks held by each thread
    std::map<ThreadId, std::set<LockId>> locksHeld;
    
    // Detect cycle in wait-for graph using DFS
    bool hasCycle();
    bool dfs(ThreadId thread, 
             std::set<ThreadId>& visited, 
             std::set<ThreadId>& recStack);
};

// Monitored mutex wrapper
class MonitoredMutex {
public:
    MonitoredMutex(DeadlockDetector& detector) 
        : detector(detector), lockId(this) {}
    
    void lock() {
        auto tid = std::this_thread::get_id();
        detector.acquireAttempt(tid, lockId);
        mtx.lock();
        detector.acquireSuccess(tid, lockId);
    }
    
    void unlock() {
        auto tid = std::this_thread::get_id();
        mtx.unlock();
        detector.release(tid, lockId);
    }
    
    bool try_lock() {
        auto tid = std::this_thread::get_id();
        if (mtx.try_lock()) {
            detector.acquireSuccess(tid, lockId);
            return true;
        }
        return false;
    }
    
private:
    std::mutex mtx;
    DeadlockDetector& detector;
    void* lockId;
};

// Test the deadlock detector
void testDeadlockDetector() {
    DeadlockDetector detector;
    
    MonitoredMutex m1(detector), m2(detector);
    
    std::atomic<bool> startFlag{false};
    
    std::thread t1([&] {
        while (!startFlag);
        
        std::lock_guard<MonitoredMutex> lock1(m1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<MonitoredMutex> lock2(m2);
    });
    
    std::thread t2([&] {
        while (!startFlag);
        
        std::lock_guard<MonitoredMutex> lock1(m2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<MonitoredMutex> lock2(m1);
    });
    
    startFlag = true;
    
    // Check for deadlock after a short delay
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    if (detector.detectDeadlock()) {
        std::cout << "DEADLOCK DETECTED!\n";
        detector.printWaitForGraph();
        
        auto threads = detector.getDeadlockedThreads();
        std::cout << "Deadlocked threads: " << threads.size() << "\n";
    }
    
    // Note: threads are deadlocked, this will hang
    // In real system, you'd terminate or recover
}
```

**Requirements:**
- Build wait-for graph dynamically
- Detect cycles efficiently (DFS-based)
- Thread-safe implementation
- Provide visualization of wait-for graph
- Minimize performance overhead

---

### Exercise 3.2: Deadlock-Free Dining Philosophers

Implement the dining philosophers problem with guaranteed deadlock freedom:

```cpp
#include <array>
#include <mutex>
#include <thread>

constexpr int NUM_PHILOSOPHERS = 5;

class DiningPhilosophers {
public:
    DiningPhilosophers() {
        for (auto& fork : forks) {
            fork = std::make_unique<std::mutex>();
        }
    }
    
    // Implement this - ensure no deadlock
    void philosopher(int id) {
        // Think, pick up forks, eat, put down forks
        // Repeat many times
    }
    
    void startDining() {
        std::vector<std::thread> philosophers;
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            philosophers.emplace_back(&DiningPhilosophers::philosopher, this, i);
        }
        
        for (auto& p : philosophers) {
            p.join();
        }
    }
    
private:
    std::array<std::unique_ptr<std::mutex>, NUM_PHILOSOPHERS> forks;
    
    // Your helper methods here
};

// Implement at least 3 different solutions:

// Solution 1: Resource hierarchy (odd/even)
class DiningPhilosophersV1 : public DiningPhilosophers {
    void philosopher(int id) override {
        // Implement using ordered lock acquisition
    }
};

// Solution 2: At most N-1 philosophers can attempt to eat
class DiningPhilosophersV2 {
    std::counting_semaphore<NUM_PHILOSOPHERS - 1> semaphore{NUM_PHILOSOPHERS - 1};
    // Implement
};

// Solution 3: Try-lock with backoff
class DiningPhilosophersV3 {
    // Implement using try_lock
};

// Solution 4: Waiter (coordinator) pattern
class DiningPhilosophersV4 {
    // Implement using a central coordinator
};
```

**Requirements:**
- Each solution must guarantee deadlock freedom
- Measure throughput (meals per second)
- Analyze fairness (variance in meals per philosopher)
- Compare CPU usage
- Discuss liveness and starvation issues

---

### Exercise 3.3: Lock-Free Alternative

Implement a lock-free solution for the resource transfer problem:

```cpp
#include <atomic>

class LockFreeResource {
public:
    LockFreeResource(int initial) : value(initial) {}
    
    // Atomically transfer value between resources
    static bool transfer(LockFreeResource& from, 
                        LockFreeResource& to, 
                        int amount) {
        // Implement using atomics
        // Should be wait-free or lock-free
        // Return false if insufficient funds
    }
    
    int getValue() const {
        return value.load(std::memory_order_acquire);
    }
    
private:
    std::atomic<int> value;
};

// Test with high contention
void testLockFree() {
    const int NUM_RESOURCES = 10;
    const int NUM_THREADS = 20;
    const int TRANSFERS_PER_THREAD = 10000;
    
    std::vector<LockFreeResource> resources;
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        resources.emplace_back(1000);
    }
    
    auto worker = [&](int id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, NUM_RESOURCES - 1);
        
        int successfulTransfers = 0;
        for (int i = 0; i < TRANSFERS_PER_THREAD; ++i) {
            int from = dis(gen);
            int to = dis(gen);
            if (from != to) {
                if (LockFreeResource::transfer(resources[from], 
                                               resources[to], 
                                               1)) {
                    ++successfulTransfers;
                }
            }
        }
        
        std::cout << "Thread " << id << " completed " 
                  << successfulTransfers << " transfers\n";
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << duration.count() << "ms\n";
    
    // Verify total is preserved
    int total = 0;
    for (const auto& r : resources) {
        total += r.getValue();
    }
    std::cout << "Total value: " << total 
              << " (expected: " << NUM_RESOURCES * 1000 << ")\n";
}
```

**Requirements:**
- Must be deadlock-free (no locks!)
- Should be lock-free (at least one thread makes progress)
- Ideally wait-free (all threads make progress)
- Compare performance with lock-based version
- Ensure correctness (total value preserved)

---

## Part 4: Debugging Concurrent Code

### Exercise 4.1: Deadlock in Production Code

You're given a log from a production system that has hung:

```
[Thread 1] Acquired lock A (0x7f8a4c0)
[Thread 2] Acquired lock B (0x7f8a4d0)
[Thread 1] Waiting for lock B (0x7f8a4d0)
[Thread 3] Acquired lock C (0x7f8a4e0)
[Thread 2] Waiting for lock C (0x7f8a4e0)
[Thread 3] Waiting for lock A (0x7f8a4c0)
[Thread 4] Acquired lock D (0x7f8a4f0)
[Thread 5] Acquired lock E (0x7f8a500)
```

**Tasks:**
1. Draw the wait-for graph
2. Identify the deadlock cycle
3. Which threads are deadlocked?
4. Which threads can make progress?
5. If you could release one lock, which would you choose?

**Bonus:** Write a tool to parse such logs and automatically detect deadlocks.

---

### Exercise 4.2: Debug with Helgrind/ThreadSanitizer

Use Valgrind's Helgrind or ThreadSanitizer to detect issues:

```cpp
class ServiceManager {
    std::mutex serviceMtx;
    std::mutex configMtx;
    std::map<std::string, std::string> services;
    std::map<std::string, std::string> config;
    
public:
    void registerService(const std::string& name, const std::string& endpoint) {
        std::lock_guard<std::mutex> svcLock(serviceMtx);
        services[name] = endpoint;
        
        // Update config with service info
        std::lock_guard<std::mutex> cfgLock(configMtx);
        config["last_service"] = name;
    }
    
    void updateConfig(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> cfgLock(configMtx);
        config[key] = value;
        
        // If it's a service config, update service
        if (key.find("service_") == 0) {
            std::string serviceName = key.substr(8);
            std::lock_guard<std::mutex> svcLock(serviceMtx);
            if (services.find(serviceName) != services.end()) {
                services[serviceName] = value;
            }
        }
    }
    
    std::string getServiceEndpoint(const std::string& name) {
        std::lock_guard<std::mutex> svcLock(serviceMtx);
        auto it = services.find(name);
        return (it != services.end()) ? it->second : "";
    }
};

void testServiceManager() {
    ServiceManager mgr;
    
    std::thread t1([&] {
        for (int i = 0; i < 100; ++i) {
            mgr.registerService("service" + std::to_string(i), "endpoint" + std::to_string(i));
        }
    });
    
    std::thread t2([&] {
        for (int i = 0; i < 100; ++i) {
            mgr.updateConfig("service_service" + std::to_string(i), "newpoint" + std::to_string(i));
        }
    });
    
    t1.join();
    t2.join();
}
```

**Tasks:**
1. Compile with: `g++ -g -fsanitize=thread -o test test.cpp -pthread`
2. Run and analyze output
3. Or use: `valgrind --tool=helgrind ./test`
4. Document all issues found
5. Fix all problems
6. Verify with tools again

---

### Exercise 4.3: Intermittent Deadlock

This code deadlocks rarely (1 in 1000 runs). Find why:

```cpp
class TaskQueue {
    std::mutex queueMtx;
    std::queue<std::function<void()>> tasks;
    std::mutex statsMtx;
    size_t processed = 0;
    
public:
    void addTask(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queueMtx);
        tasks.push(std::move(task));
    }
    
    bool getTask(std::function<void()>& task) {
        std::lock_guard<std::mutex> lock(queueMtx);
        if (tasks.empty()) {
            return false;
        }
        task = std::move(tasks.front());
        tasks.pop();
        return true;
    }
    
    void incrementProcessed() {
        std::lock_guard<std::mutex> lock(statsMtx);
        ++processed;
    }
    
    size_t getProcessedCount() {
        std::lock_guard<std::mutex> lock1(queueMtx);
        std::lock_guard<std::mutex> lock2(statsMtx);
        return processed;
    }
    
    void logStatus() {
        std::lock_guard<std::mutex> lock1(statsMtx);
        
        // Log stats
        std::cout << "Processed: " << processed;
        
        // Also log queue size
        std::lock_guard<std::mutex> lock2(queueMtx);
        std::cout << ", Queued: " << tasks.size() << "\n";
    }
};
```

**Tasks:**
1. Run this 10,000 times with stress testing
2. Identify the lock ordering issue
3. Why is it so rare?
4. Create a test that reliably reproduces it
5. Fix the issue

---

## Part 5: Performance Optimization

### Exercise 5.1: Measure Deadlock Prevention Overhead

Compare the overhead of different deadlock prevention strategies:

```cpp
class BenchmarkFramework {
public:
    struct Scenario {
        int numResources;
        int numThreads;
        int transfersPerThread;
        double contention; // 0.0 = no contention, 1.0 = high
    };
    
    struct Results {
        std::string strategy;
        double throughput;  // transfers per second
        double avgLatency;  // microseconds
        int deadlocks;      // if detection enabled
    };
    
    // Benchmark different strategies
    Results benchmarkNaive(const Scenario& s);
    Results benchmarkLockOrdering(const Scenario& s);
    Results benchmarkScopedLock(const Scenario& s);
    Results benchmarkTryLockBackoff(const Scenario& s);
    Results benchmarkLockFree(const Scenario& s);
    
    void runComparison() {
        std::vector<Scenario> scenarios = {
            {10, 4, 10000, 0.1},   // Low contention
            {10, 8, 10000, 0.5},   // Medium contention
            {10, 16, 10000, 0.9},  // High contention
        };
        
        for (const auto& scenario : scenarios) {
            std::cout << "\n=== Scenario: " << scenario.numThreads 
                      << " threads, " << scenario.contention 
                      << " contention ===\n";
                      
            auto r1 = benchmarkLockOrdering(scenario);
            auto r2 = benchmarkScopedLock(scenario);
            auto r3 = benchmarkTryLockBackoff(scenario);
            auto r4 = benchmarkLockFree(scenario);
            
            // Print comparison
        }
    }
};
```

**Analysis Required:**
1. Implement all benchmark methods
2. Graph results: throughput vs number of threads
3. Measure deadlock rate for naive implementation
4. Analyze CPU usage for each strategy
5. Provide recommendations based on scenarios

---

### Exercise 5.2: Optimize Lock-Ordering Strategy

```cpp
// Current implementation
class BankV1 {
    struct Account {
        std::mutex mtx;
        int balance;
        int id;
    };
    
    std::vector<Account> accounts;
    
public:
    BankV1(int numAccounts) : accounts(numAccounts) {
        for (int i = 0; i < numAccounts; ++i) {
            accounts[i].id = i;
            accounts[i].balance = 1000;
        }
    }
    
    void transfer(int from, int to, int amount) {
        Account& acc1 = accounts[std::min(from, to)];
        Account& acc2 = accounts[std::max(from, to)];
        
        std::lock_guard<std::mutex> lock1(acc1.mtx);
        std::lock_guard<std::mutex> lock2(acc2.mtx);
        
        if (accounts[from].balance >= amount) {
            accounts[from].balance -= amount;
            accounts[to].balance += amount;
        }
    }
};

// Your optimized version
class BankV2 {
    // Optimize this - reduce lock holding time, minimize contention
};
```

**Tasks:**
1. Profile V1 to find bottlenecks
2. Implement optimizations:
   - Reduce lock hold time
   - Use reader-writer locks where applicable
   - Consider lock-free balance reads
3. Benchmark both versions
4. Measure scalability (1-32 threads)
5. Document trade-offs

---

## Submission Guidelines

1. **Code:**
   - Compile with C++17 or later
   - Include build instructions
   - Provide test cases

2. **Documentation:**
   - MCQ answers with explanations
   - Analysis of deadlock scenarios
   - Performance benchmark results with graphs
   - Tool outputs (Helgrind/ThreadSanitizer)

3. **Debugging Reports:**
   - Screenshots of tool outputs
   - Before/after code comparisons
   - Explanation of fixes

4. **Performance Analysis:**
   - CSV data for all benchmarks
   - Graphs comparing strategies
   - Recommendations for each scenario type

---

## Evaluation Criteria

- **Problem Identification (25%):** Accurately identifying deadlock conditions
- **Solutions (25%):** Correct and efficient deadlock prevention
- **Debugging (20%):** Effective use of tools and techniques
- **Performance (20%):** Thorough benchmarking and analysis
- **Code Quality (10%):** Clean, well-documented implementations

---

## Additional Resources

- [Deadlock Detection Algorithms](https://en.wikipedia.org/wiki/Deadlock#Detection)
- [Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Valgrind Helgrind Manual](https://valgrind.org/docs/manual/hg-manual.html)
- [ThreadSanitizer Documentation](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)

---

Good luck debugging those deadlocks!
