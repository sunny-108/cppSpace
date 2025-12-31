# Assignment 10: Thread-Safe Data Structures

## Overview
Design and implement thread-safe data structures including queues, stacks, maps, and understand synchronization strategies.

**Target Audience:** Intermediate C++ developers (3-5 years)  
**Estimated Time:** 6-7 hours  
**Prerequisites:** Assignments 01-09

---

## Learning Objectives
- Design thread-safe queue and stack
- Implement thread-safe map/cache
- Choose appropriate synchronization primitives
- Understand lock-free vs lock-based approaches
- Handle producer-consumer patterns

---

## Part 1: Multiple Choice Questions (10 MCQs)

### Q1. A thread-safe data structure must:
A) Be fast  
B) Protect against concurrent access with proper synchronization  
C) Use atomics only  
D) Avoid locks  

**Answer:** B

### Q2. For a thread-safe queue, which is most appropriate?
A) Global mutex  
B) Mutex + condition_variable  
C) No synchronization  
D) Atomics alone  

**Answer:** B - Condition variable for blocking operations

### Q3. Fine-grained locking means:
A) One lock for entire structure  
B) Multiple locks for different parts  
C) No locks  
D) Very fast locks  

**Answer:** B

### Q4. A lock-free data structure:
A) Never uses locks  
B) Guarantees system-wide progress  
C) Is always faster  
D) Doesn't need synchronization  

**Answer:** B

### Q5. For a read-heavy cache, which is best?
A) std::mutex  
B) std::shared_mutex (reader-writer lock)  
C) No synchronization  
D) std::atomic only  

**Answer:** B

### Q6. The main challenge in thread-safe pop() is:
A) Speed  
B) Returning value and removing atomically  
C) Memory  
D) None  

**Answer:** B - Can't do both atomically without careful design

### Q7. A bounded queue should:
A) Never block  
B) Block when full (producer) and empty (consumer)  
C) Crash when full  
D) Use unlimited memory  

**Answer:** B

### Q8. Lock-free structures typically use:
A) Mutexes  
B) Condition variables  
C) Compare-and-swap (CAS) operations  
D) No synchronization  

**Answer:** C

### Q9. For thread-safe lazy initialization, use:
A) Double-checked locking  
B) std::call_once  
C) Mutex  
D) All of the above can work  

**Answer:** D - std::call_once is simplest

### Q10. The ABA problem occurs in:
A) Mutex-based structures  
B) Lock-free structures using CAS  
C) Single-threaded code  
D) Never occurs  

**Answer:** B - Covered in advanced topics

---

## Part 2: Code Review Exercises

### Exercise 2.1: Naive Thread-Safe Stack

```cpp
#include <stack>
#include <mutex>

template<typename T>
class NaiveThreadSafeStack {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.push(value);
    }
    
    T pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        T value = data_.top();  // What if empty?
        data_.pop();
        return value;  // What if copy throws?
    }
    
private:
    std::stack<T> data_;
    std::mutex mutex_;
};
```

**Questions:**
1. What happens if pop() is called on empty stack?
2. What if copying T throws during return?
3. Fix these issues

**Solution:**
```cpp
template<typename T>
class BetterThreadSafeStack {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.push(std::move(value));
    }
    
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_.empty()) {
            return std::shared_ptr<T>();  // Return nullptr
        }
        
        std::shared_ptr<T> result(
            std::make_shared<T>(std::move(data_.top()))
        );
        data_.pop();
        return result;  // No throw on copy
    }
    
    bool tryPop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (data_.empty()) {
            return false;
        }
        
        value = std::move(data_.top());
        data_.pop();
        return true;
    }
    
private:
    std::stack<T> data_;
    mutable std::mutex mutex_;
};
```

---

### Exercise 2.2: Queue Without Blocking

```cpp
template<typename T>
class SimpleQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
    }
    
    bool tryPop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
private:
    std::queue<T> queue_;
    std::mutex mutex_;
};
```

**Questions:**
1. What's missing for producer-consumer?
2. How to make pop() blocking?
3. Add condition_variable

**Solution:**
```cpp
template<typename T>
class BlockingQueue {
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cond_.notify_one();
    }
    
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
```

---

### Exercise 2.3: Thread-Safe Map

```cpp
template<typename Key, typename Value>
class ThreadSafeMap {
public:
    void insert(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
    }
    
    bool find(const Key& key, Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }
    
private:
    std::map<Key, Value> map_;
    std::mutex mutex_;
};
```

**Questions:**
1. Is this efficient for read-heavy workloads?
2. How to improve with shared_mutex?
3. What about fine-grained locking?

---

## Part 3: Implementation from Scratch

### Exercise 3.1: Bounded Blocking Queue

```cpp
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class BoundedBlockingQueue {
public:
    explicit BoundedBlockingQueue(size_t maxSize)
        : maxSize_(maxSize) {}
    
    void push(T value) {
        // Your implementation:
        // Block if queue is full
        // Notify consumers
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this] { 
            return queue_.size() < maxSize_; 
        });
        
        queue_.push(std::move(value));
        notEmpty_.notify_one();
    }
    
    T pop() {
        // Your implementation:
        // Block if queue is empty
        // Notify producers
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [this] { 
            return !queue_.empty(); 
        });
        
        T value = std::move(queue_.front());
        queue_.pop();
        notFull_.notify_one();
        return value;
    }
    
    bool tryPop(T& value, std::chrono::milliseconds timeout) {
        // Your implementation with timeout
        std::unique_lock<std::mutex> lock(mutex_);
        if (!notEmpty_.wait_for(lock, timeout, [this] { 
            return !queue_.empty(); 
        })) {
            return false;
        }
        
        value = std::move(queue_.front());
        queue_.pop();
        notFull_.notify_one();
        return true;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
private:
    std::queue<T> queue_;
    size_t maxSize_;
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
};

// Test
void testBoundedQueue() {
    BoundedBlockingQueue<int> queue(5);
    
    // Producer
    std::thread producer([&] {
        for (int i = 0; i < 20; ++i) {
            queue.push(i);
            std::cout << "Produced: " << i << "\n";
        }
    });
    
    // Consumer
    std::thread consumer([&] {
        for (int i = 0; i < 20; ++i) {
            int value = queue.pop();
            std::cout << "Consumed: " << value << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    producer.join();
    consumer.join();
}
```

---

### Exercise 3.2: Thread-Safe LRU Cache

```cpp
#include <unordered_map>
#include <list>
#include <mutex>
#include <shared_mutex>

template<typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity)
        : capacity_(capacity) {}
    
    void put(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto it = map_.find(key);
        if (it != map_.end()) {
            // Update existing
            lruList_.erase(it->second.second);
        } else if (map_.size() >= capacity_) {
            // Evict LRU
            auto lru = lruList_.back();
            map_.erase(lru);
            lruList_.pop_back();
        }
        
        // Insert at front (most recent)
        lruList_.push_front(key);
        map_[key] = {value, lruList_.begin()};
    }
    
    bool get(const Key& key, Value& value) {
        std::shared_lock<std::shared_mutex> readLock(mutex_);
        
        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }
        
        value = it->second.first;
        
        // Need to update LRU order (upgrade to exclusive lock)
        readLock.unlock();
        std::unique_lock<std::shared_mutex> writeLock(mutex_);
        
        // Move to front
        lruList_.erase(it->second.second);
        lruList_.push_front(key);
        it->second.second = lruList_.begin();
        
        return true;
    }
    
private:
    size_t capacity_;
    std::list<Key> lruList_;
    std::unordered_map<Key, std::pair<Value, typename std::list<Key>::iterator>> map_;
    mutable std::shared_mutex mutex_;
};

// Test
void testLRUCache() {
    LRUCache<int, std::string> cache(3);
    
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");
    
    std::string value;
    cache.get(1, value);
    std::cout << "1: " << value << "\n";
    
    cache.put(4, "four");  // Evicts 2 (LRU)
    
    if (!cache.get(2, value)) {
        std::cout << "2 was evicted\n";
    }
}
```

---

### Exercise 3.3: Thread-Safe Singleton Registry

```cpp
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>

template<typename T>
class Registry {
public:
    static Registry& getInstance() {
        static Registry instance;
        return instance;
    }
    
    void registerItem(const std::string& name, std::shared_ptr<T> item) {
        std::lock_guard<std::mutex> lock(mutex_);
        registry_[name] = item;
    }
    
    std::shared_ptr<T> get(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(name);
        return (it != registry_.end()) ? it->second : nullptr;
    }
    
    void remove(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        registry_.erase(name);
    }
    
private:
    Registry() = default;
    
    std::unordered_map<std::string, std::shared_ptr<T>> registry_;
    std::mutex mutex_;
};

// Usage
void testRegistry() {
    auto& registry = Registry<int>::getInstance();
    
    registry.registerItem("answer", std::make_shared<int>(42));
    
    auto value = registry.get("answer");
    if (value) {
        std::cout << "Value: " << *value << "\n";
    }
}
```

---

### Exercise 3.4: Object Pool

```cpp
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t poolSize) {
        for (size_t i = 0; i < poolSize; ++i) {
            pool_.push(std::make_unique<T>());
        }
    }
    
    std::unique_ptr<T> acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !pool_.empty(); });
        
        auto obj = std::move(pool_.front());
        pool_.pop();
        return obj;
    }
    
    void release(std::unique_ptr<T> obj) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pool_.push(std::move(obj));
        }
        cond_.notify_one();
    }
    
private:
    std::queue<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

// Test
class ExpensiveResource {
public:
    ExpensiveResource() {
        std::cout << "Creating resource\n";
    }
    
    void use() {
        std::cout << "Using resource\n";
    }
};

void testObjectPool() {
    ObjectPool<ExpensiveResource> pool(3);
    
    std::thread t1([&] {
        auto res = pool.acquire();
        res->use();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pool.release(std::move(res));
    });
    
    std::thread t2([&] {
        auto res = pool.acquire();
        res->use();
        pool.release(std::move(res));
    });
    
    t1.join();
    t2.join();
}
```

---

## Part 4: Debugging Exercises

### Exercise 4.1: Deadlock in Nested Locks

```cpp
template<typename T>
class BadMap {
    void swap(BadMap& other) {
        std::lock_guard<std::mutex> lock1(mutex_);
        std::lock_guard<std::mutex> lock2(other.mutex_);  // Deadlock risk!
        std::swap(data_, other.data_);
    }
    
    std::map<int, T> data_;
    std::mutex mutex_;
};
```

**Fix:** Use std::scoped_lock for multiple mutexes

---

### Exercise 4.2: Iterator Invalidation

```cpp
for (auto& item : threadSafeMap) {  // Lock acquired
    // Process item
    threadSafeMap.insert(newKey, newValue);  // Modifies during iteration!
}
```

**Question:** How to safely iterate and modify?

---

## Part 5: Performance Analysis

### Exercise 5.1: Compare Locking Strategies

Benchmark:
- Coarse-grained (one mutex)
- Fine-grained (multiple mutexes)
- Lock-free (atomics)

---

### Exercise 5.2: Reader-Writer Performance

Compare std::mutex vs std::shared_mutex for read-heavy workload.

---

## Submission Guidelines

Submit:
1. **answers.md** - MCQ answers
2. **code_review/** - Fixed code
3. **implementations/** - All implementations
4. **debugging/** - Bug fixes
5. **performance/** - Benchmarks

---

## Evaluation Criteria

- **Correctness (35%):** Thread-safe implementations
- **Design (30%):** Appropriate synchronization
- **Understanding (20%):** Clear explanations
- **Performance (15%):** Insightful analysis

---

## Key Takeaways

✅ Use condition_variable for blocking operations  
✅ shared_mutex for read-heavy workloads  
✅ Avoid returning references to protected data  
✅ Handle empty/full conditions properly  
✅ Choose lock granularity carefully  

---

## Common Pitfalls

❌ Returning references to protected data  
❌ Not handling empty containers  
❌ Iterator invalidation  
❌ Deadlock with multiple locks  

---

## Next Steps

You've completed the foundational concurrency series! Proceed to the advanced concurrency assignments in `/concurrency/` folder for expert-level topics.

---

## Resources

- [cppreference - thread support library](https://en.cppreference.com/w/cpp/thread)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) - Chapters 6-7
- [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/) - Item 40

Good luck!