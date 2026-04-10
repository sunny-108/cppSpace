/*
 * RestoreOperation.cpp
 *
 * Modern C++17 version of SAPHanaRestore::operationStream().
 *
 * Original flow per stream:
 *   1. Create context
 *   2. initStreamInstrumentation
 *   3. setStorageParameters (with objectName from inquiry)
 *   4. validateConfig
 *   5. validateStorageOperation → validate restore params
 *   6. doStorageOperation       → transfer restore data
 *   7. finishStorageOperation   → finalize
 *   8. Catch exceptions
 *   9. Destroy context
 *
 * Simplified: simulates work with sleep.
 */
#include "RestoreOperation.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

RestoreOperation::RestoreOperation(uint64_t maxParallel)
    : m_maxParallel(maxParallel)
{
}

void RestoreOperation::addFile(const std::string& fileName, const std::string& objectName) {
    m_entries.emplace_back(fileName, objectName);
}

void RestoreOperation::init() {
    std::cout << "[Restore] Initialized with " << m_entries.size() << " files." << std::endl;
}

uint64_t RestoreOperation::getNumberOfThreadsToRunInParallel() const {
    return m_maxParallel;
}

void RestoreOperation::operationStream(uint32_t index) {
    std::ostringstream oss;
    oss << "[Restore] Stream " << index
        << " (" << m_entries[index].fileName
        << " ← " << m_entries[index].objectName << ")"
        << " — thread: " << std::this_thread::get_id();
    std::cout << oss.str() << " — STARTED" << std::endl;

    // Simulate restore work (data transfer — typically slower than backup)
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Mark success
    {
        std::lock_guard<std::mutex> lock(entriesMutex);
        m_entries[index].success = true;
    }

    std::cout << oss.str() << " — DONE" << std::endl;
}
