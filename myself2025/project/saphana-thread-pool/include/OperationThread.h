/*
 * OperationThread.h
 *
 * Modern C++17 version of SAPOperationThread + PluginThread from SAP HANA plugin.
 *
 * Original (pthread-based):
 *   - PluginThread: base class wrapping pthread_create/pthread_join/pthread_mutex
 *     - PLUGINTHREAD_HANDLE m_handle (pthread_t)
 *     - bool m_running, m_started
 *     - PLUGINTHREAD_CMUTEX_HANDLE m_mutex (pthread_mutex_t)
 *     - static ThreadFunc(void*) → calls run()
 *     - start() → internal_CreateThread(pthread_create)
 *     - join()  → internal_JoinThread(pthread_join)
 *
 *   - SAPOperationThread extends PluginThread:
 *     - ISAPOperation* m_pOperation
 *     - uint32_t m_index
 *     - run() { m_pOperation->operationStream(m_index); lock(); set FINISHED; unlock(); }
 *
 * Modern C++17:
 *   - PluginThread  → std::thread (no base class needed)
 *   - pthread_create → std::thread constructor
 *   - pthread_join   → std::thread::join()
 *   - pthread_mutex  → std::mutex (used in parent Operation class)
 *   - m_running      → std::atomic<bool>
 *   - Raw pointer to ISAPOperation → raw pointer to IOperation (non-owning, parent owns)
 */
#ifndef OPERATION_THREAD_H
#define OPERATION_THREAD_H

#include "OperationTypes.h"

#include <thread>
#include <atomic>
#include <cstdint>

// Forward declaration — replaces ISAPOperation*
class IOperation;

class OperationThread {
public:
    OperationThread(IOperation* pOperation, uint32_t index);
    ~OperationThread();

    // Non-copyable, non-movable (owns a running thread)
    OperationThread(const OperationThread&) = delete;
    OperationThread& operator=(const OperationThread&) = delete;

    /*
     * Original: PluginThread::start() → internal_CreateThread(&m_handle, stackSize, ThreadFunc, this)
     * Modern:   m_thread = std::thread(&OperationThread::threadFunc, this)
     */
    void start();

    /*
     * Original: PluginThread::join() → internal_JoinThread(m_handle)
     * Modern:   m_thread.join()
     */
    void join();

    bool isRunning() const { return m_running.load(); }
    bool isStarted() const { return m_started.load(); }

private:
    /*
     * Original: static PLUGINTHREAD_FUNCTION_RETURN ThreadFunc(PLUGINTHREAD_LPVOID p)
     *           { PluginThread* t = (PluginThread*)p; t->run(); return NULL; }
     *
     *           SAPOperationThread::run()
     *           { m_pOperation->operationStream(m_index);
     *             lock(operationEntriesMutex);
     *             m_pOperation->m_operationEntries[m_index].ThreadExecutionStatus = FINISHED;
     *             unlock(operationEntriesMutex); }
     *
     * Modern: member function, no casts needed.
     */
    void threadFunc();

    IOperation*         m_pOperation;   // Non-owning — parent Operation object
    uint32_t            m_index;
    std::thread         m_thread;
    std::atomic<bool>   m_running{false};
    std::atomic<bool>   m_started{false};
};

#endif // OPERATION_THREAD_H
