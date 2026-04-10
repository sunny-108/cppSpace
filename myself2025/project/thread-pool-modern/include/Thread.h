/*
 * Thread.h
 *
 * Modern C++17 version of CThread from SqlPluginGui.
 *
 * Original (Windows API):
 *   - HANDLE m_hThread           → std::thread
 *   - DWORD m_ThreadID           → std::thread::id (obtained from std::thread)
 *   - HANDLE m_hWorkEvent[2]     → std::condition_variable + bool flags
 *   - CreateEvent/SetEvent/
 *     ResetEvent/WaitForMultiple  → condition_variable::notify/wait
 *   - BOOL m_bIsFree             → std::atomic<bool>
 *   - Command *m_cmd (raw)       → Command* (non-owning, pool manager owns the job)
 *   - thrLock_t (custom)         → std::mutex
 *   - lockObj_t (custom RAII)    → std::unique_lock / std::lock_guard
 *   - CreateThread()             → std::thread constructor
 *   - CloseHandle()              → std::thread::join()
 */
#ifndef THREAD_H
#define THREAD_H

#include "Command.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

class CThread {
public:
    CThread();
    ~CThread();

    // Non-copyable, non-movable (owns a running thread)
    CThread(const CThread&) = delete;
    CThread& operator=(const CThread&) = delete;

    void CreateWorkerThread();
    void SetCommand(Command* cmd);
    void SignalWorkEvent();
    void SignalShutDownEvent();
    void SetThreadBusy();
    bool IsFree() const;
    void Join();
    std::thread::id GetThreadID() const;

private:
    void ThreadProc();  // Replaces static DWORD WINAPI ThreadProc(LPVOID)
    void Run();

    std::thread             m_thread;
    std::atomic<bool>       m_bIsFree{true};
    Command*                m_cmd{nullptr};

    // Replaces HANDLE m_hWorkEvent[2] (work event + shutdown event)
    std::mutex              m_cvMutex;
    std::condition_variable m_cv;
    bool                    m_hasWork{false};
    bool                    m_shutdown{false};
};

#endif // THREAD_H
