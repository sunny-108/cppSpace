# Programming Assignment - Item 35: Prefer Task-Based Programming to Thread-Based

## Learning Objectives

- Understand the advantages of task-based programming
- Practice using std::async over std::thread
- Handle return values and exceptions from concurrent tasks
- Compare performance and complexity of both approaches

---

## Exercise 1: Return Value Handling (Easy)

### Problem

Create two versions of a parallel sum calculator:

1. Thread-based version using std::thread
2. Task-based version using std::async

Calculate the sum of numbers from 1 to N using 4 threads/tasks, each handling a quarter of the range.

### Requirements

```cpp
// Thread-based version
long long parallelSumThreadBased(long long N);

// Task-based version  
long long parallelSumTaskBased(long long N);
```

### Example

```cpp
long long result = parallelSumTaskBased(1000000);
// Should return sum of 1 to 1,000,000
```

### Hints

- For thread-based: Use a vector to store partial results, pass by reference
- For task-based: Use std::async and call .get() on futures
- Split the range into 4 equal parts

---

## Exercise 2: Exception Propagation (Medium)

### Problem

Implement a parallel file processor that:

- Reads multiple files concurrently
- Throws exceptions for files that don't exist
- Collects all exceptions and reports them

Create both thread-based and task-based versions.

### Requirements

```cpp
class FileProcessor {
public:
    // Returns content of files that succeeded
    // Throws aggregated exception for files that failed
  
    std::vector<std::string> processFilesThreadBased(
        const std::vector<std::string>& filenames);
  
    std::vector<std::string> processFilesTaskBased(
        const std::vector<std::string>& filenames);
};
```

### Example

```cpp
FileProcessor processor;
std::vector<std::string> files = {"file1.txt", "file2.txt", "missing.txt"};

try {
    auto contents = processor.processFilesTaskBased(files);
    // Should succeed for file1 and file2
} catch (const std::exception& e) {
    // Should contain info about missing.txt
}
```

### Hints

- Task-based: Exceptions stored in futures automatically
- Thread-based: Need to manually catch and store exceptions
- Use std::exception_ptr for thread-based version

---

## Exercise 3: Matrix Multiplication (Medium)

### Problem

Implement parallel matrix multiplication using both approaches:

- Thread-based with manual thread management
- Task-based with std::async

Compare the code complexity and performance.

### Requirements

```cpp
using Matrix = std::vector<std::vector<int>>;

class MatrixMultiplier {
public:
    Matrix multiplyThreadBased(const Matrix& A, const Matrix& B);
    Matrix multiplyTaskBased(const Matrix& A, const Matrix& B);
};
```

### Example

```cpp
Matrix A = {{1, 2}, {3, 4}};
Matrix B = {{5, 6}, {7, 8}};

MatrixMultiplier mult;
Matrix result = mult.multiplyTaskBased(A, B);
// Result: {{19, 22}, {43, 50}}
```

### Hints

- Parallelize by rows
- Each task/thread computes one or more rows
- Task-based version should be much simpler

---

## Exercise 4: Download Manager (Hard)

### Problem

Create a concurrent download manager that:

- Downloads multiple URLs simultaneously
- Returns results as they complete
- Has a timeout for slow downloads
- Reports success/failure for each URL

Implement using task-based approach to show its advantages.

### Requirements

```cpp
struct DownloadResult {
    std::string url;
    bool success;
    std::string data;
    std::string error;
};

class DownloadManager {
public:
    // Download with timeout
    std::vector<DownloadResult> downloadAll(
        const std::vector<std::string>& urls,
        std::chrono::seconds timeout);
  
private:
    DownloadResult downloadSingle(const std::string& url);
};
```

### Example

```cpp
DownloadManager dm;
std::vector<std::string> urls = {
    "http://example.com/file1",
    "http://example.com/file2"
};

auto results = dm.downloadAll(urls, 5s);
for (const auto& r : results) {
    if (r.success) {
        std::cout << r.url << ": " << r.data.size() << " bytes\n";
    } else {
        std::cout << r.url << ": " << r.error << "\n";
    }
}
```

### Hints

- Use std::async with std::launch::async
- Use wait_for() to implement timeout
- For simulation, use sleep instead of actual HTTP

---

## Exercise 5: Prime Number Finder (Medium)

### Problem

Find all prime numbers in a range using parallel computation.
Compare thread-based and task-based implementations.

### Requirements

```cpp
class PrimeFinder {
public:
    std::vector<long long> findPrimesThreadBased(
        long long start, long long end, int numThreads);
  
    std::vector<long long> findPrimesTaskBased(
        long long start, long long end, int numTasks);
  
private:
    bool isPrime(long long n);
};
```

### Example

```cpp
PrimeFinder pf;
auto primes = pf.findPrimesTaskBased(1, 1000, 4);
// Returns all primes between 1 and 1000
```

### Hints

- Divide range among tasks/threads
- Each computes primes in its sub-range
- Combine results at the end
- Task-based: easier to collect results

---

## Exercise 6: Image Processing Pipeline (Hard)

### Problem

Create an image processing pipeline that:

1. Loads images
2. Applies filters (blur, sharpen, etc.)
3. Saves results

Use task-based approach to chain operations.

### Requirements

```cpp
struct Image {
    std::vector<std::vector<int>> pixels;
    int width, height;
};

class ImagePipeline {
public:
    // Chain of operations
    std::future<Image> processImage(
        const std::string& filename,
        const std::vector<std::function<Image(Image)>>& filters);
  
    // Batch processing
    std::vector<Image> processBatch(
        const std::vector<std::string>& filenames,
        const std::vector<std::function<Image(Image)>>& filters);
};
```

### Example

```cpp
ImagePipeline pipeline;

auto filters = {
    [](Image img) { return blur(img); },
    [](Image img) { return sharpen(img); }
};

auto future = pipeline.processImage("photo.jpg", filters);
Image result = future.get();
```

### Hints

- Use std::async for each stage
- Chain futures using .then() pattern (or nested async)
- Process multiple images in parallel

---

## Exercise 7: Resource Pool Manager (Advanced)

### Problem

Implement a resource pool (e.g., database connections) where:

- Resources are created on demand
- Clients request resources via futures
- Resources are returned to pool after use

Show why task-based is superior for this pattern.

### Requirements

```cpp
template<typename Resource>
class ResourcePool {
public:
    ResourcePool(size_t maxSize, 
                 std::function<Resource()> factory);
  
    // Returns future to resource
    std::future<std::shared_ptr<Resource>> acquire();
  
    void release(std::shared_ptr<Resource> resource);
  
    size_t available() const;
};
```

### Example

```cpp
auto factory = []() { return DatabaseConnection(); };
ResourcePool<DatabaseConnection> pool(10, factory);

auto futureConn = pool.acquire();
auto conn = futureConn.get();  // Blocks if none available
// Use connection
pool.release(conn);
```

### Hints

- Use std::async to handle wait-and-acquire
- std::promise/future for signaling availability
- Queue of available resources

---

## Exercise 8: Comparison Benchmark (Medium)

### Problem

Create a comprehensive benchmark comparing thread-based and task-based approaches across multiple scenarios.

### Requirements

```cpp
class ConcurrencyBenchmark {
public:
    struct BenchmarkResult {
        std::string testName;
        double threadBasedTime;
        double taskBasedTime;
        size_t linesOfCode;  // Approximate
    };
  
    std::vector<BenchmarkResult> runAllBenchmarks();
  
private:
    BenchmarkResult benchmarkComputation();
    BenchmarkResult benchmarkIOOperations();
    BenchmarkResult benchmarkExceptionHandling();
};
```

### Example Output

```
Test: Computation
  Thread-based: 245ms, 85 LOC
  Task-based:   240ms, 42 LOC
  
Test: I/O Operations
  Thread-based: 512ms, 120 LOC
  Task-based:   498ms, 55 LOC
```

### Hints

- Use std::chrono for timing
- Count approximate lines of code for each approach
- Test various workloads

---

## Bonus Challenge: Thread Pool Implementation

### Problem

Implement a simple thread pool and show why it's better to use std::async (which may use one internally) than rolling your own threads.

### Requirements

```cpp
class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
  
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>>;
  
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};
```

### Goals

- Understand complexity of manual thread management
- Appreciate what std::async provides automatically
- Learn about work queues and synchronization

---

## Testing Your Solutions

### Compile and Run

```bash
g++ -std=c++17 -pthread -o item35 item35_solution.cpp
./item35
```

### Expected Output Format

```
Exercise 1: Return Value Handling
  Thread-based sum: 500000500000 (took 45ms)
  Task-based sum:   500000500000 (took 42ms)
  ✓ Results match

Exercise 2: Exception Propagation
  Thread-based: Caught 1 exceptions
  Task-based: Caught 1 exceptions  
  ✓ Both handled exceptions correctly

[Continue for all exercises...]
```

### Validation Checklist

- [ ] All exercises compile without warnings
- [ ] Thread-based and task-based give same results
- [ ] Task-based code is simpler (fewer LOC)
- [ ] No memory leaks (check with valgrind)
- [ ] No data races (check with thread sanitizer)
- [ ] Proper exception handling in both versions

---

## Submission Guidelines

Submit the following:

1. `item35_solution.cpp` - Your implementations
2. `item35_results.md` - Performance comparison and observations
3. `item35_analysis.md` - Discussion of when to use each approach

### Analysis Should Cover

- Code complexity comparison
- Performance differences observed
- Exception handling ease
- Resource management
- When you would choose thread-based over task-based (if ever)

---

## Grading Rubric

| Criteria        | Points        |
| --------------- | ------------- |
| Correctness     | 40            |
| Code quality    | 20            |
| Performance     | 15            |
| Error handling  | 15            |
| Documentation   | 10            |
| **Total** | **100** |

### Excellence Criteria (Bonus Points)

- Implement all bonus challenges (+10)
- Comprehensive benchmarking (+5)
- Creative additional examples (+5)
