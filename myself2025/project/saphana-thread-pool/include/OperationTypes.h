/*
 * OperationTypes.h
 *
 * Modern C++17 version of SAPCommonTypes.hpp
 *
 * Original:
 *   - eThreadExecutionStatus enum (NOT_STARTED, RUNNING, FINISHED, COMPLETED)
 *   - sOperationEntry struct with ThreadExecutionStatus field
 *   - sObjectFileMap struct with fileName, objectName, result, etc.
 *
 * Modern C++17:
 *   - enum class (scoped) instead of C-style enum
 *   - Simplified OperationEntry for practice (no SAP-specific fields)
 */
#ifndef OPERATION_TYPES_H
#define OPERATION_TYPES_H

#include <string>

/*
 * Original: eThreadExecutionStatus (unscoped enum in SAPCommonTypes.hpp)
 *   THREAD_EXECUTION_NOT_STARTED = 0,
 *   THREAD_EXECUTION_RUNNING     = 1,
 *   THREAD_EXECUTION_FINISHED    = 2,
 *   THREAD_EXECUTION_COMPLETED   = 3
 *
 * Modern: enum class for type safety — can't accidentally compare with int.
 */
enum class ThreadStatus {
    NOT_STARTED = 0,  // Thread has not been created yet
    RUNNING     = 1,  // Thread is running operationStream()
    FINISHED    = 2,  // Thread finished work, waiting to be joined
    COMPLETED   = 3   // Thread has been joined and cleaned up
};

/*
 * Original: sOperationEntry (SAPCommonTypes.hpp)
 *   - sObjectFileMap objectFileMap  (fileName, objectName, result, etc.)
 *   - uint64_t size
 *   - eThreadExecutionStatus ThreadExecutionStatus
 *   - std::string sequencerString, backupTime, etc.
 *
 * Simplified for practice — keeps the essential fields.
 */
struct OperationEntry {
    std::string fileName;
    std::string objectName;
    uint64_t    size{0};
    bool        success{false};
    ThreadStatus status{ThreadStatus::NOT_STARTED};

    OperationEntry() = default;
    explicit OperationEntry(const std::string& file, const std::string& obj = "", uint64_t sz = 0)
        : fileName(file), objectName(obj), size(sz) {}
};

#endif // OPERATION_TYPES_H
