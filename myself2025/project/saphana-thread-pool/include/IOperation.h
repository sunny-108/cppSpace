/*
 * IOperation.h
 *
 * Modern C++17 version of ISAPOperation from SAP HANA plugin.
 *
 * Original (ISAPOperation.hpp):
 *   - thrLock_t operationEntriesMutex                → std::mutex
 *   - std::vector<sOperationEntry> m_operationEntries → std::vector<OperationEntry>
 *   - bool m_operationResult                          → std::atomic<bool>
 *   - void runParallelOperation(bool, bool&, bool)    → void runParallelOperation()
 *   - virtual void operationStream(uint32_t) = 0     → same (pure virtual)
 *   - SAPOperationThread** ppOperationThreads (raw new[]) → std::vector<std::unique_ptr<OperationThread>>
 *
 * Sliding-window pool algorithm (preserved from original):
 *   1. Determine max parallel threads (getNumberOfThreadsToRunInParallel)
 *   2. Start initial batch of N threads
 *   3. Busy-loop: when a thread FINISHED → join it, start next pending entry
 *   4. Final join of any remaining threads
 *
 * Original polling: Platform::sleepMillisec(THREAD_EXCECUTION_SLEEP_TIME_MSEC) — 1000ms
 * Modern:           std::condition_variable (notified when thread finishes)
 *
 * Key methods mapped:
 *   initRunParallelOperation()  → initParallelOperation()
 *   startThreads()             → startThread()
 *   joinThreads()              → joinThread()
 *   isThreadFinished()         → findFinishedThread()
 *   getCompletionStatus()      → isCompleted()
 *   runParallelOperation()     → runParallelOperation()
 */
#ifndef IOPERATION_H
#define IOPERATION_H

#include "OperationTypes.h"
#include "OperationThread.h"

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstdint>
#include <string>

class IOperation {
public:
    // --- Public members matching ISAPOperation ---
    // Original: thrLock_t operationEntriesMutex → std::mutex
    std::mutex                      entriesMutex;
    // Original: std::vector<sOperationEntry> m_operationEntries
    std::vector<OperationEntry>     m_entries;

    // Condition variable to replace Sleep(1000) polling
    // When a thread finishes, it notifies — dispatcher wakes immediately.
    std::condition_variable         m_finishedCV;

    IOperation() = default;
    virtual ~IOperation() = default;

    /*
     * Original: virtual void operationStream(uint32_t index) = 0
     * Each concrete operation (Backup/Restore/Delete) implements this.
     * Called by SAPOperationThread::run() on a worker thread.
     */
    virtual void operationStream(uint32_t index) = 0;

    /*
     * Sliding-window dispatcher — the core of the SAP HANA thread pool.
     * Original: ISAPOperation::runParallelOperation()
     */
    void runParallelOperation();

    /*
     * Original: ISAPOperation::getNumberOfThreadsToRunInParallel()
     * In production, this queries the Catalyst server for data session limits.
     * Here it's configurable for practice.
     */
    virtual uint64_t getNumberOfThreadsToRunInParallel() const = 0;

    virtual void init() = 0;
    virtual void finish();

protected:
    std::atomic<bool> m_operationResult{true};

private:
    /*
     * Original: ISAPOperation::initRunParallelOperation(uint64_t&)
     * Validates entries and determines final thread count.
     */
    bool initParallelOperation(uint64_t& numParallel);

    /*
     * Original: ISAPOperation::startThreads(void**, uint64_t, bool)
     * Creates and starts a SAPOperationThread for the given entry index.
     */
    bool startThread(std::vector<std::unique_ptr<OperationThread>>& threads, uint64_t index);

    /*
     * Original: ISAPOperation::joinThreads(void**, uint64_t, bool)
     * Joins the thread, deletes it, marks entry as COMPLETED.
     */
    bool joinThread(std::vector<std::unique_ptr<OperationThread>>& threads, uint64_t index);

    /*
     * Original: ISAPOperation::isThreadFinished(void**, bool, bool&)
     * Scans entries for any with FINISHED status, joins it.
     * Returns true if a thread was found and joined.
     *
     * Modern: Uses condition_variable instead of Sleep(1000) polling.
     */
    bool findFinishedThread(std::vector<std::unique_ptr<OperationThread>>& threads);

    /*
     * Original: ISAPOperation::getCompletionStatus(uint64_t)
     */
    bool isCompleted(uint64_t index);
};

#endif // IOPERATION_H
