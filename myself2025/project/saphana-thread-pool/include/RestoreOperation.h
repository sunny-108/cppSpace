/*
 * RestoreOperation.h
 *
 * Modern C++17 version of SAPHanaRestore.
 *
 * Original: SAPHanaRestore extends ISAPHanaOperation extends ISAPOperation
 *   - runOperation() { runParallelOperation(false, softError, true); }
 *   - operationStream(index) { ... validateConfig; doStorageOperation;
 *       finishStorageOperation; ... }
 *
 * Simplified for practice.
 */
#ifndef RESTORE_OPERATION_H
#define RESTORE_OPERATION_H

#include "IOperation.h"
#include <cstdint>

class RestoreOperation : public IOperation {
public:
    explicit RestoreOperation(uint64_t maxParallel = 3);

    void init() override;
    void operationStream(uint32_t index) override;
    uint64_t getNumberOfThreadsToRunInParallel() const override;

    void addFile(const std::string& fileName, const std::string& objectName);

private:
    uint64_t m_maxParallel;
};

#endif // RESTORE_OPERATION_H
