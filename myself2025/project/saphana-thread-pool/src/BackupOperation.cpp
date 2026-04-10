/*
 * BackupOperation.cpp
 *
 * Modern C++17 version of SAPHanaBackup::operationStream().
 *
 * Original flow per stream:
 *   1. Create context (HANA_CONTEXT_MGR->createContext)
 *   2. initStreamInstrumentation (start timer)
 *   3. generateObjectName (unique name with mutex-protected retry)
 *   4. setStorageParameters
 *   5. validateConfig           → calls PluginController
 *   6. validateStorageOperation → validate backup params
 *   7. doStorageOperation       → transfer backup data (I/O heavy)
 *   8. finishStorageOperation   → finalize & report
 *   9. Catch exceptions → set result to ERROR
 *  10. Destroy context
 *
 * Simplified: simulates work with sleep.
 */
#include "BackupOperation.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

BackupOperation::BackupOperation(uint64_t maxParallel)
    : m_maxParallel(maxParallel)
{
}

void BackupOperation::addFile(const std::string& fileName, uint64_t size) {
    m_entries.emplace_back(fileName, "", size);
}

void BackupOperation::init() {
    /*
     * Original ISAPOperation::init():
     *   m_pIPCCommand->processInputFile();
     *   m_operationEntries = m_pIPCCommand->getOperationEntries();
     */
    std::cout << "[Backup] Initialized with " << m_entries.size() << " files." << std::endl;
}

uint64_t BackupOperation::getNumberOfThreadsToRunInParallel() const {
    /*
     * Original: queries Catalyst server for DataSessionsLimit and FreeDataSessions.
     * Applies: config limit, 60% rule (SAP_PARALLEL_STREAMS_DATA_SESSION_LIMIT),
     * and per-stream data session multiplier.
     *
     * Simplified: returns configured max.
     */
    return m_maxParallel;
}

void BackupOperation::operationStream(uint32_t index) {
    /*
     * Original SAPHanaBackup::operationStream(index):
     *   PluginCommandContext* ctx = HANA_CONTEXT_MGR->createContext();
     *   initStreamInstrumentation(ctx);
     *   generateObjectName(sequencerString, objectName);
     *   setStorageParameters(entry, location, options, params);
     *   validateConfig(ctx, location, options, entry);
     *   validateStorageOperation(ctx, entry, location, options, params);
     *   doStorageOperation(ctx, entry, location, options, params);       ← actual data transfer
     *   finishStorageOperation(ctx, entry, location, options, params);
     */
    std::ostringstream oss;
    oss << "[Backup] Stream " << index
        << " (" << m_entries[index].fileName << ")"
        << " — thread: " << std::this_thread::get_id();
    std::cout << oss.str() << " — STARTED" << std::endl;

    // Simulate backup work (data transfer)
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Mark success
    {
        std::lock_guard<std::mutex> lock(entriesMutex);
        m_entries[index].success = true;
        m_entries[index].objectName = "obj_" + std::to_string(index);
    }

    std::cout << oss.str() << " — DONE" << std::endl;
}
