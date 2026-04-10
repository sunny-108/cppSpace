/*
 * BackupOperation.h
 *
 * Modern C++17 version of SAPHanaBackup.
 *
 * Original: SAPHanaBackup extends ISAPHanaOperation extends ISAPOperation
 *   - runOperation() { runParallelOperation(false, softError); }
 *   - operationStream(index) { ... generateObjectName; validateConfig;
 *       doStorageOperation; finishStorageOperation; ... }
 *   - setNumberOfDataSessionsPerStream(1)
 *
 * Simplified for practice — simulates backup work in operationStream().
 */
#ifndef BACKUP_OPERATION_H
#define BACKUP_OPERATION_H

#include "IOperation.h"
#include <cstdint>

class BackupOperation : public IOperation {
public:
    explicit BackupOperation(uint64_t maxParallel = 3);

    void init() override;
    void operationStream(uint32_t index) override;
    uint64_t getNumberOfThreadsToRunInParallel() const override;

    void addFile(const std::string& fileName, uint64_t size = 0);

private:
    uint64_t m_maxParallel;
};

#endif // BACKUP_OPERATION_H
