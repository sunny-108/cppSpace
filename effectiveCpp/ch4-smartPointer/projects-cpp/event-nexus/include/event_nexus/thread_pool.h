#pragma once

/// @file thread_pool.h
/// @brief Fixed-size thread pool with task queue.
///
/// Smart pointer usage — Pimpl idiom (Item 22):
///   - ThreadPool holds unique_ptr<Impl> (exclusive ownership).
///   - Destructor, move ops declared in header, defined in .cpp.
///   - Tasks are submitted as std::function wrapped in unique_ptr (Item 18).

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <type_traits>

namespace nexus {

class ThreadPool {
public:
    /// Create a pool with `numThreads` worker threads.
    /// If numThreads == 0, uses std::thread::hardware_concurrency().
    explicit ThreadPool(std::size_t numThreads = 0);

    // ── Item 22: special members declared here, defined in .cpp ──
    ~ThreadPool();
    ThreadPool(ThreadPool&& rhs) noexcept;
    ThreadPool& operator=(ThreadPool&& rhs) noexcept;

    // Non-copyable (unique ownership of threads).
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// Submit a callable and get a future for its result.
    /// Task ownership: the callable is moved into a unique_ptr-wrapped type-erased
    /// task internally (Item 18 — unique_ptr for exclusive ownership).
    template <typename F, typename... Args>
    auto submit(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    /// Number of worker threads.
    [[nodiscard]] std::size_t size() const noexcept;

    /// Number of tasks waiting in the queue.
    [[nodiscard]] std::size_t pendingTasks() const noexcept;

    /// Graceful shutdown — finishes queued tasks, then joins threads.
    void shutdown();

private:
    /// Enqueue a type-erased task (called by submit()).
    void enqueueTask(std::unique_ptr<std::function<void()>> task);

    struct Impl;                           // forward declaration
    std::unique_ptr<Impl> impl_;           // Pimpl (Item 22)
};

// ─── Template implementation ────────────────────────────────────────
// Must be in the header because it's a template, but delegates to
// enqueueTask() which is defined in the .cpp (Impl is complete there).

template <typename F, typename... Args>
auto ThreadPool::submit(F&& func, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using ReturnType = std::invoke_result_t<F, Args...>;

    auto boundTask = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...));

    std::future<ReturnType> future = boundTask->get_future();

    auto wrapper = std::make_unique<std::function<void()>>(
        [task = std::move(boundTask)]() { (*task)(); });

    enqueueTask(std::move(wrapper));
    return future;
}

}  // namespace nexus
