/*
 * main.cpp
 *
 * Demo: Modern C++17 Thread Pool — based on SqlPluginGui's original Windows API implementation.
 *
 * This demonstrates the same architecture patterns:
 *   - Producer-Consumer: main thread produces jobs → dispatcher consumes and dispatches
 *   - Command pattern: BackupCmd, RestoreCmd, ListJobsCmd
 *   - Object Pool: fixed set of worker threads reused across jobs
 *   - Free-thread detection: dispatcher scans worker pool for idle thread
 */
#include "ThreadPoolMgr.h"
#include "Command.h"

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== Modern C++17 Thread Pool (from SqlPluginGui) ===" << std::endl;
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
    std::cout << std::endl;

    // Create thread pool with 3 worker threads (original default: OSSQL_DEFAULT_NTHREAD = 1, configurable 1-4)
    const int numThreads = 3;
    CThreadPoolMgr pool(numThreads);
    std::cout << "Thread pool created with " << pool.GetThreadCount() << " worker threads." << std::endl;

    // Start the dispatcher thread (original: pool.Initialize() → processTasks())
    pool.Initialize();
    std::cout << "Dispatcher thread started." << std::endl;
    std::cout << std::endl;

    // Enqueue jobs — Producer side (original: AddJobToQueue from GUI/scheduler)
    pool.AddJobToQueue(std::make_unique<BackupCmd>("BACKUP DB=TestDB TO=/store1"));
    pool.AddJobToQueue(std::make_unique<RestoreCmd>("RESTORE DB=TestDB FROM=/store1"));
    pool.AddJobToQueue(std::make_unique<ListJobsCmd>("LIST JOBS"));
    pool.AddJobToQueue(std::make_unique<BackupCmd>("BACKUP DB=ProdDB TO=/store2"));
    pool.AddJobToQueue(std::make_unique<RestoreCmd>("RESTORE DB=ProdDB FROM=/store2"));
    pool.AddJobToQueue(std::make_unique<ListJobsCmd>("LIST ALL"));

    std::cout << "All 6 jobs enqueued. Waiting for completion..." << std::endl;
    std::cout << std::endl;

    // Wait for jobs to finish (in real system, UI or scheduler manages this)
    std::this_thread::sleep_for(std::chrono::seconds(12));

    // Graceful shutdown (original: pool.ShutDown() → signal all + WaitForMultipleObjects)
    std::cout << std::endl;
    std::cout << "Shutting down thread pool..." << std::endl;
    pool.ShutDown();
    std::cout << "Thread pool shut down successfully." << std::endl;

    return 0;
}
