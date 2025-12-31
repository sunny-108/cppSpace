#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class SimpleThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

    // Worker thread function
    void worker_thread() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                
                // Wait until there's a task or stop is signaled
                this->condition.wait(lock, [this] {
                    return this->stop || !this->tasks.empty();
                });
                
                // Exit if stopping and no tasks left
                if (this->stop && this->tasks.empty()) {
                    return;
                }
                
                // Get task from queue
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }
            
            // Execute the task
            task();
        }
    }

public:
    // Constructor: creates specified number of worker threads
    SimpleThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back(&SimpleThreadPool::worker_thread, this);
        }
    }

    // Add a task to the queue
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            // Don't allow enqueueing after stopping the pool
            if (stop) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
            
            tasks.emplace([task]() { (*task)(); });
        }
        
        condition.notify_one();
        return result;
    }

    // Destructor: waits for all tasks to complete
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        
        condition.notify_all();
        
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};

// Example usage
int main() {
    // Create a thread pool with 4 worker threads
    SimpleThreadPool pool(4);
    
    std::vector<std::future<int>> results;
    
    // Enqueue 8 tasks
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "Task " << i << " started on thread " 
                          << std::this_thread::get_id() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "Task " << i << " completed" << std::endl;
                return i * i;
            })
        );
    }
    
    // Get results
    std::cout << "\nResults:" << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "Task " << i << " result: " << results[i].get() << std::endl;
    }
    
    return 0;
}
