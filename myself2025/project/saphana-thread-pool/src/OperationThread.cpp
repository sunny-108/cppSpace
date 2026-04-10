/*
 * OperationThread.cpp
 *
 * Modern C++17 implementation replacing PluginThread + SAPOperationThread.
 *
 * Original chain:
 *   PluginThread::start()
 *     → internal_CreateThread(&m_handle, stackSize, ThreadFunc, this)
 *       → pthread_create(&m_handle, &attr, ThreadFunc, this)
 *
 *   static ThreadFunc(void* p)
 *     → ((PluginThread*)p)->m_running = true;
 *     → ((PluginThread*)p)->run();
 *     → ((PluginThread*)p)->m_running = false;
 *     → return NULL;
 *
 *   SAPOperationThread::run()
 *     → m_pOperation->operationStream(m_index);
 *     → lock(operationEntriesMutex);
 *     → m_pOperation->m_operationEntries[m_index].ThreadExecutionStatus = FINISHED;
 *     → unlock(operationEntriesMutex);
 *
 * Modern C++17:
 *   start()     → m_thread = std::thread(&OperationThread::threadFunc, this)
 *   threadFunc  → operationStream(index); lock(); set FINISHED; unlock(); notify_one();
 *   join()      → m_thread.join()
 */
#include "OperationThread.h"
#include "IOperation.h"

OperationThread::OperationThread(IOperation* pOperation, uint32_t index)
    : m_pOperation(pOperation), m_index(index)
{
}

OperationThread::~OperationThread() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void OperationThread::start() {
    /*
     * Original PluginThread::start():
     *   int rc = internal_CreateThread(&m_handle, m_stackSize, ThreadFunc, this);
     *   if (rc == PLUGIN_THREAD_SUCCESS) m_started = true;
     *
     * internal_CreateThread (Linux):
     *   pthread_attr_init(&attr);
     *   pthread_attr_setstacksize(&attr, stackSize);
     *   pthread_create(pThreadHandle, &attr, pThreadFunction, pThreadArgument);
     */
    m_started.store(true);
    m_thread = std::thread(&OperationThread::threadFunc, this);
}

void OperationThread::join() {
    /*
     * Original: internal_JoinThread(m_handle) → pthread_join(threadHandle, NULL)
     */
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void OperationThread::threadFunc() {
    /*
     * Original ThreadFunc (static):
     *   PluginThread* thread = (PluginThread*)p;
     *   thread->m_running = true;
     *   thread->run();           // calls SAPOperationThread::run()
     *   thread->m_running = false;
     *   return NULL;
     *
     * Original SAPOperationThread::run():
     *   m_pOperation->operationStream(m_index);
     *   operationEntriesMutex.lock();
     *   m_pOperation->m_operationEntries[m_index].ThreadExecutionStatus = THREAD_EXECUTION_FINISHED;
     *   operationEntriesMutex.unlock();
     */
    m_running.store(true);

    // Run the actual operation (backup/restore stream for this entry)
    m_pOperation->operationStream(m_index);

    // Lock mutex and set status to FINISHED (same as original)
    {
        std::lock_guard<std::mutex> lock(m_pOperation->entriesMutex);
        m_pOperation->m_entries[m_index].status = ThreadStatus::FINISHED;
    }
    // Notify the dispatcher that a thread has finished
    // Original: no notification — dispatcher polled with Sleep(1000)
    // Modern: condition_variable eliminates the 1-second polling delay
    m_pOperation->m_finishedCV.notify_one();

    m_running.store(false);
}
