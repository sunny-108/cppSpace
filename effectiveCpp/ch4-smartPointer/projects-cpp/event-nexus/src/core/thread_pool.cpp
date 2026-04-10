/// @file thread_pool.cpp
/// @brief ThreadPool Pimpl implementation.
///
/// Item 22 in action: Impl is defined here (complete type).
/// Destructor and move operations use = default here — Impl is visible.

#include <event_nexus/thread_pool.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace nexus {

// ─── Impl definition (complete type — only visible in this TU) ──────
struct ThreadPool::Impl {
    std::vector<std::thread> workers;
    std::queue<std::unique_ptr<std::function<void()>>> taskQueue;
    std::mutex mutex;
    std::condition_variable condition;
    bool stopping = false;

    void workerLoop() {
        while (true) {
            std::unique_ptr<std::function<void()>> task;
            {
                std::unique_lock lock(mutex);
                condition.wait(lock, [this] { return stopping || !taskQueue.empty(); });

                if (stopping && taskQueue.empty()) {
                    return;
                }

                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
            (*task)();
        }
    }
};

// ─── Constructor ────────────────────────────────────────────────────
ThreadPool::ThreadPool(std::size_t numThreads)
    : impl_(std::make_unique<Impl>())  // Item 21: make_unique, not new
{
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4;  // fallback
    }

    impl_->workers.reserve(numThreads);
    for (std::size_t i = 0; i < numThreads; ++i) {
        impl_->workers.emplace_back([this] { impl_->workerLoop(); });
    }
}

// ─── Item 22: special members defined here (Impl is complete) ───────
ThreadPool::~ThreadPool() {
    if (impl_) {
        shutdown();
    }
}

ThreadPool::ThreadPool(ThreadPool&&) noexcept = default;
ThreadPool& ThreadPool::operator=(ThreadPool&&) noexcept = default;

// ─── Public methods ─────────────────────────────────────────────────
void ThreadPool::enqueueTask(std::unique_ptr<std::function<void()>> task) {
    {
        std::lock_guard lock(impl_->mutex);
        if (impl_->stopping) {
            throw std::runtime_error("Cannot enqueue on a stopped ThreadPool");
        }
        impl_->taskQueue.push(std::move(task));  // unique_ptr ownership transfer
    }
    impl_->condition.notify_one();
}

std::size_t ThreadPool::size() const noexcept {
    return impl_->workers.size();
}

std::size_t ThreadPool::pendingTasks() const noexcept {
    std::lock_guard lock(impl_->mutex);
    return impl_->taskQueue.size();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard lock(impl_->mutex);
        if (impl_->stopping) return;
        impl_->stopping = true;
    }
    impl_->condition.notify_all();
    for (auto& worker : impl_->workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

}  // namespace nexus
