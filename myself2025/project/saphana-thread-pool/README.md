# Sliding-Window Thread Pool — Practice Project

Based on the **SAP HANA Catalyst Plugin** thread pool from StoreOnce (`ISAPOperation::runParallelOperation`).

---

## Architecture: Sliding Window vs Queue-Based

This is a fundamentally **different pattern** from the SqlPluginGui thread pool:

| | SqlPluginGui (Windows) | SAP HANA (Linux) |
|---|---|---|
| **Pattern** | Queue-based pool with persistent workers | Sliding-window with per-entry threads |
| **Thread lifetime** | Created once at startup, reused for many jobs | Created per operation entry, destroyed after join |
| **Work source** | Unbounded job queue (`std::list<Command*>`) | Fixed array of entries (`std::vector<sOperationEntry>`) |
| **Dispatcher** | Dedicated dispatcher thread polls queue | Main thread runs sliding window loop |
| **Parallelism control** | Fixed worker count (configurable 1-4) | Server-capacity-based (60% of DataSessionsLimit) |
| **Completion tracking** | `m_bIsFree` flag per worker | `eThreadExecutionStatus` per entry (4 states) |
| **Thread reuse** | Worker sleeps and waits for next command | Thread is destroyed; new thread created for next entry |

### Sliding Window Algorithm
```
Entries:  [0] [1] [2] [3] [4] [5] [6] [7]    (8 files to backup)
Window:    ^^^^^^^^^^^                          (3 parallel slots)

Step 1: Start entries 0, 1, 2
Step 2: Entry 0 finishes → join it → start entry 3
Step 3: Entry 1 finishes → join it → start entry 4
...until all entries processed
Final:  Join any remaining running threads
```

---

## Modernization Map

| Original (pthread / SAP HANA) | Modern (C++17) |
|---|---|
| `PluginThread` (pthread wrapper base class) | Direct `std::thread` member |
| `internal_CreateThread()` → `pthread_create()` | `std::thread(&OperationThread::threadFunc, this)` |
| `internal_JoinThread()` → `pthread_join()` | `m_thread.join()` |
| `pthread_mutex_t` / `thrLock_t` | `std::mutex` + `std::lock_guard` |
| `bool m_running` (not atomic) | `std::atomic<bool> m_running` |
| `eThreadExecutionStatus` (C-style enum) | `enum class ThreadStatus` |
| `SAPOperationThread** = new ...[]` / `delete[]` | `std::vector<std::unique_ptr<OperationThread>>` |
| `Platform::sleepMillisec(1000)` polling | `std::condition_variable::wait_for()` |
| `sOperationEntry` (complex SAP struct) | `OperationEntry` (simplified) |
| `ISAPOperation` (virtual base class) | `IOperation` (virtual base class) |

### Thread State Machine (preserved)
```
NOT_STARTED ──start()──→ RUNNING ──operationStream()──→ FINISHED ──join()──→ COMPLETED
```

---

## Build & Run

```bash
cd project/saphana-thread-pool
make
./sliding_pool_demo
```

---

## Source File Mapping

| Modern File | Original SAP HANA File |
|---|---|
| `include/OperationTypes.h` | `sap/sapcommon/public/inc/SAPCommonTypes.hpp` |
| `include/OperationThread.h` | `sap/sapcommon/public/inc/ISAPOperation.hpp` (SAPOperationThread class) |
| `include/IOperation.h` | `sap/sapcommon/public/inc/ISAPOperation.hpp` (ISAPOperation class) |
| `include/BackupOperation.h` | `sap/hana/saphana/plugin/src/SAPHanaBackup.hpp` |
| `include/RestoreOperation.h` | `sap/hana/saphana/plugin/src/SAPHanaRestore.hpp` |
| `src/OperationThread.cpp` | `PluginThread.cpp` + ISAPOperation.hpp (run method) |
| `src/Operation.cpp` | `sap/sapcommon/sapcommon/src/ISAPOperation.cpp` |
| `src/BackupOperation.cpp` | `sap/hana/saphana/plugin/src/SAPHanaBackup.cpp` |
| `src/RestoreOperation.cpp` | `sap/hana/saphana/plugin/src/SAPHanaRestore.cpp` |
| `src/main.cpp` | Backint CLI entry point |
