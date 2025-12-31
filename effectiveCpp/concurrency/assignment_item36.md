# Programming Assignment - Item 36: Specify std::launch::async if Asynchronicity is Essential

## Learning Objectives

- Understand different launch policies (async, deferred, default)
- Recognize problems with default launch policy
- Practice explicit launch policy specification
- Handle timeout-based operations correctly

---

## Exercise 1: Launch Policy Detection (Easy)

### Problem

Create a utility that detects which launch policy was used by checking future status.

### Requirements

```cpp
enum class LaunchPolicy {
    Async,
    Deferred,
    Unknown
};

class PolicyDetector {
public:
    template<typename F>
    static LaunchPolicy detectPolicy(F&& f);
  
    static void demonstrateAllPolicies();
};
```

### Example

```cpp
auto future = std::async(std::launch::async, []() { return 42; });
auto policy = PolicyDetector::detectPolicy(future);
// Should return LaunchPolicy::Async
```

### Hints

Use wait_for(0s) to check status immediately

Check for std::future_status::deferred

Remember default policy can be either

---

## Exercise 2: Timeout Handler (Medium)

### Problem

Implement a robust async operation with timeout that works correctly with all launch policies.

### Requirements

```cpp
template<typename F, typename... Args>
class TimeoutTask {
public:
    TimeoutTask(std::chrono::milliseconds timeout);
  
    auto execute(F&& f, Args&&... args) 
        -> std::optional<std::invoke_result_t<F, Args...>>;
  
    bool timedOut() const;
};
```

### Example

```cpp
TimeoutTask<int()> task(1000ms);  // 1 second timeout

auto result = task.execute([]() {
    std::this_thread::sleep_for(2s);
    return 42;
});

if (!result) {
    std::cout << "Task timed out!" << std::endl;
}
```

### Hints

- Must use std::launch::async for reliable timeout
- Check if future is deferred first
- Use wait_for() with the timeout duration

---

## Exercise 3: Thread-Local Storage Issue (Medium)

### Problem

Demonstrate the problem with thread-local storage when using default launch policy, then fix it.

### Requirements

```cpp
class ThreadLocalDemo {
public:
    // Buggy version using default policy
    static int buggyThreadLocalAccess();
  
    // Fixed version using explicit async
    static int correctThreadLocalAccess();
  
    // Show the difference
    static void demonstrateProblem();
};
```

### Example

```cpp
ThreadLocalDemo::demonstrateProblem();
// Output:
// Buggy version: Unexpected thread ID behavior
// Correct version: Guaranteed new thread
```

### Hints

- Use thread_local variable
- Compare thread IDs
- Default policy might run in calling thread

---

## Exercise 4: Real-World Async Wrapper (Hard)

### Problem

Create a safer async wrapper that forces async execution and provides better error messages.

### Requirements

```cpp
template<typename F, typename... Args>
auto reallyAsync(F&& f, Args&&... args) 
    -> std::future<std::invoke_result_t<F, Args...>>;

template<typename F, typename... Args>
auto asyncWithTimeout(F&& f, std::chrono::milliseconds timeout, Args&&... args)
    -> std::optional<std::invoke_result_t<F, Args...>>;
```

### Example

```cpp
// Guaranteed to run asynchronously
auto future = reallyAsync(heavyComputation, data);

// With timeout
auto result = asyncWithTimeout(networkCall, 5000ms, url);
if (!result) {
    std::cout << "Network call timed out\n";
}
```

### Hints

- Always use std::launch::async
- Handle both timeout and exceptions
- Return optional for timeout case

---

## Exercise 5: Lazy Evaluation System (Medium)

### Problem

Build a lazy evaluation system that uses deferred policy appropriately.

### Requirements

```cpp
template<typename T>
class LazyValue {
public:
    template<typename F>
    LazyValue(F&& f);
  
    T& get();  // Compute on first access
    bool isEvaluated() const;
    void reset();  // Clear and re-evaluate next time
  
private:
    std::future<T> future;
    bool evaluated;
};
```

### Example

```cpp
LazyValue<int> expensive([]() {
    std::this_thread::sleep_for(1s);
    return 42;
});

// Not computed yet
std::cout << "Created lazy value\n";

// Computed on first access
int value = expensive.get();  // Takes 1 second

// Cached on subsequent access
int value2 = expensive.get();  // Instant
```

### Hints

- Use std::launch::deferred
- Track evaluation state
- Use promise/future for reset capability

---

## Exercise 6: Policy-Aware Task Queue (Hard)

### Problem

Implement a task queue that allows users to specify launch policy per task.

### Requirements

```cpp
class TaskQueue {
public:
    struct TaskConfig {
        std::launch policy;
        std::optional<std::chrono::milliseconds> timeout;
        int priority;
    };
  
    template<typename F, typename... Args>
    auto submit(F&& f, const TaskConfig& config, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;
  
    size_t pendingTasks() const;
    void waitAll();
};
```

### Example

```cpp
TaskQueue queue;

// High priority, must be async
queue.submit(criticalTask, {
    .policy = std::launch::async,
    .timeout = 5000ms,
    .priority = 10
});

// Low priority, can be deferred
queue.submit(logTask, {
    .policy = std::launch::deferred,
    .timeout = std::nullopt,
    .priority = 1
});

queue.waitAll();
```

### Hints

- Store futures with their configs
- Handle timeout in submit
- Priority queue for deferred tasks

---

## Exercise 7: Benchmark Launch Policies (Medium)

### Problem

Compare performance and behavior of different launch policies.

### Requirements

```cpp
class LaunchPolicyBenchmark {
public:
    struct BenchmarkResult {
        std::string testName;
        double asyncTime;
        double deferredTime;
        double defaultTime;
        size_t threadsCreated;
    };
  
    BenchmarkResult benchmarkShortTasks(int numTasks);
    BenchmarkResult benchmarkLongTasks(int numTasks);
    BenchmarkResult benchmarkTimeouts(int numTasks);
  
    void printResults();
};
```

### Example Output

```
Short Tasks (1000 tasks):
  async:    450ms (1000 threads)
  deferred: 320ms (1 thread)
  default:  380ms (8 threads)

Long Tasks (10 tasks):
  async:    5200ms (10 threads)
  deferred: 51000ms (1 thread)
  default:  5300ms (10 threads)
```

### Hints

- Measure time with std::chrono
- Track thread creation (thread::id)
- Show when each policy is better

---

## Exercise 8: Conditional Async Execution (Medium)

### Problem

Create a system that chooses launch policy based on runtime conditions.

### Requirements

```cpp
class SmartAsync {
public:
    struct ExecutionPolicy {
        size_t maxConcurrentTasks;
        size_t taskComplexityThreshold;
        bool preferAsync;
    };
  
    SmartAsync(const ExecutionPolicy& policy);
  
    template<typename F, typename... Args>
    auto execute(F&& f, size_t complexity, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;
  
private:
    std::launch chooseLaunchPolicy(size_t complexity);
    std::atomic<size_t> activeTasks;
    ExecutionPolicy policy;
};
```

### Example

```cpp
SmartAsync::ExecutionPolicy policy{
    .maxConcurrentTasks = 4,
    .taskComplexityThreshold = 1000,
    .preferAsync = true
};

SmartAsync executor(policy);

// Simple task - might use deferred
auto f1 = executor.execute(simpleTask, 10);

// Complex task - uses async
auto f2 = executor.execute(complexTask, 5000);
```

### Hints

- Check active task count
- Use complexity threshold
- Fall back to deferred if too many tasks

---

## Exercise 9: Async with Cancellation (Advanced)

### Problem

Implement async operations that can be cancelled, using appropriate launch policies.

### Requirements

```cpp
class CancellableTask {
public:
    template<typename F, typename... Args>
    CancellableTask(F&& f, Args&&... args);
  
    void cancel();
    bool isCancelled() const;
  
    template<typename T>
    std::optional<T> getResult(std::chrono::milliseconds timeout);
  
private:
    std::atomic<bool> cancelled{false};
    std::future<void> future;
};
```

### Example

```cpp
CancellableTask task([](std::atomic<bool>& cancel) {
    for (int i = 0; i < 100 && !cancel; ++i) {
        std::this_thread::sleep_for(100ms);
    }
    return i;
});

std::this_thread::sleep_for(500ms);
task.cancel();

auto result = task.getResult(1000ms);
// result should be around 5 (500ms / 100ms)
```

### Hints

- Must use std::launch::async for cancellation
- Pass cancel flag to task
- Check flag periodically in task

---

## Exercise 10: Policy Migration Tool (Advanced)

### Problem

Create a tool that analyzes code using default policy and suggests where async should be explicit.

### Requirements

```cpp
class PolicyAnalyzer {
public:
    struct CodePattern {
        std::string description;
        bool needsExplicitAsync;
        std::string reason;
    };
  
    static CodePattern analyzePattern(const std::string& code);
    static std::vector<std::string> getSuggestions();
};
```

### Example

```cpp
std::string code = R"(
    auto f = std::async(task);
    if (f.wait_for(1s) == std::future_status::timeout) {
        // timeout handling
    }
)";

auto pattern = PolicyAnalyzer::analyzePattern(code);
// needsExplicitAsync = true
// reason = "Uses wait_for which may not work with deferred"
```

### Hints

- Look for wait_for/wait_until usage
- Check for thread_local access
- Identify timeout patterns

---

## Testing Your Solutions

### Compile and Run

```bash
g++ -std=c++17 -pthread -o item36 item36_solution.cpp
./item36
```

### Expected Behaviors to Verify

- [ ] Async policy creates new thread immediately
- [ ] Deferred policy delays execution
- [ ] Default policy behavior is understood
- [ ] Timeouts work correctly with async
- [ ] Thread-local storage behaves correctly
- [ ] No accidental blocking

---

## Common Pitfalls to Avoid

1. **Using default policy with wait_for**

   - May return deferred status immediately
   - Timeout never triggers for deferred tasks
2. **Assuming default is always async**

   - Implementation-defined behavior
   - Can't rely on new thread creation
3. **Not checking for deferred status**

   - Leads to infinite waits
   - Confusing behavior
4. **Using deferred when async is needed**

   - No parallelism
   - Runs in calling thread

---

## Grading Rubric

| Criteria             | Points        |
| -------------------- | ------------- |
| Policy understanding | 30            |
| Timeout handling     | 25            |
| Thread safety        | 20            |
| Code quality         | 15            |
| Documentation        | 10            |
| **Total**      | **100** |
