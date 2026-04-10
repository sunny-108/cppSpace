/*
 * ThreadPoolMgr.cpp
 *
 * Modern C++17 implementation of CThreadPoolMgr.
 *
 * Key modernization from original SqlPluginGui:
 *
 *   ORIGINAL (Windows API)                         MODERN (C++17)
 *   ─────────────────────────────────────────────────────────────────────────
 *   CThread* m_ptrCThread[MAX_THREADS]          →  std::vector<std::unique_ptr<CThread>>
 *   HANDLE m_hThreadPool[MAX_THREADS]           →  eliminated (Join() each thread)
 *   new CThread() / delete m_ptrCThread[i]      →  std::make_unique<CThread>()
 *   std::list<Command*> jobQueue                →  std::list<std::unique_ptr<Command>>
 *   thrLock_t m_qlock (custom platform lock)    →  std::mutex
 *   lockObj_t (custom RAII)                     →  std::lock_guard / std::unique_lock
 *   Sleep(1000) busy-polling in processJobs()   →  m_queueCV.wait() with notification
 *   CreateThread for dispatcher in processTasks →  std::thread for dispatcher
 *   WaitForMultipleObjects(n, handles, TRUE,..) →  for-each thread.Join()
 */
#include "ThreadPoolMgr.h"

#include <iostream>

CThreadPoolMgr::CThreadPoolMgr(int nThreads)
    : m_nThreadCount(nThreads)
{
    /*
     * Original:
     *   while (nCounter <= nThreadCount) {
     *       m_ptrCThread[nCounter] = new CThread();
     *       m_ptrCThread[nCounter]->CreateWorkerThread();
     *       m_hThreadPool[nCounter] = m_ptrCThread[nCounter]->GetThreadHandle();
     *       nCounter++;
     *   }
     *
     * Modern: smart pointers, no handle array needed.
     */
    m_threads.reserve(m_nThreadCount);
    for (int i = 0; i < m_nThreadCount; ++i) {
        auto thread = std::make_unique<CThread>();
        thread->CreateWorkerThread();
        m_threads.push_back(std::move(thread));
    }
}

CThreadPoolMgr::~CThreadPoolMgr() {
    if (m_running.load()) {
        ShutDown();
    }
}

void CThreadPoolMgr::Initialize() {
    /*
     * Original: processTasks() spawned a dispatcher thread via CreateThread.
     * Modern: launch dispatcher as std::thread.
     */
    m_running.store(true);
    m_dispatcherThread = std::thread(&CThreadPoolMgr::processJobs, this);
}

void CThreadPoolMgr::processJobs() {
    /*
     * Original (busy-polling):
     *   while (true) {
     *       int Count = GetFreeThread();
     *       if (Count != -1) {
     *           if (jobQueue.size() > 0) {
     *               m_qlock.lock();
     *               Command *Task = jobQueue.front();
     *               jobQueue.pop_front();
     *               m_qlock.unlock();
     *               m_ptrCThread[Count]->SetThreadBusy();
     *               m_ptrCThread[Count]->SetCommand(Task);
     *               m_ptrCThread[Count]->SignalWorkEvent();
     *           }
     *       }
     *       Sleep(1000);  // <-- 1 second busy-wait!
     *   }
     *
     * Modern: condition_variable eliminates busy-polling.
     * The dispatcher wakes up only when a new job is enqueued or shutdown is requested.
     */
    while (m_running.load()) {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        // Wait until there's a job or we're shutting down
        m_queueCV.wait(lock, [this] {
            return !m_jobQueue.empty() || !m_running.load();
        });

        if (!m_running.load() && m_jobQueue.empty()) {
            break;
        }

        // Find a free thread (same logic as original GetFreeThread)
        int freeIdx = GetFreeThread();
        if (freeIdx == -1) {
            // All threads busy — release lock briefly, then retry
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Dequeue the job
        auto task = std::move(m_jobQueue.front());
        m_jobQueue.pop_front();
        lock.unlock();

        // Dispatch to the free worker thread
        m_threads[freeIdx]->SetThreadBusy();
        m_threads[freeIdx]->SetCommand(task.release());  // Worker takes raw ptr; job lifetime is within Execute()
        m_threads[freeIdx]->SignalWorkEvent();
    }
}

void CThreadPoolMgr::AddJobToQueue(std::unique_ptr<Command> task) {
    /*
     * Original:
     *   m_qlock.lock();
     *   jobQueue.push_back(Task);
     *   m_qlock.unlock();
     *
     * Modern: std::lock_guard + condition_variable notify.
     */
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_jobQueue.push_back(std::move(task));
    }
    m_queueCV.notify_one();
}

void CThreadPoolMgr::ShutDown() {
    /*
     * Original:
     *   - Signal shutdown event on each worker thread
     *   - WaitForMultipleObjects(n, m_hThreadPool, TRUE, INFINITE)
     *   - CloseHandle + delete each CThread
     *
     * Modern:
     *   - Set running flag to false, notify dispatcher
     *   - Signal shutdown on each worker, then join each
     *   - Smart pointers auto-cleanup
     */
    m_running.store(false);
    m_queueCV.notify_all();

    // Join the dispatcher thread
    if (m_dispatcherThread.joinable()) {
        m_dispatcherThread.join();
    }

    // Signal shutdown to all worker threads and join them
    for (auto& thread : m_threads) {
        thread->SignalShutDownEvent();
    }
    for (auto& thread : m_threads) {
        thread->Join();
    }

    // unique_ptr automatically deletes CThread objects when vector is destroyed
    m_threads.clear();
}

int CThreadPoolMgr::GetFreeThread() const {
    /*
     * Original:
     *   while (count <= m_nThreadCount - 1) {
     *       if (m_ptrCThread[count]->IsFree() == TRUE) return count;
     *       count++;
     *   }
     *   cerr << "All thread are busy..." << endl;
     *   return SQLGUI_ERR_ERROR;  // -1
     */
    for (int i = 0; i < static_cast<int>(m_threads.size()); ++i) {
        if (m_threads[i]->IsFree()) {
            return i;
        }
    }
    return -1;
}

int CThreadPoolMgr::GetThreadCount() const {
    return m_nThreadCount;
}
