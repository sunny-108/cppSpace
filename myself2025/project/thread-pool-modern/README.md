# Modern C++17 Thread Pool — Practice Project

Based on the **SqlPluginGui** thread pool from StoreOnce Catalyst SQL Plugin (HPE).  
Ref: CV Line 33 — *"Modern C++ Thread Pool Refactoring"*

---

## What This Project Demonstrates

The original SqlPluginGui used **Windows API** for threading. This project modernizes it to **portable C++17**, preserving the same architecture and design patterns.

### Modernization Map

| Original (Windows API) | Modern (C++17) |
|---|---|
| `CreateThread(NULL, ..., ThreadProc, ...)` | `std::thread(&CThread::ThreadProc, this)` |
| `CreateEvent / SetEvent / ResetEvent` | `std::condition_variable` + bool flags |
| `WaitForMultipleObjects(2, events, ...)` | `cv.wait(lock, predicate)` |
| `HANDLE m_hThread` | `std::thread m_thread` |
| `HANDLE m_hWorkEvent[2]` | `std::condition_variable m_cv` |
| `BOOL m_bIsFree` (not thread-safe) | `std::atomic<bool> m_bIsFree` |
| `CThread*` raw pointers + `new/delete` | `std::unique_ptr<CThread>` |
| `Command*` raw ownership | `std::unique_ptr<Command>` |
| `thrLock_t` (custom platform lock) | `std::mutex` |
| `lockObj_t` (custom RAII wrapper) | `std::lock_guard` / `std::unique_lock` |
| `Sleep(1000)` busy-polling | `condition_variable::wait()` with notify |
| `WaitForMultipleObjects(n, .., TRUE, ..)` | `thread.join()` loop |
| `CloseHandle()` | automatic (RAII / join) |
| `CThread* m_ptrCThread[MAX_THREADS]` | `std::vector<std::unique_ptr<CThread>>` |

### Design Patterns Preserved

- **Command Pattern** — `Command` base class with `BackupCmd`, `RestoreCmd`, `ListJobsCmd`
- **Producer-Consumer** — `AddJobToQueue()` (producer) → dispatcher `processJobs()` (consumer)
- **Object Pool** — Fixed set of worker threads reused across multiple jobs
- **Free-Thread Detection** — `GetFreeThread()` scans pool for idle worker

---

## Build & Run

```bash
cd project/thread-pool-modern
cmake -B build -S .
cmake --build build
./build/thread_pool_demo
```

## Source File Mapping

| Modern File | Original SqlPluginGui File |
|---|---|
| `include/Command.h` | `include/header/Command.h` |
| `include/Thread.h` | `include/header/Thread.h` |
| `include/ThreadPoolMgr.h` | `include/header/ThreadPoolMgr.h` |
| `src/Command.cpp` | `src/Command.cpp` (simplified) |
| `src/Thread.cpp` | `src/Thread.cpp` |
| `src/ThreadPoolMgr.cpp` | `src/ThreadPoolMgr.cpp` |
| `src/main.cpp` | `src/samplebrowse.cpp` (GUI entry) |
