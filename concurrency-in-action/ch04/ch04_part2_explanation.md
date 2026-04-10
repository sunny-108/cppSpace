# Chapter 4: Synchronizing Concurrent Operations - Part 2

## Introduction

Part 2 covers more advanced ways to work with asynchronous tasks: packaged tasks, async functions, time limits, and using synchronization to simplify code design.

---

## Part 2: Advanced Asynchronous Operations

---

## 1. Saving a Task for Later with std::packaged_task

### 1.1 What is a Packaged Task?

**Simple Explanation:**  
`std::packaged_task` is like wrapping a function call in a box with a tracking label. You can store this box, pass it around, and execute it later. The tracking label (future) lets you get the result.

**Comparison:**
- **std::promise** - You manually set the value
- **std::packaged_task** - It automatically sets the value by calling a function

### 1.2 Basic Packaged Task Example

**Example: Simple Packaged Task**
```cpp
#include <iostream>
#include <future>
#include <thread>

int compute_square(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x * x;
}

int main() {
    // Wrap the function in a packaged_task
    std::packaged_task<int(int)> task(compute_square);
    
    // Get the future before moving the task
    std::future<int> result = task.get_future();
    
    // Run the task in another thread
    std::thread t(std::move(task), 10); // Pass argument 10
    
    std::cout << "Waiting for result...\n";
    std::cout << "Result: " << result.get() << std::endl;
    
    t.join();
    return 0;
}
```

**Output:**
```
Waiting for result...
Result: 100
```

### 1.3 Packaged Task with Task Queue

**Simple Explanation:**  
You can store packaged tasks in a queue and execute them later, like a to-do list.

**Example: Task Queue System**
```cpp
#include <iostream>
#include <future>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>

class TaskQueue {
private:
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    
public:
    // Add task to queue
    template<typename Func>
    std::future<void> submit(Func f) {
        std::packaged_task<void()> task(f);
        std::future<void> result = task.get_future();
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one();
        
        return result;
    }
    
    // Worker thread function
    void worker() {
        while (true) {
            std::packaged_task<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]{ return !tasks.empty() || done; });
                
                if (done && tasks.empty()) {
                    break;
                }
                
                task = std::move(tasks.front());
                tasks.pop();
            }
            
            task(); // Execute the task
        }
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cv.notify_all();
    }
};

int main() {
    TaskQueue queue;
    
    // Start worker thread
    std::thread worker([&queue]() { queue.worker(); });
    
    // Submit tasks
    auto fut1 = queue.submit([]{ 
        std::cout << "Task 1 executing\n"; 
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    
    auto fut2 = queue.submit([]{ 
        std::cout << "Task 2 executing\n"; 
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    
    auto fut3 = queue.submit([]{ 
        std::cout << "Task 3 executing\n"; 
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    
    // Wait for all tasks
    fut1.get();
    fut2.get();
    fut3.get();
    
    std::cout << "All tasks completed!\n";
    
    queue.shutdown();
    worker.join();
    
    return 0;
}
```

### 1.4 Packaged Task for Flexible Function Calls

**Example: Different Return Types**
```cpp
#include <iostream>
#include <future>
#include <string>

int add(int a, int b) { return a + b; }
std::string concat(const std::string& a, const std::string& b) { 
    return a + b; 
}

int main() {
    // Packaged task for int function
    std::packaged_task<int(int, int)> task1(add);
    auto fut1 = task1.get_future();
    task1(3, 4);
    std::cout << "3 + 4 = " << fut1.get() << std::endl;
    
    // Packaged task for string function
    std::packaged_task<std::string(const std::string&, const std::string&)> 
        task2(concat);
    auto fut2 = task2.get_future();
    task2("Hello, ", "World!");
    std::cout << fut2.get() << std::endl;
    
    return 0;
}
```

---

## 2. Making (std::)Promises with std::async

### 2.1 What is std::async?

**Simple Explanation:**  
`std::async` is the easiest way to run a function asynchronously. It automatically creates threads (or uses thread pools) and returns a future. Think of it as "just run this function in the background."

**Syntax:**
```cpp
std::future<ReturnType> fut = std::async(function, args...);
ReturnType result = fut.get();
```

### 2.2 Basic std::async Example

**Example: Simple Async**
```cpp
#include <iostream>
#include <future>
#include <chrono>

int expensive_computation(int x) {
    std::cout << "Computing...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return x * x;
}

int main() {
    std::cout << "Starting async computation\n";
    
    // Launch async - returns immediately
    std::future<int> result = std::async(expensive_computation, 10);
    
    std::cout << "Doing other work while computation runs...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Still working...\n";
    
    // Get result (blocks if not ready)
    std::cout << "Result: " << result.get() << std::endl;
    
    return 0;
}
```

### 2.3 Launch Policies

**Simple Explanation:**  
You can control HOW `std::async` runs the function:

| Policy | Meaning |
|--------|---------|
| `std::launch::async` | Guaranteed to run in separate thread |
| `std::launch::deferred` | Runs when you call `get()` (lazy evaluation) |
| `std::launch::async \| std::launch::deferred` | Implementation decides (default) |

**Example: Different Launch Policies**
```cpp
#include <iostream>
#include <future>

void print_thread_id(const std::string& label) {
    std::cout << label << " thread ID: " 
              << std::this_thread::get_id() << std::endl;
}

int compute(int x) {
    print_thread_id("Compute");
    return x * x;
}

int main() {
    print_thread_id("Main");
    
    // Policy 1: Async - definitely runs in another thread
    std::cout << "\n=== Async Policy ===\n";
    auto fut1 = std::async(std::launch::async, compute, 10);
    std::cout << "Result: " << fut1.get() << std::endl;
    
    // Policy 2: Deferred - runs in calling thread when get() is called
    std::cout << "\n=== Deferred Policy ===\n";
    auto fut2 = std::async(std::launch::deferred, compute, 20);
    std::cout << "Before get() call\n";
    std::cout << "Result: " << fut2.get() << std::endl;
    std::cout << "After get() call\n";
    
    // Policy 3: Default - implementation chooses
    std::cout << "\n=== Default Policy ===\n";
    auto fut3 = std::async(compute, 30);
    std::cout << "Result: " << fut3.get() << std::endl;
    
    return 0;
}
```

### 2.4 When to Use Each Approach

**Comparison Table:**

| Approach | Use When |
|----------|----------|
| **std::async** | Simple background tasks, don't care about thread details |
| **std::thread** | Need control over thread lifecycle |
| **std::packaged_task** | Want to separate task creation from execution |
| **std::promise** | Manually control when/how result is set |

**Example: Choosing the Right Tool**
```cpp
#include <iostream>
#include <future>
#include <thread>

int compute() { return 42; }

void example_async() {
    // Easy: Just run it in background
    auto fut = std::async(compute);
    std::cout << "Async result: " << fut.get() << std::endl;
}

void example_thread() {
    // More control: Manage thread explicitly
    int result;
    std::thread t([&result]() { result = compute(); });
    t.join();
    std::cout << "Thread result: " << result << std::endl;
}

void example_packaged_task() {
    // Flexibility: Create task, execute later
    std::packaged_task<int()> task(compute);
    auto fut = task.get_future();
    std::thread t(std::move(task));
    std::cout << "Packaged task result: " << fut.get() << std::endl;
    t.join();
}

void example_promise() {
    // Full control: Set value manually
    std::promise<int> prom;
    auto fut = prom.get_future();
    std::thread t([](std::promise<int> p) { 
        p.set_value(compute()); 
    }, std::move(prom));
    std::cout << "Promise result: " << fut.get() << std::endl;
    t.join();
}

int main() {
    example_async();
    example_thread();
    example_packaged_task();
    example_promise();
    return 0;
}
```

---

## 3. Associating a Task with a Future

### 3.1 Understanding Future State

**Simple Explanation:**  
A future can be in different states:
- **Not ready** - Result not available yet
- **Ready** - Result is available
- **Retrieved** - Result has been retrieved (future consumed)

**Example: Checking Future Status**
```cpp
#include <iostream>
#include <future>
#include <chrono>

int slow_function() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42;
}

int main() {
    auto fut = std::async(std::launch::async, slow_function);
    
    // Check if result is ready without blocking
    while (fut.wait_for(std::chrono::milliseconds(100)) != 
           std::future_status::ready) {
        std::cout << "Still waiting...\n";
    }
    
    std::cout << "Result is ready!\n";
    std::cout << "Result: " << fut.get() << std::endl;
    
    return 0;
}
```

### 3.2 Future Status Values

```cpp
enum class future_status {
    ready,      // Result is available
    timeout,    // Timeout expired, result not ready
    deferred    // Function hasn't started (deferred launch)
};
```

**Example: Handling Different Statuses**
```cpp
#include <iostream>
#include <future>

int main() {
    // Deferred future
    auto fut_def = std::async(std::launch::deferred, []{ return 42; });
    
    auto status = fut_def.wait_for(std::chrono::seconds(0));
    
    if (status == std::future_status::deferred) {
        std::cout << "Function is deferred - will run on get()\n";
        std::cout << "Result: " << fut_def.get() << std::endl;
    }
    
    // Async future
    auto fut_async = std::async(std::launch::async, []{ 
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 100; 
    });
    
    status = fut_async.wait_for(std::chrono::milliseconds(500));
    
    if (status == std::future_status::timeout) {
        std::cout << "Not ready yet, continuing...\n";
    }
    
    status = fut_async.wait_for(std::chrono::seconds(1));
    
    if (status == std::future_status::ready) {
        std::cout << "Ready! Result: " << fut_async.get() << std::endl;
    }
    
    return 0;
}
```

---

## 4. Waiting with a Time Limit

### 4.1 Timed Wait Functions

**Simple Explanation:**  
Sometimes you don't want to wait forever. Timed waits let you say "wait up to X seconds, then give up."

**Available Functions:**
- `wait_for(duration)` - Wait for a relative time
- `wait_until(time_point)` - Wait until an absolute time

### 4.2 Using wait_for()

**Example: Timeout-Based Waiting**
```cpp
#include <iostream>
#include <future>
#include <chrono>

int unpredictable_task() {
    // Randomly takes 1-3 seconds
    int delay = (rand() % 3) + 1;
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    return delay;
}

int main() {
    srand(time(nullptr));
    
    auto fut = std::async(std::launch::async, unpredictable_task);
    
    std::cout << "Waiting up to 2 seconds...\n";
    
    auto status = fut.wait_for(std::chrono::seconds(2));
    
    if (status == std::future_status::ready) {
        std::cout << "Task completed in time! Result: " 
                  << fut.get() << " seconds\n";
    } else if (status == std::future_status::timeout) {
        std::cout << "Task is taking too long! Will wait more...\n";
        int result = fut.get(); // Wait indefinitely
        std::cout << "Finally got result: " << result << " seconds\n";
    }
    
    return 0;
}
```

### 4.3 Using wait_until()

**Example: Deadline-Based Waiting**
```cpp
#include <iostream>
#include <future>
#include <chrono>

int main() {
    auto fut = std::async(std::launch::async, []{ 
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 42; 
    });
    
    // Set deadline: 2 seconds from now
    auto deadline = std::chrono::steady_clock::now() + 
                    std::chrono::seconds(2);
    
    std::cout << "Waiting until deadline...\n";
    
    auto status = fut.wait_until(deadline);
    
    if (status == std::future_status::ready) {
        std::cout << "Finished before deadline!\n";
    } else {
        std::cout << "Missed deadline, still waiting...\n";
        fut.get(); // Continue waiting
        std::cout << "Done!\n";
    }
    
    return 0;
}
```

### 4.4 Timed Waits with Condition Variables

**Example: Condition Variable with Timeout**
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool data_ready = false;

void wait_for_data_with_timeout() {
    std::unique_lock<std::mutex> lock(mtx);
    
    std::cout << "Waiting for data (max 3 seconds)...\n";
    
    // Wait for up to 3 seconds
    bool result = cv.wait_for(lock, std::chrono::seconds(3), 
                              []{ return data_ready; });
    
    if (result) {
        std::cout << "Data received!\n";
    } else {
        std::cout << "Timeout! Data not received.\n";
    }
}

void prepare_data(int delay_seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(delay_seconds));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        data_ready = true;
    }
    
    std::cout << "Data prepared!\n";
    cv.notify_one();
}

int main() {
    // Test 1: Data arrives in time
    std::cout << "=== Test 1: Data arrives quickly ===\n";
    data_ready = false;
    std::thread t1(wait_for_data_with_timeout);
    std::thread t2(prepare_data, 1); // 1 second delay
    t1.join();
    t2.join();
    
    // Test 2: Data takes too long
    std::cout << "\n=== Test 2: Data arrives slowly ===\n";
    data_ready = false;
    std::thread t3(wait_for_data_with_timeout);
    std::thread t4(prepare_data, 5); // 5 second delay
    t3.join();
    t4.join();
    
    return 0;
}
```

---

## 5. Using Synchronization to Simplify Code

### 5.1 Functional Programming Style with Futures

**Simple Explanation:**  
Instead of managing threads and locks manually, use futures to write cleaner, more functional code.

**Example: Sequential vs Parallel Processing**
```cpp
#include <iostream>
#include <future>
#include <vector>
#include <algorithm>
#include <numeric>

// Process data sequentially
int process_sequential(const std::vector<int>& data) {
    int sum = 0;
    for (int val : data) {
        // Simulate expensive computation
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sum += val * val;
    }
    return sum;
}

// Process data in parallel using futures
int process_parallel(const std::vector<int>& data) {
    std::vector<std::future<int>> futures;
    
    // Launch async tasks for each element
    for (int val : data) {
        futures.push_back(std::async(std::launch::async, [val]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return val * val;
        }));
    }
    
    // Collect results
    int sum = 0;
    for (auto& fut : futures) {
        sum += fut.get();
    }
    
    return sum;
}

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto start = std::chrono::steady_clock::now();
    int result1 = process_sequential(data);
    auto end = std::chrono::steady_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::steady_clock::now();
    int result2 = process_parallel(data);
    end = std::chrono::steady_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential: " << result1 << " in " << duration1.count() << "ms\n";
    std::cout << "Parallel: " << result2 << " in " << duration2.count() << "ms\n";
    std::cout << "Speedup: " << (duration1.count() / (double)duration2.count()) << "x\n";
    
    return 0;
}
```

### 5.2 Quick Sort with Parallel Execution

**Example: Parallel Quick Sort**
```cpp
#include <iostream>
#include <future>
#include <vector>
#include <algorithm>

template<typename T>
std::vector<T> parallel_quick_sort(std::vector<T> input) {
    if (input.size() < 2) {
        return input;
    }
    
    // Choose pivot
    T pivot = input[0];
    
    // Partition
    auto divide_point = std::partition(input.begin() + 1, input.end(),
                                       [&](const T& t) { return t < pivot; });
    
    // Lower half
    std::vector<T> lower(input.begin() + 1, divide_point);
    
    // Upper half
    std::vector<T> upper(divide_point, input.end());
    
    // Sort lower half asynchronously
    std::future<std::vector<T>> lower_future = 
        std::async(std::launch::async, parallel_quick_sort<T>, std::move(lower));
    
    // Sort upper half in this thread
    auto upper_sorted = parallel_quick_sort(std::move(upper));
    
    // Wait for lower half
    auto lower_sorted = lower_future.get();
    
    // Combine results
    std::vector<T> result;
    result.reserve(input.size());
    result.insert(result.end(), lower_sorted.begin(), lower_sorted.end());
    result.push_back(pivot);
    result.insert(result.end(), upper_sorted.begin(), upper_sorted.end());
    
    return result;
}

int main() {
    std::vector<int> data = {9, 3, 7, 1, 5, 2, 8, 4, 6};
    
    std::cout << "Original: ";
    for (int n : data) std::cout << n << " ";
    std::cout << std::endl;
    
    auto sorted = parallel_quick_sort(data);
    
    std::cout << "Sorted: ";
    for (int n : sorted) std::cout << n << " ";
    std::cout << std::endl;
    
    return 0;
}
```

### 5.3 Pipeline Pattern with Futures

**Example: Data Processing Pipeline**
```cpp
#include <iostream>
#include <future>
#include <string>
#include <vector>

// Stage 1: Read data
std::vector<std::string> read_data() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Stage 1: Data read\n";
    return {"data1", "data2", "data3"};
}

// Stage 2: Process data
std::vector<std::string> process_data(std::vector<std::string> data) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Stage 2: Data processed\n";
    for (auto& s : data) {
        s = s + "_processed";
    }
    return data;
}

// Stage 3: Save data
void save_data(std::vector<std::string> data) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Stage 3: Data saved\n";
    for (const auto& s : data) {
        std::cout << "  - " << s << std::endl;
    }
}

int main() {
    std::cout << "Starting pipeline...\n";
    
    // Chain futures to create a pipeline
    auto fut1 = std::async(std::launch::async, read_data);
    auto fut2 = std::async(std::launch::async, process_data, fut1.get());
    std::async(std::launch::async, save_data, fut2.get()).get();
    
    std::cout << "Pipeline complete!\n";
    
    return 0;
}
```

---

## 6. Real-World Example: Web Scraper with Concurrent Requests

**Example: Concurrent HTTP Requests (Simulated)**
```cpp
#include <iostream>
#include <future>
#include <vector>
#include <string>
#include <chrono>
#include <random>

// Simulate fetching a URL
std::string fetch_url(const std::string& url) {
    // Random delay 1-3 seconds
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 3);
    
    int delay = dis(gen);
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    
    return "Content from " + url + " (took " + std::to_string(delay) + "s)";
}

int main() {
    std::vector<std::string> urls = {
        "http://example.com/page1",
        "http://example.com/page2",
        "http://example.com/page3",
        "http://example.com/page4",
        "http://example.com/page5"
    };
    
    std::cout << "Fetching " << urls.size() << " URLs concurrently...\n\n";
    
    auto start = std::chrono::steady_clock::now();
    
    // Launch all requests concurrently
    std::vector<std::future<std::string>> futures;
    for (const auto& url : urls) {
        futures.push_back(std::async(std::launch::async, fetch_url, url));
    }
    
    // Collect results as they complete
    for (auto& fut : futures) {
        std::string content = fut.get();
        std::cout << content << std::endl;
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    std::cout << "\nTotal time: " << duration.count() << " seconds\n";
    std::cout << "(Would have taken ~" << (urls.size() * 2) 
              << "s sequentially)\n";
    
    return 0;
}
```

---

## Summary of Part 2

| Concept | Purpose | Best Use Case |
|---------|---------|---------------|
| **std::packaged_task** | Wrap function for later execution | Task queues, thread pools |
| **std::async** | Easy async execution | Simple parallel tasks |
| **wait_for/wait_until** | Timed waiting | Timeouts, deadlines |
| **Launch policies** | Control execution | Performance tuning |
| **Parallel patterns** | Simplify code | Functional programming |

---

## Key Design Patterns

### Pattern 1: Task Queue with packaged_task
```cpp
std::queue<std::packaged_task<void()>> tasks;
// Add tasks, worker threads execute them
```

### Pattern 2: Async with wait_for
```cpp
auto fut = std::async(task);
if (fut.wait_for(timeout) == std::future_status::ready) {
    // Task completed
}
```

### Pattern 3: Parallel Processing
```cpp
std::vector<std::future<Result>> futures;
for (auto& item : items) {
    futures.push_back(std::async(process, item));
}
// Collect results
```

### Pattern 4: Pipeline
```cpp
auto stage1 = std::async(read_data);
auto stage2 = std::async(process, stage1.get());
auto stage3 = std::async(save, stage2.get());
```

---

## Complete Comparison: All Async Mechanisms

| Feature | async | packaged_task | promise | thread |
|---------|-------|---------------|---------|--------|
| **Ease of use** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐ | ⭐⭐ |
| **Control** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **Flexibility** | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| **Auto future** | ✅ | ✅ | ✅ | ❌ |
| **Exception passing** | ✅ | ✅ | ✅ | ❌ |

---

## Best Practices

1. **Use std::async for simple cases** - It's the easiest and safest
2. **Use packaged_task for task queues** - Separate creation from execution
3. **Use promise for manual control** - When you need fine-grained control
4. **Always use timeouts for unreliable operations** - Prevent infinite waits
5. **Prefer futures over manual synchronization** - Cleaner and safer code

---

## Common Mistakes to Avoid

```cpp
// ❌ WRONG: Forgetting to get() the future
auto fut = std::async(task);
// Task might not run! (deferred launch)

// ✅ RIGHT: Always get() or wait()
auto fut = std::async(task);
fut.get(); // or fut.wait()

// ❌ WRONG: Moving packaged_task without getting future first
std::packaged_task<int()> task(func);
std::thread t(std::move(task)); // Can't get future now!

// ✅ RIGHT: Get future before moving
std::packaged_task<int()> task(func);
auto fut = task.get_future();
std::thread t(std::move(task));
int result = fut.get();
```

---

## Final Thoughts

Chapter 4 provides powerful tools for:
- ✅ Efficient thread synchronization
- ✅ Clean asynchronous code
- ✅ Flexible task management
- ✅ Timeout handling
- ✅ Functional programming patterns

**Remember:** The goal is to write code that is both concurrent AND maintainable!
