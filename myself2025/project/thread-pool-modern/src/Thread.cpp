/*
 * Thread.cpp
 *
 * Modern C++17 implementation of CThread.
 *
 * Key modernization from original SqlPluginGui:
 *
 *   ORIGINAL (Windows API)                    MODERN (C++17)
 *   ─────────────────────────────────────────────────────────────────
 *   CreateEvent(NULL, TRUE, FALSE, NULL)   →  std::condition_variable + bool flags
 *   SetEvent(m_hWorkEvent[0])              →  { m_hasWork = true; m_cv.notify_one(); }
 *   SetEvent(m_hWorkEvent[1])              →  { m_shutdown = true; m_cv.notify_one(); }
 *   ResetEvent(m_hWorkEvent[0])            →  m_hasWork = false (under lock)
 *   WaitForMultipleObjects(2, events, ..)  →  m_cv.wait(lock, [&]{ return m_hasWork || m_shutdown; })
 *   CreateThread(NULL,..,ThreadProc,..)    →  std::thread(&CThread::ThreadProc, this)
 *   CloseHandle(m_hThread)                 →  m_thread.join()
 *   BOOL m_bIsFree                         →  std::atomic<bool> m_bIsFree
 */
#include "Thread.h"

#include <iostream>

CThread::CThread() = default;

CThread::~CThread() {
    // Ensure thread is joined if still running
    if (m_thread.joinable()) {
        SignalShutDownEvent();
        m_thread.join();
    }
}

void CThread::CreateWorkerThread() {
    // Original: m_hThread = CreateThread(NULL, NULL, ThreadProc, (LPVOID)this, NULL, &m_ThreadID);
    m_thread = std::thread(&CThread::ThreadProc, this);
}

void CThread::ThreadProc() {
    /*
     * Original:
     *   while (!bShutDown) {
     *       DWORD dwWaitResult = WaitForMultipleObjects(2, m_hWorkEvent, FALSE, INFINITE);
     *       switch (dwWaitResult) {
     *           case WAIT_OBJECT_0:     ptrThread->Run(); break;
     *           case WAIT_OBJECT_0 + 1: bShutDown = TRUE; break;
     *       }
     *   }
     *
     * Modern: condition_variable replaces WaitForMultipleObjects.
     * The predicate checks both work and shutdown flags.
     */
    while (true) {
        std::unique_lock<std::mutex> lock(m_cvMutex);
        m_cv.wait(lock, [this] { return m_hasWork || m_shutdown; });

        if (m_shutdown) {
            break;
        }

        if (m_hasWork) {
            m_hasWork = false;   // Original: ResetEvent(m_hWorkEvent[0])
            lock.unlock();
            Run();
        }
    }
}

void CThread::Run() {
    // Original: m_cmd->Execute(); m_bIsFree = TRUE; ResetEvent(m_hWorkEvent[0]);
    if (m_cmd) {
        m_cmd->Execute();
    }
    m_bIsFree.store(true);
}

bool CThread::IsFree() const {
    // Original: return m_bIsFree;  (BOOL, not thread-safe)
    // Modern: std::atomic<bool> for lock-free thread-safe reads
    return m_bIsFree.load();
}

void CThread::SetCommand(Command* cmd) {
    m_cmd = cmd;
}

void CThread::SignalWorkEvent() {
    // Original: SetEvent(m_hWorkEvent[0]);
    {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        m_hasWork = true;
    }
    m_cv.notify_one();
}

void CThread::SignalShutDownEvent() {
    // Original: SetEvent(m_hWorkEvent[1]);
    {
        std::lock_guard<std::mutex> lock(m_cvMutex);
        m_shutdown = true;
    }
    m_cv.notify_one();
}

void CThread::SetThreadBusy() {
    // Original: m_bIsFree = FALSE;
    m_bIsFree.store(false);
}

void CThread::Join() {
    // Original: CloseHandle(m_hThread) + CloseHandle(m_hWorkEvent[0/1])
    // Modern: just join the thread; no handles to close
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

std::thread::id CThread::GetThreadID() const {
    return m_thread.get_id();
}
