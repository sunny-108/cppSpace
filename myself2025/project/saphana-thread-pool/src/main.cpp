/*
 * main.cpp
 *
 * Demo: Modern C++17 Sliding-Window Thread Pool
 * Based on SAP HANA plugin's ISAPOperation::runParallelOperation()
 *
 * Architecture:
 *   - Unlike a traditional queue-based thread pool (SqlPluginGui), this uses a
 *     SLIDING WINDOW pattern:
 *     - Fixed array of operation entries (files/pipes to backup/restore)
 *     - Limited parallel slots (determined by server capacity)
 *     - When a slot finishes, the next pending entry takes its place
 *   - Each entry = one backup/restore stream processed by one thread
 *   - Thread lifecycle: NOT_STARTED → RUNNING → FINISHED → COMPLETED
 *
 * Comparison with SqlPluginGui thread pool:
 *   SqlPluginGui (Windows)              SAP HANA (Linux)
 *   ─────────────────────────────────────────────────────
 *   Queue-based (unbounded)             Array-based (bounded by entries)
 *   Persistent worker threads           Threads created per entry, joined when done
 *   Dispatcher thread polls queue       Sliding window scans for FINISHED
 *   Windows Events + WaitForMultiple    pthread_create / pthread_join
 *   Command pattern (polymorphic)       Virtual operationStream(index)
 *   Sleep(1000) in dispatcher           Sleep(1000) in window loop
 */
#include "BackupOperation.h"
#include "RestoreOperation.h"

#include <iostream>

int main() {
    std::cout << "=== Modern C++17 Sliding-Window Thread Pool (from SAP HANA) ===" << std::endl;
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
    std::cout << std::endl;

    // ===== BACKUP DEMO =====
    // Simulates: backint -f backup -i /input_file -o /output_file
    // 8 files with max 3 parallel streams (like SAP HANA multi-stream backup)
    {
        std::cout << "────────────────────────────────────────" << std::endl;
        std::cout << "  BACKUP: 8 files, 3 parallel streams"   << std::endl;
        std::cout << "────────────────────────────────────────" << std::endl;

        BackupOperation backup(/*maxParallel=*/3);

        // Simulate SAP HANA sending 8 pipe entries via backint interface
        backup.addFile("/hana/data/SYSTEMDB/datafile_001.dat", 1024 * 1024);
        backup.addFile("/hana/data/SYSTEMDB/datafile_002.dat", 2048 * 1024);
        backup.addFile("/hana/data/SYSTEMDB/datafile_003.dat", 512 * 1024);
        backup.addFile("/hana/data/HDB00/datafile_001.dat",    4096 * 1024);
        backup.addFile("/hana/data/HDB00/datafile_002.dat",    768 * 1024);
        backup.addFile("/hana/data/HDB00/datafile_003.dat",    1536 * 1024);
        backup.addFile("/hana/log/SYSTEMDB/logfile_001.dat",   256 * 1024);
        backup.addFile("/hana/log/HDB00/logfile_001.dat",      128 * 1024);

        backup.init();
        backup.runParallelOperation();
        backup.finish();
    }

    std::cout << std::endl;

    // ===== RESTORE DEMO =====
    // 5 files with max 2 parallel streams
    {
        std::cout << "────────────────────────────────────────" << std::endl;
        std::cout << "  RESTORE: 5 files, 2 parallel streams"  << std::endl;
        std::cout << "────────────────────────────────────────" << std::endl;

        RestoreOperation restore(/*maxParallel=*/2);

        restore.addFile("/hana/data/SYSTEMDB/datafile_001.dat", "obj_0");
        restore.addFile("/hana/data/SYSTEMDB/datafile_002.dat", "obj_1");
        restore.addFile("/hana/data/SYSTEMDB/datafile_003.dat", "obj_2");
        restore.addFile("/hana/data/HDB00/datafile_001.dat",    "obj_3");
        restore.addFile("/hana/data/HDB00/datafile_002.dat",    "obj_4");

        restore.init();
        restore.runParallelOperation();
        restore.finish();
    }

    return 0;
}
