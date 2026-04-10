# Chapter 4: Advanced Level Programming Assignments
## Synchronizing Concurrent Operations

These assignments tackle complex async patterns, performance optimization, and production-grade synchronization suitable for experienced developers.

---

## Assignment 1: Lock-Free Work-Stealing Thread Pool

### Objective
Implement an advanced thread pool with work-stealing algorithm for optimal load balancing, using futures and atomics.

### Real-World Scenario
Build a high-performance task execution engine similar to Intel TBB or Java's ForkJoinPool, used in video processing pipelines or scientific computing.

### Requirements

1. **Work-Stealing Architecture:**
   - Each thread has its own task queue (deque)
   - Threads steal tasks from other threads when idle
   - Lock-free or mostly lock-free implementation
   - Support for task dependencies

2. **Advanced Features:**
   - Automatic work distribution
   - Priority-based task execution
   - Task graphs (tasks depend on other tasks)
   - Adaptive thread count based on load
   - Statistics tracking (steals, idle time, throughput)

3. **Template Design:**
```cpp
template<typename ResultType>
class Task {
public:
    virtual ResultType execute() = 0;
    virtual std::vector<std::shared_ptr<Task>> dependencies() const;
    virtual int priority() const;
};

class WorkStealingThreadPool {
    // Your implementation
};
```

### Implementation Hints

**High-Level Architecture:**
```cpp
class WorkStealingThreadPool {
private:
    struct WorkerThread {
        std::thread thread;
        std::deque<std::packaged_task<void()>> local_queue;
        std::mutex queue_mutex;
        std::atomic<size_t> tasks_executed{0};
        std::atomic<size_t> tasks_stolen{0};
    };
    
    std::vector<WorkerThread> workers;
    std::atomic<bool> done{false};
    std::atomic<size_t> active_tasks{0};
    
    void worker_loop(size_t worker_id) {
        while (!done.load(std::memory_order_relaxed)) {
            // 1. Try to get task from own queue
            // 2. If empty, try to steal from others
            // 3. Execute task
            // 4. Update statistics
        }
    }
    
    bool try_steal_task(size_t thief_id, std::packaged_task<void()>& task) {
        // Random or round-robin selection of victim
        // Try to steal from tail of victim's queue
        // Return true if successful
    }
    
public:
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        // Determine least loaded worker
        // Add task to its queue
        // Return future
    }
    
    template<typename F>
    auto submit_with_deps(F&& f, std::vector<std::future<void>> deps)
        -> std::future<typename std::result_of<F()>::type>
    {
        // Wait for dependencies in separate task
        // Then execute main task
    }
};
```

**Work Stealing Logic:**
```cpp
bool try_steal_task(size_t thief_id, std::packaged_task<void()>& task) {
    // Try random victims to avoid contention
    std::vector<size_t> victims = generate_random_order(workers.size());
    
    for (size_t victim_id : victims) {
        if (victim_id == thief_id) continue;
        
        std::unique_lock<std::mutex> lock(workers[victim_id].queue_mutex, 
                                          std::try_to_lock);
        if (!lock.owns_lock()) continue;  // Victim is busy
        
        auto& victim_queue = workers[victim_id].local_queue;
        if (victim_queue.empty()) continue;
        
        // Steal from back (FIFO for stealer, LIFO for owner)
        task = std::move(victim_queue.back());
        victim_queue.pop_back();
        
        workers[thief_id].tasks_stolen.fetch_add(1);
        return true;
    }
    
    return false;
}
```

**Key Challenges:**
- Minimize lock contention between workers
- Proper load balancing strategy
- Handle dependencies without deadlocks
- Efficient idle worker management

### Testing Scenarios

1. **Load Balancing:**
   - Submit 1000 tasks of varying duration
   - Verify even distribution across threads
   - Measure steal count per worker

2. **Task Dependencies:**
   - Create DAG of dependent tasks
   - Verify correct execution order
   - No deadlocks

3. **Performance:**
   - Compare with standard thread pool
   - Measure overhead of work stealing
   - Test scalability with thread count

### Expected Metrics
```
=== Work-Stealing Thread Pool Statistics ===
Workers: 8
Total tasks executed: 10,000
Total time: 12.4s
Throughput: 806 tasks/sec

Per-Worker Statistics:
Worker 0: 1,234 executed, 0 stolen
Worker 1: 1,189 executed, 45 stolen from others
Worker 2: 1,256 executed, 0 stolen
Worker 3: 1,198 executed, 23 stolen from others
...

Load balance factor: 0.94 (1.0 = perfect balance)
Average steal latency: 15µs
```

---

## Assignment 2: Distributed Future with Remote Execution

### Objective
Extend futures to work across network boundaries, allowing async execution on remote machines.

### Real-World Scenario
Build a distributed computing system similar to Ray or Dask, where computations can be offloaded to remote nodes and results retrieved via futures.

### Requirements

1. **Network-Aware Futures:**
   - Serialize tasks and send over network
   - Execute on remote workers
   - Return results via network
   - Handle network failures gracefully

2. **Architecture Components:**
   - Master node (submits tasks)
   - Worker nodes (execute tasks)
   - Result cache (avoid recomputation)
   - Task serialization/deserialization

3. **Protocol Design:**
```cpp
struct TaskMessage {
    uint64_t task_id;
    std::vector<uint8_t> serialized_function;
    std::vector<uint8_t> serialized_args;
};

struct ResultMessage {
    uint64_t task_id;
    bool success;
    std::vector<uint8_t> serialized_result;
    std::string error_message;
};
```

### Implementation Hints

**Master Node:**
```cpp
class DistributedExecutor {
private:
    struct RemoteTask {
        uint64_t task_id;
        std::promise<std::vector<uint8_t>> result_promise;
        std::chrono::steady_clock::time_point submitted_time;
        std::string assigned_worker;
    };
    
    std::map<uint64_t, RemoteTask> pending_tasks;
    std::vector<std::string> worker_addresses;
    std::mutex tasks_mutex;
    
    // Network thread receiving results
    void result_receiver_thread() {
        while (running) {
            ResultMessage msg = receive_from_network();
            
            std::lock_guard<std::mutex> lock(tasks_mutex);
            auto it = pending_tasks.find(msg.task_id);
            if (it != pending_tasks.end()) {
                if (msg.success) {
                    it->second.result_promise.set_value(msg.serialized_result);
                } else {
                    it->second.result_promise.set_exception(
                        std::make_exception_ptr(
                            std::runtime_error(msg.error_message)
                        )
                    );
                }
                pending_tasks.erase(it);
            }
        }
    }
    
public:
    template<typename F, typename... Args>
    auto submit_remote(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using result_type = typename std::result_of<F(Args...)>::type;
        
        // 1. Serialize function and arguments
        std::vector<uint8_t> serialized_func = serialize(f);
        std::vector<uint8_t> serialized_args = serialize(std::make_tuple(args...));
        
        // 2. Create task and promise
        uint64_t task_id = generate_task_id();
        RemoteTask task;
        task.task_id = task_id;
        task.submitted_time = std::chrono::steady_clock::now();
        
        std::future<std::vector<uint8_t>> serialized_future = 
            task.result_promise.get_future();
        
        // 3. Select worker and send task
        std::string worker = select_least_loaded_worker();
        task.assigned_worker = worker;
        
        TaskMessage msg{task_id, serialized_func, serialized_args};
        send_to_worker(worker, msg);
        
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            pending_tasks[task_id] = std::move(task);
        }
        
        // 4. Return future that deserializes result
        return std::async(std::launch::deferred, [serialized_future]() mutable {
            std::vector<uint8_t> data = serialized_future.get();
            return deserialize<result_type>(data);
        });
    }
};
```

**Worker Node:**
```cpp
class RemoteWorker {
private:
    void task_executor_thread() {
        while (running) {
            TaskMessage msg = receive_task_from_master();
            
            try {
                // Deserialize and execute
                auto function = deserialize_function(msg.serialized_function);
                auto args = deserialize_args(msg.serialized_args);
                
                auto result = apply(function, args);
                
                // Serialize and send back
                std::vector<uint8_t> serialized_result = serialize(result);
                ResultMessage response{msg.task_id, true, serialized_result, ""};
                send_to_master(response);
                
            } catch (const std::exception& e) {
                ResultMessage response{msg.task_id, false, {}, e.what()};
                send_to_master(response);
            }
        }
    }
    
public:
    void start() {
        // Start executor threads
        // Register with master
    }
};
```

**Key Challenges:**
- Function serialization (may need to use strings/identifiers instead)
- Network error handling and retry logic
- Task timeout and resubmission
- Worker failure detection
- Result caching for idempotent tasks

### Testing Scenarios

1. **Basic Remote Execution:**
   - Submit simple tasks to remote workers
   - Verify correct results
   - Measure network overhead

2. **Fault Tolerance:**
   - Kill worker during execution
   - Verify task resubmission
   - No lost results

3. **Load Balancing:**
   - Multiple workers, varying loads
   - Verify even distribution
   - Dynamic worker addition/removal

### Expected Output
```
=== Distributed Executor ===
Master: localhost:8000
Workers: 
  - worker1:8001 (4 cores, 0% load)
  - worker2:8002 (8 cores, 0% load)
  - worker3:8003 (4 cores, 0% load)

Submitting 100 remote tasks...

Task #1: Assigned to worker2, executing...
Task #2: Assigned to worker3, executing...
Task #3: Assigned to worker1, executing...
...

Worker2 failed! Reassigning 12 tasks...

Results collected: 100/100
Total time: 45.2s
Network overhead: ~8%
Average task latency: 452ms
```

---

## Assignment 3: Reactive Async Pipeline with Back-Pressure

### Objective
Implement a reactive stream processing pipeline with automatic back-pressure handling, similar to RxCpp or reactive streams.

### Real-World Scenario
Build a real-time data processing pipeline for IoT sensor data, where producers can overwhelm consumers if not controlled.

### Requirements

1. **Reactive Stream Components:**
   - Observable (data source)
   - Operators (transform, filter, buffer)
   - Subscriber (data sink)
   - Back-pressure strategy

2. **Back-Pressure Mechanisms:**
   - Buffer with limit
   - Drop oldest/newest
   - Block producer
   - Sample/throttle

3. **Pipeline Operators:**
```cpp
template<typename T>
class Observable {
public:
    Observable<T> map(std::function<T(T)> transform);
    Observable<T> filter(std::function<bool(T)> predicate);
    Observable<T> buffer(size_t count);
    Observable<T> throttle(std::chrono::milliseconds interval);
    void subscribe(std::function<void(T)> on_next,
                   std::function<void(std::exception_ptr)> on_error,
                   std::function<void()> on_complete);
};
```

### Implementation Hints

**Core Architecture:**
```cpp
template<typename T>
class Observable {
private:
    struct Subscriber {
        std::function<void(T)> on_next;
        std::function<void(std::exception_ptr)> on_error;
        std::function<void()> on_complete;
        std::atomic<size_t> pending_count{0};
        size_t max_pending;
        BackPressureStrategy strategy;
    };
    
    std::function<void(Subscriber&)> producer_func;
    
public:
    template<typename Func>
    static Observable<T> create(Func&& producer) {
        Observable<T> obs;
        obs.producer_func = [producer](Subscriber& sub) {
            try {
                producer([&sub](const T& value) {
                    // Apply back-pressure
                    while (sub.pending_count.load() >= sub.max_pending) {
                        if (sub.strategy == BackPressureStrategy::BLOCK) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        } else if (sub.strategy == BackPressureStrategy::DROP_OLDEST) {
                            // Drop implementation
                            break;
                        }
                    }
                    
                    sub.pending_count.fetch_add(1);
                    sub.on_next(value);
                });
                sub.on_complete();
            } catch (...) {
                sub.on_error(std::current_exception());
            }
        };
        return obs;
    }
    
    template<typename MapFunc>
    Observable<typename std::result_of<MapFunc(T)>::type> 
    map(MapFunc&& func) {
        using U = typename std::result_of<MapFunc(T)>::type;
        
        return Observable<U>::create([self = *this, func](auto emit) {
            self.subscribe(
                [emit, func](const T& value) {
                    emit(func(value));
                },
                [](std::exception_ptr) {},
                []() {}
            );
        });
    }
    
    Observable<std::vector<T>> buffer(size_t count) {
        return Observable<std::vector<T>>::create([self = *this, count](auto emit) {
            std::vector<T> buffer;
            std::mutex mtx;
            
            self.subscribe(
                [&buffer, &mtx, count, emit](const T& value) {
                    std::lock_guard<std::mutex> lock(mtx);
                    buffer.push_back(value);
                    if (buffer.size() >= count) {
                        emit(buffer);
                        buffer.clear();
                    }
                },
                [](std::exception_ptr) {},
                [&buffer, emit]() {
                    if (!buffer.empty()) {
                        emit(buffer);
                    }
                }
            );
        });
    }
};
```

**Back-Pressure Strategies:**
```cpp
enum class BackPressureStrategy {
    BLOCK,          // Block producer until consumer catches up
    DROP_OLDEST,    // Drop oldest items in buffer
    DROP_NEWEST,    // Drop newest items
    ERROR          // Throw exception when buffer full
};

class BackPressureController {
private:
    std::queue<std::future<void>> pending_operations;
    size_t max_pending;
    std::mutex mtx;
    std::condition_variable cv;
    
public:
    template<typename Func>
    void submit_with_backpressure(Func&& func, BackPressureStrategy strategy) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait if too many pending
        if (pending_operations.size() >= max_pending) {
            switch (strategy) {
                case BackPressureStrategy::BLOCK:
                    cv.wait(lock, [this]{ 
                        return pending_operations.size() < max_pending; 
                    });
                    break;
                    
                case BackPressureStrategy::DROP_OLDEST:
                    pending_operations.pop();
                    break;
                    
                case BackPressureStrategy::ERROR:
                    throw std::runtime_error("Back-pressure limit exceeded");
            }
        }
        
        // Submit operation
        auto fut = std::async(std::launch::async, std::forward<Func>(func));
        pending_operations.push(std::move(fut));
        
        // Clean completed operations
        while (!pending_operations.empty()) {
            auto& front = pending_operations.front();
            if (front.wait_for(std::chrono::seconds(0)) == 
                std::future_status::ready) {
                pending_operations.pop();
            } else {
                break;
            }
        }
        
        cv.notify_all();
    }
};
```

**Key Concepts:**
- Lazy evaluation (operations don't run until subscribe)
- Composable operators
- Automatic thread management
- Graceful overload handling

### Testing Scenarios

1. **Fast Producer, Slow Consumer:**
   - Producer: 1000 items/sec
   - Consumer: 100 items/sec
   - Different back-pressure strategies
   - Verify no memory explosion

2. **Complex Pipeline:**
   - Chain multiple operators
   - Verify correct transformation
   - Measure latency through pipeline

3. **Error Handling:**
   - Inject errors in pipeline
   - Verify propagation to subscriber
   - Graceful cleanup

### Expected Output
```
=== Reactive Pipeline ===
Strategy: BLOCK with max 100 pending

Pipeline: 
  Source → Map(x*2) → Filter(x>10) → Buffer(10) → Subscriber

Producer rate: 1000 items/sec
Consumer rate: 200 items/sec

[00:00] Buffering... (100 pending)
[00:02] Back-pressure activated - producer blocked
[00:04] Consumer catching up... (50 pending)
[00:06] Normal operation resumed
[00:10] Complete - 10,000 items processed

Dropped: 0
Blocked time: 2.3s
Average latency: 150ms
```

---

## Assignment 4: Continuation-Based Async Framework

### Objective
Build a framework supporting async/await-style continuation chains without language support, using futures and callbacks.

### Real-World Scenario
Create a database query library with chainable async operations, similar to Promise.then() in JavaScript or C++20 coroutines.

### Requirements

1. **Continuation Support:**
   - Chain async operations
   - Automatic error propagation
   - Thread pool for execution
   - Cancellation tokens

2. **API Design:**
```cpp
auto result = async_query("SELECT * FROM users")
    .then([](QueryResult res) {
        return async_process(res);
    })
    .then([](ProcessedData data) {
        return async_save(data);
    })
    .catch_error([](std::exception_ptr ex) {
        // Handle error
    })
    .get();  // Block and get final result
```

3. **Features:**
   - Automatic chaining
   - Error short-circuiting
   - Parallel composition (when_all, when_any)
   - Timeout support

### Implementation Hints

**Core ContinuableFuture Class:**
```cpp
template<typename T>
class ContinuableFuture {
private:
    std::shared_ptr<std::promise<T>> promise_ptr;
    std::shared_future<T> future;
    std::shared_ptr<ThreadPool> thread_pool;
    std::shared_ptr<CancellationToken> cancellation;
    
public:
    ContinuableFuture(std::future<T> fut, 
                      std::shared_ptr<ThreadPool> pool)
        : future(fut.share())
        , thread_pool(pool)
        , cancellation(std::make_shared<CancellationToken>())
    {}
    
    template<typename Func>
    auto then(Func&& func) 
        -> ContinuableFuture<typename std::result_of<Func(T)>::type>
    {
        using result_type = typename std::result_of<Func(T)>::type;
        
        auto prom = std::make_shared<std::promise<result_type>>();
        auto fut = prom->get_future();
        
        // Submit continuation to thread pool
        thread_pool->submit([
            prev_future = this->future,
            func = std::forward<Func>(func),
            prom,
            cancel = this->cancellation
        ]() mutable {
            try {
                if (cancel->is_cancelled()) {
                    prom->set_exception(
                        std::make_exception_ptr(
                            std::runtime_error("Cancelled")
                        )
                    );
                    return;
                }
                
                T value = prev_future.get();
                
                if (cancel->is_cancelled()) {
                    prom->set_exception(
                        std::make_exception_ptr(
                            std::runtime_error("Cancelled")
                        )
                    );
                    return;
                }
                
                result_type result = func(value);
                prom->set_value(std::move(result));
                
            } catch (...) {
                prom->set_exception(std::current_exception());
            }
        });
        
        return ContinuableFuture<result_type>(std::move(fut), thread_pool);
    }
    
    template<typename ErrorFunc>
    ContinuableFuture<T> catch_error(ErrorFunc&& handler) {
        auto prom = std::make_shared<std::promise<T>>();
        auto fut = prom->get_future();
        
        thread_pool->submit([
            prev_future = this->future,
            handler = std::forward<ErrorFunc>(handler),
            prom
        ]() {
            try {
                T value = prev_future.get();
                prom->set_value(std::move(value));
            } catch (...) {
                try {
                    handler(std::current_exception());
                    // Optionally set a default value or rethrow
                    prom->set_exception(std::current_exception());
                } catch (...) {
                    prom->set_exception(std::current_exception());
                }
            }
        });
        
        return ContinuableFuture<T>(std::move(fut), thread_pool);
    }
    
    T get() {
        return future.get();
    }
    
    void cancel() {
        cancellation->cancel();
    }
};
```

**Parallel Composition:**
```cpp
template<typename... Futures>
auto when_all(Futures&&... futures) {
    using tuple_type = std::tuple<typename Futures::value_type...>;
    
    auto prom = std::make_shared<std::promise<tuple_type>>();
    auto fut = prom->get_future();
    
    // Launch async task to wait for all
    std::async(std::launch::async, [
        prom,
        ... futs = std::forward<Futures>(futures)
    ]() mutable {
        try {
            tuple_type results = std::make_tuple(futs.get()...);
            prom->set_value(std::move(results));
        } catch (...) {
            prom->set_exception(std::current_exception());
        }
    });
    
    return ContinuableFuture<tuple_type>(std::move(fut), /* thread_pool */);
}

template<typename... Futures>
auto when_any(Futures&&... futures) {
    // Return first completed future
    // Cancel or ignore others
}
```

**Key Challenges:**
- Type deduction through continuation chains
- Proper exception propagation
- Avoiding unnecessary copies
- Cancellation propagation through chain

### Testing Scenarios

1. **Simple Chain:**
   - 3-4 chained operations
   - Verify correct execution order
   - Verify final result

2. **Error Propagation:**
   - Inject error in middle of chain
   - Verify short-circuiting
   - Verify error handler called

3. **Parallel Composition:**
   - Multiple independent futures
   - when_all: wait for all
   - when_any: return first
   - Verify correct behavior

4. **Cancellation:**
   - Long-running chain
   - Cancel mid-execution
   - Verify subsequent operations skipped

### Expected Output
```
=== Continuation Chain Execution ===

Step 1: Fetching user data... [DONE] (150ms)
Step 2: Processing records... [DONE] (320ms)
Step 3: Calculating statistics... [DONE] (80ms)
Step 4: Saving results... [DONE] (200ms)

Total time: 750ms
Result: { users: 1500, processed: 1500, saved: true }

=== Error Handling Test ===

Step 1: Fetching user data... [DONE]
Step 2: Processing records... [ERROR] - Invalid format
Error handler: Caught exception, using fallback data
Step 3: Calculating statistics... [DONE] (with fallback)

Result: { users: 0, processed: 0, saved: false }

=== Parallel Composition ===

Query1: SELECT users... [DONE] (300ms)
Query2: SELECT orders... [DONE] (450ms)
Query3: SELECT products... [DONE] (200ms)

when_all completed: 450ms (max of all)
All results received

when_any completed: 200ms (first to finish)
Result from fastest query
```

---

## Assignment 5: Adaptive Concurrent Algorithm Library

### Objective
Implement a library of common algorithms (sort, reduce, scan) that automatically adapt execution strategy based on input size and available resources.

### Real-World Scenario
Build a high-performance computing library similar to Intel TBB or HPX, where algorithms automatically parallelize when beneficial.

### Requirements

1. **Adaptive Algorithms:**
   - parallel_sort
   - parallel_reduce
   - parallel_scan (prefix sum)
   - parallel_for_each
   - parallel_find

2. **Adaptation Strategy:**
   - Measure input size
   - Check CPU load and available cores
   - Decide: sequential vs parallel
   - Choose chunk size dynamically
   - Fall back to sequential for small inputs

3. **Performance Monitoring:**
   - Track execution time
   - Measure speedup
   - Adjust thresholds based on history

### Implementation Hints

**Adaptive Execution Framework:**
```cpp
class AdaptiveExecutor {
private:
    static constexpr size_t MIN_PARALLEL_SIZE = 10000;
    static constexpr size_t CHUNK_SIZE_FACTOR = 1000;
    
    std::shared_ptr<ThreadPool> thread_pool;
    std::atomic<double> avg_sequential_time{0.0};
    std::atomic<double> avg_parallel_time{0.0};
    size_t sample_count = 0;
    
    bool should_parallelize(size_t input_size) {
        // Decision factors:
        // 1. Input size threshold
        if (input_size < MIN_PARALLEL_SIZE) return false;
        
        // 2. Available hardware parallelism
        if (std::thread::hardware_concurrency() <= 1) return false;
        
        // 3. Historical performance (if available)
        if (sample_count > 10) {
            double speedup = avg_sequential_time.load() / 
                           avg_parallel_time.load();
            if (speedup < 1.5) return false;  // Not worth overhead
        }
        
        // 4. Current system load (optional)
        // Could check CPU usage here
        
        return true;
    }
    
public:
    template<typename RandomIt, typename Compare>
    void adaptive_sort(RandomIt first, RandomIt last, Compare comp) {
        size_t size = std::distance(first, last);
        
        if (should_parallelize(size)) {
            parallel_sort(first, last, comp);
        } else {
            std::sort(first, last, comp);
        }
    }
};
```

**Parallel Sort Implementation:**
```cpp
template<typename RandomIt, typename Compare>
void parallel_sort(RandomIt first, RandomIt last, Compare comp) {
    size_t size = std::distance(first, last);
    
    if (size < SEQUENTIAL_THRESHOLD) {
        std::sort(first, last, comp);
        return;
    }
    
    // Choose pivot and partition
    auto pivot = *std::next(first, size / 2);
    auto middle1 = std::partition(first, last, 
        [&](const auto& elem) { return comp(elem, pivot); });
    auto middle2 = std::partition(middle1, last,
        [&](const auto& elem) { return !comp(pivot, elem); });
    
    // Sort partitions in parallel
    auto left_future = std::async(std::launch::async,
        [first, middle1, comp]() {
            parallel_sort(first, middle1, comp);
        });
    
    parallel_sort(middle2, last, comp);  // Right in this thread
    left_future.wait();
}
```

**Parallel Reduce:**
```cpp
template<typename RandomIt, typename T, typename BinaryOp>
T parallel_reduce(RandomIt first, RandomIt last, T init, BinaryOp op) {
    size_t size = std::distance(first, last);
    size_t num_threads = std::thread::hardware_concurrency();
    
    if (size < MIN_PARALLEL_SIZE || num_threads <= 1) {
        return std::accumulate(first, last, init, op);
    }
    
    size_t chunk_size = size / num_threads;
    std::vector<std::future<T>> futures;
    
    auto chunk_start = first;
    for (size_t i = 0; i < num_threads - 1; ++i) {
        auto chunk_end = std::next(chunk_start, chunk_size);
        
        futures.push_back(std::async(std::launch::async,
            [chunk_start, chunk_end, init, op]() {
                return std::accumulate(chunk_start, chunk_end, init, op);
            }));
        
        chunk_start = chunk_end;
    }
    
    // Process last chunk in this thread
    T result = std::accumulate(chunk_start, last, init, op);
    
    // Combine results
    for (auto& fut : futures) {
        result = op(result, fut.get());
    }
    
    return result;
}
```

**Parallel Scan (Prefix Sum):**
```cpp
template<typename RandomIt, typename T, typename BinaryOp>
void parallel_scan(RandomIt first, RandomIt last, 
                   RandomIt d_first, T init, BinaryOp op) {
    size_t size = std::distance(first, last);
    
    if (size < MIN_PARALLEL_SIZE) {
        std::partial_sum(first, last, d_first, op);
        return;
    }
    
    size_t num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = size / num_threads;
    
    // Phase 1: Compute local scans
    std::vector<std::future<T>> local_sums;
    std::vector<RandomIt> chunk_starts;
    
    auto chunk_start = first;
    auto out_start = d_first;
    
    for (size_t i = 0; i < num_threads; ++i) {
        auto chunk_end = (i == num_threads - 1) ? last :
                         std::next(chunk_start, chunk_size);
        auto out_end = std::next(out_start, std::distance(chunk_start, chunk_end));
        
        chunk_starts.push_back(out_start);
        
        local_sums.push_back(std::async(std::launch::async,
            [chunk_start, chunk_end, out_start, out_end, init, op]() {
                std::partial_sum(chunk_start, chunk_end, out_start, op);
                return *std::prev(out_end);  // Last element
            }));
        
        chunk_start = chunk_end;
        out_start = out_end;
    }
    
    // Phase 2: Compute prefix of chunk sums
    std::vector<T> chunk_sums;
    for (auto& fut : local_sums) {
        chunk_sums.push_back(fut.get());
    }
    
    std::vector<T> chunk_prefixes(chunk_sums.size());
    std::partial_sum(chunk_sums.begin(), chunk_sums.end(), 
                     chunk_prefixes.begin(), op);
    
    // Phase 3: Add chunk prefix to each element
    std::vector<std::future<void>> adjust_futures;
    for (size_t i = 1; i < num_threads; ++i) {
        adjust_futures.push_back(std::async(std::launch::async,
            [start = chunk_starts[i], 
             end = (i == num_threads - 1) ? d_first + size : chunk_starts[i+1],
             prefix = chunk_prefixes[i-1],
             op]() {
                for (auto it = start; it != end; ++it) {
                    *it = op(*it, prefix);
                }
            }));
    }
    
    for (auto& fut : adjust_futures) {
        fut.wait();
    }
}
```

**Key Challenges:**
- Dynamic threshold tuning
- Minimizing parallelization overhead
- Load balancing across cores
- Cache-friendly data access patterns
- Handling non-commutative operations

### Testing Scenarios

1. **Small Input:**
   - Array of 100 elements
   - Verify sequential execution chosen
   - Measure overhead

2. **Large Input:**
   - Array of 10,000,000 elements
   - Verify parallel execution
   - Measure speedup

3. **Varying Load:**
   - Run with different system loads
   - Verify adaptive behavior
   - Track decision-making

4. **Correctness:**
   - Compare with std algorithms
   - Test with various data types
   - Edge cases (empty, single element)

### Expected Output
```
=== Adaptive Algorithm Library ===

Test 1: Small Array (100 elements)
Decision: Sequential (below threshold)
Time: 0.05ms
Speedup: N/A

Test 2: Large Array (10M elements)
Decision: Parallel (8 threads)
Sequential time: 1,234ms
Parallel time: 189ms
Speedup: 6.5x
Efficiency: 81%

Test 3: Parallel Reduce
Input: 50M integers
Operation: Sum
Sequential: 2,145ms
Parallel: 298ms
Speedup: 7.2x

Test 4: Parallel Scan
Input: 20M integers
Operation: Prefix sum
Sequential: 1,876ms
Parallel: 412ms
Speedup: 4.6x

=== Adaptation Statistics ===
Total operations: 1,000
Sequential chosen: 234 (23.4%)
Parallel chosen: 766 (76.6%)
Average speedup (parallel): 5.8x
```

---

## Evaluation Criteria

1. **Performance (30%)**
   - Actual speedup achieved
   - Scalability with core count
   - Low overhead for small inputs

2. **Correctness (25%)**
   - Thread-safe implementation
   - Correct results
   - No race conditions or deadlocks

3. **Design Quality (20%)**
   - Clean abstractions
   - Extensibility
   - Code organization

4. **Robustness (15%)**
   - Error handling
   - Resource management
   - Graceful degradation

5. **Innovation (10%)**
   - Novel optimizations
   - Creative solutions
   - Advanced techniques

---

## Bonus Challenges

1. **Implement cache-line padding** to avoid false sharing
2. **Add NUMA awareness** for multi-socket systems
3. **Implement lock-free data structures** where possible
4. **Create visualization tools** for pipeline/task execution
5. **Add profiling hooks** for performance analysis
6. **Implement custom allocators** for better memory locality

---

## Performance Optimization Techniques

1. **Work Stealing:** Minimize idle time
2. **Cache Locality:** Arrange data for sequential access
3. **Lock-Free:** Reduce synchronization overhead
4. **Batch Processing:** Amortize task submission cost
5. **Adaptive Chunking:** Balance load dynamically
6. **SIMD Operations:** Vectorize where possible
7. **Memory Pooling:** Reduce allocation overhead

---

## Testing Tools

- **Thread Sanitizer:** Detect race conditions
- **Valgrind/Helgrind:** Memory and threading errors
- **perf/vtune:** Performance profiling
- **Custom benchmarking framework:** Measure speedup
- **Stress testing:** High contention scenarios

---

## Learning Objectives

After completing these assignments:
- ✅ Master advanced synchronization patterns
- ✅ Implement production-grade concurrent systems
- ✅ Optimize for performance and scalability
- ✅ Handle complex error scenarios
- ✅ Design adaptive algorithms
- ✅ Build reusable concurrent libraries

These are challenging assignments that will push your concurrent programming skills to expert level! 🚀
