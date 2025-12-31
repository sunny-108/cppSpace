/*
 * THREAD POOL - Basic Concepts and Design
 * ========================================
 * 
 * WHAT IS A THREAD POOL?
 * ----------------------
 * A thread pool is a collection of pre-created worker threads that wait for tasks
 * to be assigned. Instead of creating a new thread for each task (expensive operation),
 * we reuse existing threads from the pool.
 * 
 * WHY USE A THREAD POOL?
 * ----------------------
 * 1. Performance: Thread creation/destruction is expensive
 * 2. Resource Management: Limits number of concurrent threads
 * 3. Better Utilization: Keeps threads busy with queued tasks
 * 4. Simplicity: Abstracts thread management complexity
 * 
 * KEY COMPONENTS:
 * ---------------
 * 1. Worker Threads: Pool of threads waiting for work
 * 2. Task Queue: Queue holding tasks to be executed
 * 3. Synchronization: Mutex and condition variables for coordination
 * 4. Task Submission: Interface to add tasks to the queue
 * 5. Lifecycle Management: Start, stop, and cleanup mechanisms
 */

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <chrono>

// Simple Thread Pool Implementation
class ThreadPool {
public:
    // Constructor: Creates worker threads
    explicit ThreadPool(size_t numThreads) : stop(false) {
        std::cout << "Creating thread pool with " << numThreads << " threads\n";
        
        // Create worker threads
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, i] {
                workerThread(i);
            });
        }
    }

    // Destructor: Stops all threads and waits for completion
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;  // Signal threads to stop
        }
        
        // Wake up all threads
        condition.notify_all();
        
        // Wait for all threads to finish
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Thread pool destroyed\n";
    }

    // Submit a task to the thread pool
    // Returns a future to get the result
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        // Create a packaged task (wraps function and provides future)
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            // Don't allow enqueueing after stopping
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            
            // Add task to queue
            tasks.emplace([task]() { (*task)(); });
        }
        
        // Notify one waiting thread
        condition.notify_one();
        
        return result;
    }
    
    // Get number of pending tasks
    size_t pendingTasks() {
        std::unique_lock<std::mutex> lock(queueMutex);
        return tasks.size();
    }

private:
    // Worker thread function - the heart of the thread pool
    void workerThread(size_t threadId) {
        std::cout << "Worker thread " << threadId << " started\n";
        
        while (true) {
            std::function<void()> task;
            
            {
                // Wait for a task or stop signal
                std::unique_lock<std::mutex> lock(queueMutex);
                
                // Wait until there's a task or we need to stop
                condition.wait(lock, [this] {
                    return stop || !tasks.empty();
                });
                
                // Exit if stopped and no more tasks
                if (stop && tasks.empty()) {
                    std::cout << "Worker thread " << threadId << " exiting\n";
                    return;
                }
                
                // Get task from queue
                task = std::move(tasks.front());
                tasks.pop();
            }
            
            // Execute the task (outside the lock!)
            task();
        }
    }

    // Thread pool components
    std::vector<std::thread> workers;           // Worker threads
    std::queue<std::function<void()>> tasks;    // Task queue
    
    // Synchronization primitives
    std::mutex queueMutex;                      // Protects task queue
    std::condition_variable condition;          // For thread coordination
    bool stop;                                  // Stop flag
};

// ====================================================================================
// DEMONSTRATION EXAMPLES
// ====================================================================================

// Example 1: Simple task execution
void simpleTask(int id) {
    std::cout << "Task " << id << " executing on thread " 
              << std::this_thread::get_id() << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Task " << id << " completed\n";
}

// Example 2: Task with return value
int computeSquare(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return x * x;
}

// Example 3: More complex computation
long fibonacci(int n) {
    if (n <= 1) return n;
    if (n == 2) return 1;
    
    long a = 0, b = 1, c;
    for (int i = 2; i <= n; ++i) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

void demonstrateBasicUsage() {
    std::cout << "\n=== DEMONSTRATION 1: Basic Task Execution ===\n";
    
    ThreadPool pool(4);  // Create pool with 4 threads
    
    // Submit multiple tasks
    for (int i = 0; i < 8; ++i) {
        pool.enqueue(simpleTask, i);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void demonstrateReturnValues() {
    std::cout << "\n=== DEMONSTRATION 2: Tasks with Return Values ===\n";
    
    ThreadPool pool(3);
    std::vector<std::future<int>> results;
    
    // Submit tasks and collect futures
    for (int i = 1; i <= 10; ++i) {
        results.emplace_back(pool.enqueue(computeSquare, i));
    }
    
    // Get results
    for (size_t i = 0; i < results.size(); ++i) {
        int value = results[i].get();  // Blocks until result is ready
        std::cout << (i + 1) << "² = " << value << "\n";
    }
}

void demonstrateComplexTasks() {
    std::cout << "\n=== DEMONSTRATION 3: Complex Computations ===\n";
    
    ThreadPool pool(4);
    std::vector<std::future<long>> results;
    
    // Calculate Fibonacci numbers in parallel
    std::vector<int> numbers = {10, 15, 20, 25, 30, 35, 40};
    
    for (int n : numbers) {
        results.emplace_back(pool.enqueue(fibonacci, n));
    }
    
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "Fibonacci(" << numbers[i] << ") = " 
                  << results[i].get() << "\n";
    }
}

void demonstrateLambdaTasks() {
    std::cout << "\n=== DEMONSTRATION 4: Lambda Functions ===\n";
    
    ThreadPool pool(2);
    
    // Submit lambda tasks
    auto future1 = pool.enqueue([] {
        std::cout << "Lambda task 1 running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return "Result from lambda 1";
    });
    
    auto future2 = pool.enqueue([](int a, int b) {
        std::cout << "Lambda task 2: computing " << a << " + " << b << "\n";
        return a + b;
    }, 5, 7);
    
    std::cout << future1.get() << "\n";
    std::cout << "5 + 7 = " << future2.get() << "\n";
}

// ====================================================================================
// KEY DESIGN PATTERNS AND CONCEPTS
// ====================================================================================

void explainKeyConceptsTheoretically() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════╗
║                       THREAD POOL DESIGN CONCEPTS                            ║
╚══════════════════════════════════════════════════════════════════════════════╝

1. PRODUCER-CONSUMER PATTERN
   - Main thread = Producer (adds tasks)
   - Worker threads = Consumers (execute tasks)
   - Task queue = Shared buffer

2. SYNCHRONIZATION MECHANISMS
   - Mutex (std::mutex): Protects the task queue from race conditions
   - Condition Variable (std::condition_variable): Efficient thread waiting
     • wait(): Thread sleeps until notified
     • notify_one(): Wakes up one waiting thread
     • notify_all(): Wakes up all waiting threads

3. TASK QUEUE WORKFLOW
   ┌─────────────┐
   │ Submit Task │
   └──────┬──────┘
          ↓
   ┌─────────────┐      Lock mutex
   │ Lock Queue  │◄────────────────
   └──────┬──────┘
          ↓
   ┌─────────────┐
   │  Add Task   │
   └──────┬──────┘
          ↓
   ┌─────────────┐
   │   Unlock    │
   └──────┬──────┘
          ↓
   ┌─────────────┐      Wake waiting thread
   │   Notify    │◄────────────────
   └─────────────┘

4. WORKER THREAD LIFECYCLE
   START → WAIT (condition.wait()) → TASK AVAILABLE? → EXECUTE → WAIT
                     ↑                      │
                     │                      ↓
                     └──────── NO ──────  STOP?
                                            │ YES
                                            ↓
                                          EXIT

5. FUTURES AND PROMISES
   - std::packaged_task: Wraps any callable target
   - std::future: Allows retrieving the result later
   - Provides exception safety and type safety

6. RESOURCE MANAGEMENT (RAII)
   - Constructor: Creates threads
   - Destructor: Cleans up automatically
   - No manual memory management needed

7. PERFORMANCE CONSIDERATIONS
   - Thread count ≈ CPU cores (for CPU-bound tasks)
   - Thread count > CPU cores (for I/O-bound tasks)
   - Task granularity: Not too fine, not too coarse
   - Avoid lock contention: Execute tasks outside locks

)" << std::endl;
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║         THREAD POOL - Educational Tutorial            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    // Theoretical explanation
    explainKeyConceptsTheoretically();
    
    // Practical demonstrations
    demonstrateBasicUsage();
    demonstrateReturnValues();
    demonstrateComplexTasks();
    demonstrateLambdaTasks();
    
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║                Tutorial Complete!                      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
