# Chapter 4: Medium Level Programming Assignments
## Synchronizing Concurrent Operations

These assignments focus on fundamental async operations and synchronization patterns suitable for intermediate developers.

---

## Assignment 1: Thread-Safe Message Queue with Condition Variables

### Objective
Implement a thread-safe message queue that allows multiple producers and consumers to communicate efficiently using condition variables.

### Real-World Scenario
Build a chat server backend where multiple clients send messages (producers) and multiple server threads process these messages (consumers).

### Requirements

1. **MessageQueue Class:**
   - Store messages with timestamps
   - Support multiple producers and consumers
   - Block when queue is empty (consumers wait)
   - Block when queue is full (producers wait)
   - Graceful shutdown mechanism

2. **Message Structure:**
```cpp
struct Message {
    int sender_id;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};
```

3. **Key Operations:**
   - `void send_message(Message msg)` - Blocks if queue full
   - `Message receive_message()` - Blocks if queue empty
   - `bool try_receive(Message& msg, milliseconds timeout)` - Try with timeout
   - `void shutdown()` - Signal all threads to stop
   - `size_t pending_count()` - Number of pending messages

### Implementation Hints

**Class Structure:**
```cpp
class MessageQueue {
private:
    std::queue<Message> messages;
    std::mutex mtx;
    std::condition_variable cv_empty;  // For consumers
    std::condition_variable cv_full;   // For producers
    size_t capacity;
    bool shutdown_flag;
    
public:
    // Your methods here
};
```

**Key Considerations:**
- Use `wait()` with predicates to avoid spurious wakeups
- Notify appropriate condition variables after state changes
- Handle shutdown gracefully (wake all waiting threads)
- Consider using `notify_one()` vs `notify_all()` carefully

### Testing Scenarios

1. **Basic Operation:**
   - 3 producers sending 100 messages each
   - 2 consumers receiving messages
   - Verify all messages received exactly once

2. **Capacity Limits:**
   - Queue capacity: 10
   - Fast producer (1ms delay) vs slow consumer (100ms delay)
   - Verify producers block when queue full

3. **Graceful Shutdown:**
   - Initiate shutdown while threads are waiting
   - Ensure all threads terminate cleanly

### Expected Behavior
```
Producer 1: Sending message #1
Consumer A: Received from Producer 1: message #1
Producer 2: Sending message #1
Queue full! Producer 1 waiting...
Consumer B: Received from Producer 2: message #1
Producer 1: Resumed, sending message #2
...
Shutdown initiated
All consumers exiting...
All producers exiting...
```

### Performance Metrics to Track
- Average wait time for producers
- Average wait time for consumers
- Total messages processed
- Number of times producers/consumers blocked

---

## Assignment 2: Future-Based Parallel File Processor

### Objective
Create a system that processes multiple files in parallel using `std::async` and collects results using futures.

### Real-World Scenario
Build a log analyzer that reads multiple log files concurrently, extracts statistics, and aggregates results.

### Requirements

1. **FileStats Structure:**
```cpp
struct FileStats {
    std::string filename;
    size_t line_count;
    size_t word_count;
    size_t error_count;
    size_t warning_count;
    std::chrono::milliseconds processing_time;
};
```

2. **Operations:**
   - Process multiple files concurrently
   - Each file processed by separate async task
   - Aggregate statistics from all files
   - Handle file errors gracefully

3. **Features:**
   - Progress tracking (which files are done)
   - Error handling (file not found, permission denied)
   - Timeout mechanism (abandon slow files)
   - Result aggregation

### Implementation Hints

**Function Signatures:**
```cpp
FileStats process_file(const std::string& filename);

struct AggregateStats {
    size_t total_files_processed;
    size_t total_lines;
    size_t total_errors;
    // ... more fields
};

AggregateStats process_files_parallel(const std::vector<std::string>& files);
```

**High-Level Structure:**
```cpp
AggregateStats process_files_parallel(const std::vector<std::string>& files) {
    std::vector<std::future<FileStats>> futures;
    
    // Launch async tasks for each file
    for (const auto& file : files) {
        futures.push_back(std::async(std::launch::async, process_file, file));
    }
    
    // Collect results with timeout
    AggregateStats result;
    for (auto& fut : futures) {
        // Use wait_for to implement timeout
        // Aggregate results
    }
    
    return result;
}
```

**Key Techniques:**
- Use `std::async` with `std::launch::async` policy
- Implement timeout using `wait_for()`
- Handle exceptions from file processing
- Track completion status

### Testing Scenarios

1. **Normal Processing:**
   - 10 files of varying sizes
   - All process successfully
   - Verify total statistics

2. **Error Handling:**
   - Include non-existent files
   - Include files without read permission
   - Verify system continues processing other files

3. **Timeout Handling:**
   - Simulate slow file processing
   - Set reasonable timeout
   - Verify timeouts are detected

### Expected Output
```
Processing 10 files in parallel...

File: log1.txt [DONE] - 1000 lines, 15 errors
File: log2.txt [DONE] - 2500 lines, 3 errors
File: log3.txt [ERROR] - File not found
File: log4.txt [TIMEOUT] - Processing took too long
...

=== Aggregate Statistics ===
Files processed: 7/10
Total lines: 15,432
Total errors: 87
Total warnings: 234
Processing time: 3.2s
```

---

## Assignment 3: Promise-Based Task Scheduler

### Objective
Build a simple task scheduler that executes tasks at specific times using promises to return results.

### Real-World Scenario
Create a job scheduling system like cron, where tasks are scheduled to run at specific times and return results via futures.

### Requirements

1. **Task Structure:**
```cpp
struct ScheduledTask {
    std::string task_name;
    std::function<int()> task_function;  // Returns result code
    std::chrono::system_clock::time_point scheduled_time;
    int priority;  // Optional: for ordering
};
```

2. **Scheduler Features:**
   - Add tasks with scheduled execution times
   - Execute tasks at the right time
   - Return future for each scheduled task
   - Support immediate execution
   - Support recurring tasks (bonus)

3. **Operations:**
   - `std::future<int> schedule_task(ScheduledTask task)`
   - `void start()` - Start scheduler thread
   - `void stop()` - Stop scheduler gracefully
   - `size_t pending_tasks()` - Count of scheduled tasks

### Implementation Hints

**Class Structure:**
```cpp
class TaskScheduler {
private:
    struct TaskEntry {
        ScheduledTask task;
        std::promise<int> promise;
    };
    
    std::priority_queue<TaskEntry> task_queue;  // Ordered by time
    std::mutex mtx;
    std::condition_variable cv;
    std::thread scheduler_thread;
    bool running;
    
    void scheduler_loop() {
        // Main loop: wait until next task is ready
        // Execute task and fulfill promise
    }
    
public:
    // Your methods here
};
```

**Scheduler Logic:**
```cpp
void scheduler_loop() {
    while (running) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (task_queue.empty()) {
            // Wait for new tasks
            cv.wait(lock);
            continue;
        }
        
        auto next_task = task_queue.top();
        auto now = std::chrono::system_clock::now();
        
        if (next_task.scheduled_time <= now) {
            // Execute task
            task_queue.pop();
            lock.unlock();
            
            // Run task and set promise value
            try {
                int result = next_task.task_function();
                next_task.promise.set_value(result);
            } catch (...) {
                next_task.promise.set_exception(std::current_exception());
            }
        } else {
            // Wait until task is ready
            cv.wait_until(lock, next_task.scheduled_time);
        }
    }
}
```

**Key Concepts:**
- Use priority queue ordered by scheduled time
- Use `wait_until()` to wait for next task
- Set promise value after task execution
- Handle exceptions in tasks

### Testing Scenarios

1. **Sequential Execution:**
   - Schedule 5 tasks at 1-second intervals
   - Verify each executes at correct time

2. **Immediate Execution:**
   - Schedule tasks in the past
   - Verify immediate execution

3. **Task Results:**
   - Schedule tasks that return different values
   - Retrieve results via futures
   - Verify correct values received

### Expected Output
```
Scheduler started

Scheduled: Task A at 14:30:00
Scheduled: Task B at 14:30:02
Scheduled: Task C at 14:30:05

[14:30:00] Executing Task A... Result: 0 (SUCCESS)
[14:30:02] Executing Task B... Result: 1 (WARNING)
[14:30:05] Executing Task C... Result: 0 (SUCCESS)

Scheduler stopped
All tasks completed
```

---

## Assignment 4: Thread Pool with packaged_task

### Objective
Implement a simple thread pool that executes submitted tasks using `std::packaged_task`.

### Real-World Scenario
Build a web server backend that handles HTTP requests using a fixed-size thread pool instead of creating a thread per request.

### Requirements

1. **ThreadPool Features:**
   - Fixed number of worker threads
   - Task queue for pending work
   - Submit tasks and get futures
   - Graceful shutdown

2. **Template Support:**
   - Accept any callable with any return type
   - Return appropriate future type

3. **Operations:**
```cpp
template<typename F, typename... Args>
auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

void shutdown();
void wait_for_completion();
```

### Implementation Hints

**Class Structure:**
```cpp
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;
    
    void worker_thread() {
        while (true) {
            std::packaged_task<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]{ return stop || !tasks.empty(); });
                
                if (stop && tasks.empty()) return;
                
                task = std::move(tasks.front());
                tasks.pop();
            }
            
            task();  // Execute the task
        }
    }
    
public:
    ThreadPool(size_t num_threads) : stop(false) {
        // Create worker threads
    }
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        // Create packaged_task
        // Get future
        // Add to queue
        // Notify worker
        // Return future
    }
};
```

**Submit Implementation Pattern:**
```cpp
template<typename F, typename... Args>
auto submit(F&& f, Args&&... args) {
    using return_type = typename std::result_of<F(Args...)>::type;
    
    // Bind arguments to function
    auto task_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    
    // Create packaged_task
    auto task = std::make_shared<std::packaged_task<return_type()>>(task_func);
    
    std::future<return_type> result = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (stop) throw std::runtime_error("ThreadPool is stopped");
        
        // Wrap in void() callable for queue
        tasks.emplace([task](){ (*task)(); });
    }
    
    cv.notify_one();
    return result;
}
```

**Key Challenges:**
- Type erasure for different task types
- Proper lifetime management of packaged_task
- Template metaprogramming for return types

### Testing Scenarios

1. **Basic Functionality:**
   - Submit 20 tasks to 4-thread pool
   - Verify all tasks execute
   - Verify results are correct

2. **Different Task Types:**
   - Submit tasks with different return types (int, string, void)
   - Verify all work correctly

3. **Load Testing:**
   - Submit 1000 fast tasks
   - Submit 10 slow tasks
   - Verify proper queuing and execution

### Expected Output
```
ThreadPool created with 4 workers

Submitting 100 tasks...
Tasks queued

Worker 1: Processing task #1
Worker 2: Processing task #2
Worker 3: Processing task #3
Worker 4: Processing task #4
...

All tasks completed
Results collected: 100/100 successful

Shutting down thread pool...
All workers terminated
```

---

## Assignment 5: Parallel Map-Reduce with Futures

### Objective
Implement a parallel map-reduce framework using futures for concurrent processing.

### Real-World Scenario
Build a word count system that processes large text files in parallel (like Hadoop MapReduce).

### Requirements

1. **Map-Reduce Operations:**
   - Map: Transform input data in parallel
   - Reduce: Combine results from map phase
   - Support custom map and reduce functions

2. **Template Design:**
```cpp
template<typename InputIt, typename MapFunc, typename ReduceFunc>
auto parallel_map_reduce(InputIt first, InputIt last, 
                         MapFunc map_func, 
                         ReduceFunc reduce_func);
```

3. **Features:**
   - Automatically partition input data
   - Execute map operations in parallel using async
   - Collect and reduce results
   - Configurable chunk size

### Implementation Hints

**High-Level Structure:**
```cpp
template<typename InputIt, typename MapFunc, typename ReduceFunc>
auto parallel_map_reduce(InputIt first, InputIt last, 
                         MapFunc map_func, 
                         ReduceFunc reduce_func) 
{
    using input_type = typename std::iterator_traits<InputIt>::value_type;
    using map_result_type = decltype(map_func(*first));
    
    // 1. Partition data into chunks
    size_t total_size = std::distance(first, last);
    size_t num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = total_size / num_threads;
    
    // 2. Launch map operations
    std::vector<std::future<map_result_type>> futures;
    
    auto chunk_start = first;
    for (size_t i = 0; i < num_threads; ++i) {
        auto chunk_end = (i == num_threads - 1) ? last : 
                         std::next(chunk_start, chunk_size);
        
        futures.push_back(std::async(std::launch::async, 
            [chunk_start, chunk_end, map_func]() {
                // Process chunk
                // Apply map_func to each element
                // Return intermediate result
            }
        ));
        
        chunk_start = chunk_end;
    }
    
    // 3. Collect map results
    std::vector<map_result_type> map_results;
    for (auto& fut : futures) {
        map_results.push_back(fut.get());
    }
    
    // 4. Reduce results
    return std::accumulate(map_results.begin(), map_results.end(),
                          map_result_type{}, reduce_func);
}
```

**Example Usage:**
```cpp
// Word count example
std::vector<std::string> files = {"file1.txt", "file2.txt", ...};

auto word_count = parallel_map_reduce(
    files.begin(), files.end(),
    
    // Map: count words in each file
    [](const std::string& filename) -> std::map<std::string, int> {
        // Read file and count words
        // Return map of word -> count
    },
    
    // Reduce: combine word counts
    [](const std::map<std::string, int>& a, 
       const std::map<std::string, int>& b) {
        // Merge two maps
        // Return combined map
    }
);
```

**Key Concepts:**
- Data partitioning strategy
- Async execution for map phase
- Result collection and reduction
- Generic programming with templates

### Testing Scenarios

1. **Simple Sum:**
   - Map: square each number
   - Reduce: sum all results
   - Verify correctness

2. **Word Count:**
   - Process multiple text files
   - Count word frequencies
   - Verify total counts

3. **Performance:**
   - Compare with sequential version
   - Measure speedup with different input sizes

### Expected Output
```
=== Parallel Map-Reduce ===
Input size: 1,000,000 elements
Threads: 8
Chunk size: 125,000 elements per thread

Map phase: Launching 8 async tasks...
Map phase: Complete (0.45s)

Reduce phase: Combining results...
Reduce phase: Complete (0.12s)

Total time: 0.57s
Sequential time: 2.34s
Speedup: 4.1x

Result: 333,283,500,000
```

---

## Assignment 6: Event Notification System with Shared Futures

### Objective
Create an event broadcasting system where multiple listeners wait for events using shared_future.

### Real-World Scenario
Build a stock trading system where multiple trading algorithms wait for market data updates simultaneously.

### Requirements

1. **Event Structure:**
```cpp
struct MarketEvent {
    std::string symbol;
    double price;
    int volume;
    std::chrono::system_clock::time_point timestamp;
};
```

2. **EventBroadcaster Features:**
   - Register multiple listeners
   - Broadcast events to all listeners
   - Listeners wait using shared_future
   - Support multiple event types

3. **Operations:**
   - `shared_future<MarketEvent> subscribe(string symbol)`
   - `void publish(MarketEvent event)`
   - `void unsubscribe()`

### Implementation Hints

**Class Structure:**
```cpp
class EventBroadcaster {
private:
    struct EventChannel {
        std::promise<MarketEvent> promise;
        std::shared_future<MarketEvent> shared_future;
        bool event_set;
    };
    
    std::map<std::string, EventChannel> channels;
    std::mutex mtx;
    
public:
    std::shared_future<MarketEvent> subscribe(const std::string& symbol) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Create new channel if doesn't exist
        if (channels.find(symbol) == channels.end()) {
            EventChannel channel;
            channel.promise = std::promise<MarketEvent>();
            channel.shared_future = channel.promise.get_future().share();
            channel.event_set = false;
            channels[symbol] = std::move(channel);
        }
        
        return channels[symbol].shared_future;
    }
    
    void publish(const MarketEvent& event) {
        // Set promise value for the symbol's channel
        // Multiple threads waiting on shared_future will all receive it
    }
};
```

**Listener Pattern:**
```cpp
void trading_algorithm(EventBroadcaster& broadcaster, std::string symbol) {
    auto future = broadcaster.subscribe(symbol);
    
    std::cout << "Waiting for " << symbol << " update...\n";
    
    MarketEvent event = future.get();  // Block until event arrives
    
    std::cout << "Received: " << symbol << " @ $" << event.price << "\n";
    // Make trading decision
}
```

**Key Points:**
- Use `shared_future` for broadcasting
- Each symbol has its own promise/future pair
- Multiple threads can wait on same shared_future
- Consider one-time events vs continuous streams

### Testing Scenarios

1. **Multiple Listeners:**
   - 5 listeners for same stock symbol
   - Publish event
   - Verify all 5 receive it simultaneously

2. **Different Symbols:**
   - Listeners for different symbols
   - Publish events for each
   - Verify correct routing

3. **Timing:**
   - Listeners subscribe at different times
   - Some before event, some after
   - Handle appropriately

### Expected Output
```
=== Stock Market Event System ===

Algorithm A: Subscribed to AAPL
Algorithm B: Subscribed to AAPL
Algorithm C: Subscribed to GOOGL
Algorithm D: Subscribed to AAPL

Market Update: AAPL @ $150.25

Algorithm A: Received AAPL @ $150.25 - BUYING
Algorithm B: Received AAPL @ $150.25 - HOLDING
Algorithm D: Received AAPL @ $150.25 - SELLING

Market Update: GOOGL @ $2800.50

Algorithm C: Received GOOGL @ $2800.50 - BUYING

All algorithms processed events successfully
```

---

## Evaluation Criteria

For all assignments:

1. **Correctness (30%)**
   - Proper synchronization
   - No race conditions
   - Correct results

2. **Design (25%)**
   - Clean class structure
   - Proper abstractions
   - RAII principles

3. **Exception Safety (20%)**
   - Handle errors gracefully
   - No resource leaks
   - Promise/future error handling

4. **Testing (15%)**
   - Comprehensive test cases
   - Edge cases covered
   - Performance measurements

5. **Documentation (10%)**
   - Clear comments
   - Usage examples
   - Design decisions explained

---

## Bonus Challenges

1. **Add monitoring/metrics** to all assignments
2. **Implement timeouts** where appropriate
3. **Add cancellation support** to long-running operations
4. **Compare performance** with sequential versions
5. **Visualize** thread activity and synchronization

---

## Common Pitfalls to Avoid

1. ❌ Forgetting to check for spurious wakeups
2. ❌ Not handling promise destruction (broken_promise)
3. ❌ Calling get() multiple times on same future
4. ❌ Holding locks while doing expensive operations
5. ❌ Not testing shutdown/cleanup paths
6. ❌ Ignoring exceptions in async operations

---

## Learning Objectives

By completing these assignments, you will:
- ✅ Master condition variable usage
- ✅ Understand promise/future mechanics
- ✅ Learn packaged_task for task management
- ✅ Use std::async effectively
- ✅ Handle timeouts and cancellation
- ✅ Design thread-safe systems
- ✅ Apply async patterns to real problems

Good luck! 🚀
