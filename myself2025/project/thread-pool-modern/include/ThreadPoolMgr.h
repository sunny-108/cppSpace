/*
 * ThreadPoolMgr.h
 *
 * Modern C++17 version of CThreadPoolMgr from SqlPluginGui.
 *
 * Original (Windows API):
 *   - CThread* m_ptrCThread[MAX_THREADS]  → std::vector<std::unique_ptr<CThread>>
 *   - HANDLE m_hThreadPool[MAX_THREADS]   → eliminated (use thread.Join())
 *   - std::list<Command*> jobQueue        → std::list<std::unique_ptr<Command>>
 *   - thrLock_t m_qlock (custom)          → std::mutex
 *   - Sleep(1000) busy-polling            → std::condition_variable wait with notify
 *   - WaitForMultipleObjects              → thread::join() in loop
 *   - processTasks() spawns a separate
 *     dispatcher thread via CreateThread  → std::thread for dispatcher
 *
 * Design Patterns preserved:
 *   - Producer-Consumer: AddJobToQueue (producer) → processJobs (consumer)
 *   - Object Pool: fixed set of worker threads reused for multiple jobs
 *   - Command: abstract Command dispatched to workers
 */
#ifndef THREADPOOLMGR_H
#define THREADPOOLMGR_H

#include "Thread.h"

#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <thread>

class CThreadPoolMgr {
public:
    explicit CThreadPoolMgr(int nThreads);
    ~CThreadPoolMgr();

    // Non-copyable, non-movable
    CThreadPoolMgr(const CThreadPoolMgr&) = delete;
    CThreadPoolMgr& operator=(const CThreadPoolMgr&) = delete;

    void Initialize();
    void AddJobToQueue(std::unique_ptr<Command> task);
    void ShutDown();
    int  GetFreeThread() const;
    int  GetThreadCount() const;

private:
    void processJobs();  // Consumer loop (runs on dispatcher thread)

    std::vector<std::unique_ptr<CThread>> m_threads;
    int m_nThreadCount;

    // Job queue with its own mutex + condition variable
    // Replaces: std::list<Command*> + thrLock_t + Sleep(1000) polling
    std::list<std::unique_ptr<Command>> m_jobQueue;
    mutable std::mutex                  m_queueMutex;
    std::condition_variable             m_queueCV;

    // Dispatcher thread (replaces processTasks() CreateThread call)
    std::thread      m_dispatcherThread;
    std::atomic<bool> m_running{false};
};

#endif // THREADPOOLMGR_H
