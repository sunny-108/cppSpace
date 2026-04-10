/*
 * Operation.cpp
 *
 * Modern C++17 implementation of ISAPOperation's sliding-window thread pool.
 *
 * The core algorithm from ISAPOperation::runParallelOperation():
 *
 *   1. initRunParallelOperation() → validate entries, get max parallel count
 *   2. Allocate SAPOperationThread** array (new[])
 *   3. Start initial batch: for (i=0; i<numParallel; i++) startThreads(i)
 *   4. Sliding window:
 *        while (started < total) {
 *            if (isThreadFinished()) {  // scans for FINISHED status
 *                startThreads(next);
 *                started++;
 *            } else {
 *                Platform::sleepMillisec(1000);  // <-- 1 second poll!
 *            }
 *        }
 *   5. Final join: for all entries, if not COMPLETED → joinThreads()
 *   6. delete[] ppOperationThreads
 *
 * Modern C++17 changes:
 *   - SAPOperationThread** (raw new[]) → std::vector<std::unique_ptr<OperationThread>>
 *   - Platform::sleepMillisec(1000) → condition_variable::wait_for()
 *   - thrLock_t mutex.lock()/unlock() → std::lock_guard<std::mutex>
 *   - delete[] ppOperationThreads → automatic (vector + unique_ptr RAII)
 */
#include "IOperation.h"

#include <iostream>
#include <chrono>

bool IOperation::initParallelOperation(uint64_t& numParallel) {
    /*
     * Original ISAPOperation::initRunParallelOperation():
     *   if (0 == m_operationEntries.size()) { log error; return false; }
     *   numThreadsToRunInParallel = getNumberOfThreadsToRunInParallel();
     *   if (0 == numThreadsToRunInParallel) { log error; return false; }
     *   if (numThreadsToRunInParallel > m_operationEntries.size())
     *       numThreadsToRunInParallel = m_operationEntries.size();
     */
    if (m_entries.empty()) {
        std::cerr << "[Operation] Nothing to do — 0 entries." << std::endl;
        return false;
    }

    numParallel = getNumberOfThreadsToRunInParallel();
    if (numParallel == 0) {
        std::cerr << "[Operation] 0 parallel threads configured." << std::endl;
        return false;
    }

    // Don't use more threads than entries
    if (numParallel > m_entries.size()) {
        numParallel = m_entries.size();
    }

    std::cout << "[Operation] Entries: " << m_entries.size()
              << ", Parallel threads: " << numParallel << std::endl;
    return true;
}

bool IOperation::startThread(std::vector<std::unique_ptr<OperationThread>>& threads, uint64_t index) {
    /*
     * Original ISAPOperation::startThreads():
     *   m_operationEntries[threadId].objectFileMap.result = SAP_OPERATION_ERROR; // default
     *   ppOperationThreads[threadId] = new SAPOperationThread(this, threadId);
     *   operationEntriesMutex.lock();
     *   m_operationEntries[threadId].ThreadExecutionStatus = THREAD_EXECUTION_RUNNING;
     *   operationEntriesMutex.unlock();
     *   ppOperationThreads[threadId]->start();
     */
    if (index >= m_entries.size()) return false;

    std::cout << "[Operation] Starting thread for entry " << index
              << " (" << m_entries[index].fileName << ")" << std::endl;

    threads[index] = std::make_unique<OperationThread>(this, static_cast<uint32_t>(index));

    {
        std::lock_guard<std::mutex> lock(entriesMutex);
        m_entries[index].status = ThreadStatus::RUNNING;
    }

    threads[index]->start();
    return true;
}

bool IOperation::joinThread(std::vector<std::unique_ptr<OperationThread>>& threads, uint64_t index) {
    /*
     * Original ISAPOperation::joinThreads():
     *   ppOperationThreads[threadId]->join();
     *   delete ppOperationThreads[threadId];
     *   m_operationEntries[threadId].ThreadExecutionStatus = THREAD_EXECUTION_COMPLETED;
     */
    if (index >= m_entries.size()) return false;
    if (!threads[index]) return false;

    threads[index]->join();
    threads[index].reset();  // delete — original did: delete ppOperationThreads[threadId]

    m_entries[index].status = ThreadStatus::COMPLETED;

    std::cout << "[Operation] Joined thread for entry " << index
              << " (" << m_entries[index].fileName << ") — "
              << (m_entries[index].success ? "SUCCESS" : "ERROR") << std::endl;
    return true;
}

bool IOperation::findFinishedThread(std::vector<std::unique_ptr<OperationThread>>& threads) {
    /*
     * Original ISAPOperation::isThreadFinished():
     *   operationEntriesMutex.lock();
     *   for (i=0; i<entries; i++) {
     *       if (m_operationEntries[i].ThreadExecutionStatus == THREAD_EXECUTION_FINISHED) {
     *           joinThreads(opnThreads, i, fileOnlineOption);
     *           finished = true;
     *       }
     *   }
     *   operationEntriesMutex.unlock();
     *   return finished;
     *
     * Modern: Uses condition_variable wait instead of caller's Sleep(1000) poll.
     */
    std::unique_lock<std::mutex> lock(entriesMutex);

    // Wait until at least one thread has FINISHED status
    // Original: no wait here — caller did Sleep(1000) in a loop
    // Modern: condition_variable eliminates 1-second latency
    m_finishedCV.wait_for(lock, std::chrono::milliseconds(200), [this] {
        for (const auto& entry : m_entries) {
            if (entry.status == ThreadStatus::FINISHED) return true;
        }
        return false;
    });

    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].status == ThreadStatus::FINISHED) {
            lock.unlock();
            joinThread(threads, i);
            return true;
        }
    }
    return false;
}

bool IOperation::isCompleted(uint64_t index) {
    /*
     * Original ISAPOperation::getCompletionStatus():
     *   operationEntriesMutex.lock();
     *   if (status != COMPLETED && status != NOT_STARTED) completion = false;
     *   operationEntriesMutex.unlock();
     */
    std::lock_guard<std::mutex> lock(entriesMutex);
    return m_entries[index].status == ThreadStatus::COMPLETED
        || m_entries[index].status == ThreadStatus::NOT_STARTED;
}

void IOperation::runParallelOperation() {
    /*
     * Original ISAPOperation::runParallelOperation(bool fileOnline, bool& softError, bool waitForThread):
     *
     * PHASE 1: Init
     *   initRunParallelOperation(numThreadsToRunInParallel);
     *   ppOperationThreads = new SAPOperationThread*[m_operationEntries.size()];
     *
     * PHASE 2: Start initial batch
     *   for (i=0; i<numThreadsToRunInParallel; i++)
     *       startThreads(ppOperationThreads, i, waitForThreadStatus);
     *       numThreadsThatHaveBeenStarted++;
     *
     * PHASE 3: Sliding window
     *   while (numThreadsThatHaveBeenStarted < m_operationEntries.size()) {
     *       if (isThreadFinished(ppOperationThreads, ...)) {
     *           startThreads(ppOperationThreads, numThreadsThatHaveBeenStarted, ...);
     *           numThreadsThatHaveBeenStarted++;
     *       } else {
     *           Platform::sleepMillisec(THREAD_EXCECUTION_SLEEP_TIME_MSEC);  // 1000ms poll
     *       }
     *   }
     *
     * PHASE 4: Final join
     *   for (i=0; i<m_operationEntries.size(); i++) {
     *       if (!getCompletionStatus(i)) joinThreads(ppOperationThreads, i, ...);
     *   }
     *   delete[] ppOperationThreads;
     */

    uint64_t numParallel = 0;
    if (!initParallelOperation(numParallel)) return;

    // ===== PHASE 1: Allocate thread array =====
    // Original: SAPOperationThread **ppOperationThreads = new SAPOperationThread*[size];
    // Modern: vector of unique_ptr — automatic cleanup, no delete[] needed.
    std::vector<std::unique_ptr<OperationThread>> threads(m_entries.size());

    uint64_t numStarted = 0;

    // ===== PHASE 2: Start initial batch =====
    std::cout << "\n[Pool] === PHASE 2: Starting initial batch of "
              << numParallel << " threads ===" << std::endl;
    for (uint64_t i = 0; i < numParallel; ++i) {
        if (startThread(threads, i)) {
            ++numStarted;
        }
    }

    // ===== PHASE 3: Sliding window =====
    std::cout << "\n[Pool] === PHASE 3: Sliding window ===" << std::endl;
    while (numStarted < m_entries.size()) {
        if (findFinishedThread(threads)) {
            // A slot freed up — start next pending entry
            if (startThread(threads, numStarted)) {
                ++numStarted;
            }
        }
        // Original had: Platform::sleepMillisec(1000) in the else branch
        // Modern: findFinishedThread() already uses condition_variable wait
    }

    // ===== PHASE 4: Final join =====
    std::cout << "\n[Pool] === PHASE 4: Final join ===" << std::endl;
    for (uint64_t i = 0; i < m_entries.size(); ++i) {
        if (!isCompleted(i)) {
            joinThread(threads, i);
        }
    }

    // Original: delete[] ppOperationThreads;
    // Modern: vector<unique_ptr> auto-cleans. Nothing to do.
    std::cout << "[Pool] All operations complete." << std::endl;
}

void IOperation::finish() {
    std::cout << "[Operation] Finished. Result: "
              << (m_operationResult.load() ? "SUCCESS" : "ERRORS OCCURRED") << std::endl;
}
