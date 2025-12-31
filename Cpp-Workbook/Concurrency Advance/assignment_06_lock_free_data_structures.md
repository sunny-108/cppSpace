# Assignment 06: Lock-Free Data Structures

## Overview
Implement and analyze lock-free data structures including stacks, queues, and hash tables. Master techniques to handle the ABA problem, memory reclamation, and hazard pointers.

**Estimated Time:** 8-10 hours

---

## Learning Objectives
- Implement lock-free stacks and queues
- Understand and solve the ABA problem
- Implement hazard pointers for safe memory reclamation
- Design lock-free hash tables
- Compare performance with lock-based alternatives

---

## Part 1: Multiple Choice Questions (15 MCQs)

### Q1. The ABA problem in lock-free programming occurs when:
A) Two variables named A and B conflict  
B) A value changes from A to B and back to A, making CAS succeed incorrectly  
C) Atomic operations fail twice  
D) Memory ordering is wrong  

**Answer:** B

### Q2. Hazard pointers are used for:
A) Preventing the ABA problem  
B) Safe memory reclamation in lock-free structures  
C) Faster atomic operations  
D) Thread synchronization  

**Answer:** B

### Q3. A lock-free algorithm guarantees:
A) All threads make progress  
B) At least one thread makes progress  
C) No threads block  
D) Better performance than locks  

**Answer:** B

### Q4. Wait-free algorithms are stronger than lock-free because:
A) They're faster  
B) Every thread makes progress in a bounded number of steps  
C) They use no atomics  
D) They prevent deadlocks  

**Answer:** B

### Q5. The Michael-Scott queue is:
A) A lock-based queue  
B) A lock-free FIFO queue using CAS  
C) A wait-free queue  
D) A blocking queue  

**Answer:** B

### Q6. To solve the ABA problem, you can:
A) Use tagged pointers with version numbers  
B) Use stronger memory ordering  
C) Use more threads  
D) Avoid atomic operations  

**Answer:** A

### Q7. Split-ordered lists for lock-free hash tables use:
A) Multiple hash functions  
B) Recursive splitting with ordered list  
C) Lock striping  
D) Sequential consistency  

**Answer:** B

### Q8. Epoch-based reclamation:
A) Immediately deletes memory  
B) Defers deletion until all threads pass an epoch  
C) Uses reference counting  
D) Prevents memory leaks only  

**Answer:** B

### Q9. Compare-and-swap (CAS) on x86 is implemented using:
A) LOCK CMPXCHG instruction  
B) MOV instruction  
C) Software emulation  
D) XOR instruction  

**Answer:** A

### Q10. Load-linked/store-conditional (LL/SC) is:
A) The same as CAS  
B) An alternative to CAS, detects any modification  
C) Slower than CAS  
D) Only available on x86  

**Answer:** B

### Q11. In a lock-free queue, which is harder to implement?
A) Enqueue  
B) Dequeue  
C) Both are equally difficult  
D) Neither requires special handling  

**Answer:** B

### Q12. Double-word CAS (DWCAS) allows:
A) Updating two adjacent memory locations atomically  
B) Faster single-word CAS  
C) Lock-free operations on any structure  
D) Automatic ABA prevention  

**Answer:** A

### Q13. The main advantage of lock-free structures is:
A) Always faster  
B) No blocking, better for real-time systems  
C) Simpler to implement  
D) Less memory usage  

**Answer:** B

### Q14. Reference counting for memory reclamation in lock-free structures:
A) Is trivial to implement  
B) Can itself cause contention and complexity  
C) Prevents all memory leaks  
D) Is lock-free by definition  

**Answer:** B

### Q15. Obstruction-free is weaker than lock-free and means:
A) Threads never block  
B) A thread makes progress if it runs in isolation  
C) All threads make progress  
D) No progress guarantee  

**Answer:** B

---

## Part 2: Code Review Exercises

### Exercise 2.1: ABA Problem
Review this lock-free stack and identify the ABA vulnerability:

```cpp
template<typename T>
class LockFreeStack {
    struct Node {
        T data;
        Node* next;
    };
    
    std::atomic<Node*> head{nullptr};
    
public:
    void push(const T& value) {
        Node* newNode = new Node{value, nullptr};
        newNode->next = head.load();
        while (!head.compare_exchange_weak(newNode->next, newNode));
    }
    
    bool pop(T& value) {
        Node* oldHead = head.load();
        while (oldHead && 
               !head.compare_exchange_weak(oldHead, oldHead->next));
        
        if (oldHead) {
            value = oldHead->data;
            delete oldHead;  // Problem!
            return true;
        }
        return false;
    }
};
```

**Tasks:**
1. Demonstrate ABA scenario with thread interleaving
2. Show how reused memory causes incorrect behavior
3. Implement solution using tagged pointers
4. Implement solution using hazard pointers
5. Compare both solutions

---

### Exercise 2.2: Memory Reclamation Issues
```cpp
template<typename T>
class LockFreeQueue {
    struct Node {
        std::atomic<T*> data;
        std::atomic<Node*> next;
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node;
        dummy->next = nullptr;
        head = tail = dummy;
    }
    
    void enqueue(const T& value) {
        Node* node = new Node;
        node->data = new T(value);
        node->next = nullptr;
        
        while (true) {
            Node* last = tail.load();
            Node* next = last->next.load();
            
            if (last == tail.load()) {
                if (next == nullptr) {
                    if (last->next.compare_exchange_weak(next, node)) {
                        tail.compare_exchange_weak(last, node);
                        return;
                    }
                } else {
                    tail.compare_exchange_weak(last, next);
                }
            }
        }
    }
    
    bool dequeue(T& value) {
        while (true) {
            Node* first = head.load();
            Node* last = tail.load();
            Node* next = first->next.load();
            
            if (first == head.load()) {
                if (first == last) {
                    if (next == nullptr) return false;
                    tail.compare_exchange_weak(last, next);
                } else {
                    T* data = next->data.load();
                    if (head.compare_exchange_weak(first, next)) {
                        value = *data;
                        delete data;
                        delete first;  // When is this safe?
                        return true;
                    }
                }
            }
        }
    }
};
```

**Questions:**
1. When can `delete first` cause problems?
2. How many threads might access a node concurrently?
3. Design hazard pointer solution
4. Design epoch-based reclamation solution
5. Analyze performance trade-offs

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Hazard Pointers
```cpp
class HazardPointerManager {
public:
    static constexpr size_t MAX_THREADS = 100;
    static constexpr size_t MAX_HAZARDS_PER_THREAD = 4;
    
    // Acquire a hazard pointer slot
    size_t acquireHazard(void* ptr);
    
    // Release a hazard pointer
    void releaseHazard(size_t index);
    
    // Retire a pointer for later deletion
    void retirePointer(void* ptr);
    
    // Scan and delete safe pointers
    void scan();
    
private:
    struct HazardPointer {
        std::atomic<void*> ptr{nullptr};
        std::atomic<std::thread::id> owner;
    };
    
    std::array<HazardPointer, MAX_THREADS * MAX_HAZARDS_PER_THREAD> hazards;
    
    // Per-thread retired list
    thread_local static std::vector<void*> retired;
};

// Use hazard pointers with lock-free stack
template<typename T>
class SafeLockFreeStack {
    struct Node {
        T data;
        Node* next;
    };
    
    std::atomic<Node*> head{nullptr};
    HazardPointerManager hpManager;
    
public:
    void push(const T& value);
    bool pop(T& value);  // Use hazard pointers to protect node access
};
```

**Requirements:**
- Complete hazard pointer implementation
- Handle thread-local retirement lists
- Implement scanning and safe deletion
- Test with high contention
- Verify no memory leaks or use-after-free

---

### Exercise 3.2: Lock-Free Queue (Michael-Scott)
Implement the complete Michael-Scott lock-free queue:

```cpp
template<typename T>
class MichaelScottQueue {
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;
        Node() : next(nullptr) {}
        Node(const T& value) : data(std::make_shared<T>(value)), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    MichaelScottQueue();
    ~MichaelScottQueue();
    
    void enqueue(const T& value);
    bool dequeue(T& value);
    bool empty() const;
};

// Stress test
void testMSQueue();
```

**Requirements:**
- Correct implementation with proper memory orders
- Handle memory reclamation
- Test with multiple producers/consumers
- Compare with locked queue
- Measure latency distribution

---

### Exercise 3.3: Lock-Free Hash Table
```cpp
template<typename K, typename V>
class LockFreeHashTable {
    struct Node {
        K key;
        V value;
        std::atomic<Node*> next;
        size_t hash;
    };
    
    static constexpr size_t INITIAL_SIZE = 16;
    std::atomic<Node**> buckets;
    std::atomic<size_t> size;
    
public:
    LockFreeHashTable();
    
    bool insert(const K& key, const V& value);
    bool find(const K& key, V& value);
    bool erase(const K& key);
    
    void resize();  // Lock-free resizing!
    
private:
    size_t hashKey(const K& key) const;
};
```

**Requirements:**
- Lock-free insert/find/erase
- Implement lock-free resize
- Handle memory reclamation
- Benchmark vs std::unordered_map with mutex
- Test scalability

---

## Part 4: Debugging & Analysis

### Exercise 4.1: Race Condition in Lock-Free Code
Debug this buggy lock-free counter:

```cpp
class LockFreeCounter {
    struct Node {
        int value;
        Node* next;
    };
    
    std::atomic<Node*> head{nullptr};
    
public:
    void add(int value) {
        Node* newNode = new Node{value, head.load()};
        while (!head.compare_exchange_weak(newNode->next, newNode));
    }
    
    int sum() {
        int total = 0;
        Node* current = head.load();
        while (current) {
            total += current->value;
            current = current->next;  // Dangerous!
        }
        return total;
    }
};
```

**Tasks:**
1. Identify the race condition
2. Create a test that exposes it
3. Fix using hazard pointers
4. Verify with ThreadSanitizer

---

## Part 5: Performance Optimization

### Exercise 5.1: Lock-Free vs Locked Comparison
Comprehensive benchmark framework:

```cpp
class DataStructureBenchmark {
public:
    struct Config {
        size_t numThreads;
        size_t operations;
        double readRatio;
        size_t contentionLevel;
    };
    
    struct Results {
        std::string name;
        double throughput;
        double latency;
        std::vector<double> latencyPercentiles;  // 50th, 95th, 99th
    };
    
    // Test various structures
    Results benchmarkLockFreeQueue(const Config& cfg);
    Results benchmarkLockedQueue(const Config& cfg);
    Results benchmarkLockFreeStack(const Config& cfg);
    Results benchmarkLockedStack(const Config& cfg);
    
    void runFullComparison();
};
```

**Analysis Required:**
1. Implement all benchmarks
2. Vary thread count 1-32
3. Test different contention levels
4. Measure latency distribution
5. Provide detailed recommendations

---

## Submission Guidelines
- Full implementations with tests
- Memory leak checks (Valgrind)
- Thread safety verification (ThreadSanitizer)
- Performance analysis with graphs
- Detailed explanations of ABA solutions

---

## Evaluation Criteria
- **Correctness (40%):** No races, proper memory reclamation
- **ABA Handling (20%):** Correct solutions to ABA problem
- **Performance (20%):** Efficient implementations
- **Analysis (20%):** Thorough benchmarking and understanding

---

## Resources
- "The Art of Multiprocessor Programming" by Herlihy & Shavit
- [Lock-Free Data Structures - Preshing](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
- [Hazard Pointers Paper](https://www.research.ibm.com/people/m/michael/ieeetpds-2004.pdf)
- [Michael-Scott Queue Paper](https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf)

Good luck!
