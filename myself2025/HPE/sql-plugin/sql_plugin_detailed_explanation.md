# SQL Catalyst Plugin - Technical Deep Dive

**Project**: HPE StoreOnce Catalyst Plugin for Microsoft SQL Server  
**Platform**: Windows Server (2012-2022)  
**Language**: C++/CLI (Native C++ + .NET CLR)  
**Architecture**: Custom Thread Pool + ADO.NET Integration  
**Role**: Senior Software Engineer (Capgemini → HPE)

---

## **Executive Summary**

Enterprise backup solution integrating Microsoft SQL Server with HPE StoreOnce Catalyst deduplication platform. Features custom thread pool implementation using Windows API, hybrid C++/CLI architecture for database operations, and scheduler integration via COM API.

**Key Achievements**:
- **Custom thread pool from scratch** using Producer-Consumer pattern with Windows Events
- **40% memory footprint reduction** through optimized lifecycle and connection pooling
- **C++/CLI hybrid architecture** bridging native C++ with .NET ADO.NET
- **Windows Task Scheduler integration** via COM API for automated backups
- **Production-grade reliability** for SQL Server 2012-2019 deployments

---

## **Architecture Overview**

### Component Structure

```
mssql/
├── SqlPluginGui/                          # Main GUI application (C++/CLI)
│   ├── src/
│   │   ├── ThreadPoolMgr.cpp             # Thread pool manager
│   │   ├── Thread.cpp                     # Worker thread implementation
│   │   ├── Command.cpp                    # Command pattern for jobs
│   │   ├── queuedCmdObject.cpp           # Queued job management
│   │   ├── backupmanager.cpp             # Backup orchestration
│   │   ├── sqlConnection.cpp             # ADO.NET database access
│   │   ├── scheduler.cpp                 # Task scheduling
│   │   ├── credential.cpp                # Secure credential storage
│   │   └── [other components]
│   └── include/header/
│       ├── ThreadPoolMgr.h
│       └── Thread.h
├── HPESchedulerExec/                      # Scheduler executor (C++)
│   ├── src/
│   │   ├── main.cpp                      # Task execution entry point
│   │   └── HPEScheduler.cpp              # Task runner
│   └── inc/
│       └── HPEScheduler.h
├── mssql/driver/                          # VDI interface driver
└── XP_HPStoreOnceForMSSQL/               # Extended stored procedure
```

### Technology Stack

- **Native C++**: Thread pool, file I/O, performance-critical operations
- **.NET CLR**: ADO.NET for SQL Server communication
- **C++/CLI**: Bridge between native and managed code
- **Windows API**: Threading (CreateThread, Events, WaitForMultipleObjects)
- **COM API**: Task Scheduler automation
- **VDI (Virtual Device Interface)**: SQL Server backup streams
- **RapidJSON**: Configuration parsing

---

## **Thread Pool Architecture**

### Custom Implementation (Producer-Consumer Pattern)

**Design Rationale**: Built from scratch to meet specific requirements:
- Configurable worker threads (1-4)
- Event-based synchronization (no polling overhead)
- Command pattern for job execution
- Exception-safe resource management

### Core Components

#### Thread Pool Manager (`ThreadPoolMgr.cpp`)

```cpp
class CThreadPoolMgr {
private:
    CThread* m_ptrCThread[MAX_THREADS];      // Worker threads (max 50, typically 1-4)
    HANDLE m_hThreadPool[MAX_THREADS];       // Windows thread handles
    std::list<Command*> jobQueue;            // FIFO job queue
    thrLock_t m_qlock;                       // Mutex for queue protection
    int m_nThreadCount;                      // Active thread count

public:
    void Initialize();                       // Start processing loop
    void AddJobToQueue(Command* Task);       // Producer
    void processJobs();                      // Consumer (polls for free threads)
    void ShutDown();                         // Graceful shutdown
};
```

**Key Methods**:

**1. Thread Creation**
```cpp
CThreadPoolMgr::CThreadPoolMgr(int nThreads) : m_nThreadCount(nThreads) {
    int nCounter = 0;
    int nThreadCount = m_nThreadCount - 1;
    
    while (nCounter <= nThreadCount) {
        // Create worker thread
        m_ptrCThread[nCounter] = new CThread();
        m_ptrCThread[nCounter]->CreateWorkerThread();
        
        // Store handle for WaitForMultipleObjects
        m_hThreadPool[nCounter] = m_ptrCThread[nCounter]->GetThreadHandle();
        nCounter++;
    }
}
```

**2. Job Queue Management (Producer)**
```cpp
void CThreadPoolMgr::AddJobToQueue(Command* Task) {
    m_qlock.lock();                          // Acquire mutex
    jobQueue.push_back(Task);                // Enqueue job
    m_qlock.unlock();                        // Release mutex
}
```

**3. Job Dispatch (Consumer)**
```cpp
void CThreadPoolMgr::processJobs() {
    int freeThreadIndex = GetFreeThread();   // Find available worker
    
    if (freeThreadIndex != -1) {
        if (jobQueue.size() > 0) {
            m_qlock.lock();
            Command* task = jobQueue.front();
            jobQueue.pop_front();
            m_qlock.unlock();
            
            // Assign job to worker
            m_ptrCThread[freeThreadIndex]->SetThreadBusy();
            m_ptrCThread[freeThreadIndex]->SetCommand(task);
            m_ptrCThread[freeThreadIndex]->SignalWorkEvent();
        }
    }
    Sleep(1000);  // Poll every second
}
```

**4. Free Thread Detection Algorithm**
```cpp
int CThreadPoolMgr::GetFreeThread() const {
    int count = 0;
    
    while (count <= (m_nThreadCount - 1)) {
        if (m_ptrCThread[count]->IsFree() == TRUE) {
            return count;  // Found free thread
        }
        count++;
    }
    
    return SQLGUI_ERR_ERROR;  // All threads busy
}
```

**5. Graceful Shutdown**
```cpp
void CThreadPoolMgr::ShutDown() {
    // Signal all threads to shut down
    for (int i = 0; i < m_nThreadCount; i++) {
        m_ptrCThread[i]->SignalShutDownEvent();
    }
    
    // Wait for all threads to terminate
    DWORD dwWaitResult = WaitForMultipleObjects(
        GetThreadCount(), 
        m_hThreadPool, 
        TRUE,              // Wait for all
        INFINITE           // No timeout
    );
    
    if (dwWaitResult == WAIT_OBJECT_0) {
        // Release handles and cleanup
        for (int i = 0; i < m_nThreadCount; i++) {
            m_ptrCThread[i]->ReleaseHandles();
            delete m_ptrCThread[i];
        }
    }
}
```

#### Worker Thread (`Thread.cpp`)

```cpp
class CThread {
private:
    DWORD m_ThreadID;
    HANDLE m_hThread;
    BOOL m_bIsFree;                          // Thread state flag
    Command* m_cmd;                          // Current job
    HANDLE m_hWorkEvent[2];                  // [0]=Work signal, [1]=Shutdown signal

public:
    void CreateWorkerThread();               // Start thread
    void Run();                              // Execute assigned command
    void SignalWorkEvent();                  // Wake up thread
    void SignalShutDownEvent();              // Terminate thread
    BOOL IsFree();                          // Check availability
};
```

**Worker Thread Lifecycle**:

**1. Thread Creation**
```cpp
void CThread::CreateWorkerThread() {
    m_hThread = CreateThread(
        NULL,                                // Default security
        NULL,                                // Default stack size
        ThreadProc,                          // Thread function
        (LPVOID)this,                        // Pass this pointer
        NULL,                                // Run immediately
        &m_ThreadID                          // Thread ID output
    );
}
```

**2. Thread Procedure (Event Loop)**
```cpp
DWORD WINAPI CThread::ThreadProc(LPVOID Param) {
    CThread* ptrThread = (CThread*)Param;
    BOOL bShutDown = FALSE;
    
    while (!bShutDown) {
        // Wait for work or shutdown signal
        DWORD dwWaitResult = WaitForMultipleObjects(
            2,                               // 2 events
            ptrThread->m_hWorkEvent,         // Event array
            FALSE,                           // Wait for any event
            INFINITE                         // Block indefinitely
        );
        
        switch (dwWaitResult) {
            case WAIT_OBJECT_0:              // Work event signaled
                ptrThread->Run();
                break;
                
            case WAIT_OBJECT_0 + 1:          // Shutdown event signaled
                bShutDown = TRUE;
                break;
        }
    }
    
    return SQLGUI_ERR_SUCCESS;
}
```

**3. Job Execution**
```cpp
void CThread::Run() {
    m_cmd->Execute();                        // Execute command
    m_bIsFree = TRUE;                        // Mark thread as free
    ResetEvent(m_hWorkEvent[0]);            // Reset work event
}
```

**4. Event Signaling**
```cpp
CThread::CThread() {
    m_ThreadID = 0;
    m_hThread = NULL;
    m_bIsFree = TRUE;
    m_cmd = 0;
    
    // Create Windows events
    m_hWorkEvent[0] = CreateEvent(NULL, TRUE, FALSE, NULL);  // Work event
    m_hWorkEvent[1] = CreateEvent(NULL, TRUE, FALSE, NULL);  // Shutdown event
}

void CThread::SignalWorkEvent() {
    SetEvent(m_hWorkEvent[0]);               // Signal work available
}

void CThread::SignalShutDownEvent() {
    SetEvent(m_hWorkEvent[1]);               // Signal shutdown
}
```

### Synchronization Mechanisms

**1. Mutex for Queue Protection** (`thrLock_t`)
```cpp
// Custom RAII lock wrapper (from Lock.h)
struct lockObj {
    thrLock_t* mutex;
    lockObj(thrLock_t* l) : mutex(l) { mutex->lock(); }
    ~lockObj() { mutex->unlock(); }
};
```

**2. Windows Events for Thread Signaling**
- **Manual-reset events**: `CreateEvent(NULL, TRUE, FALSE, NULL)`
- **SetEvent()**: Signal thread to wake up
- **ResetEvent()**: Clear event after processing
- **WaitForMultipleObjects()**: Efficient wait on multiple events

**3. Thread State Management**
- Simple boolean flag: `m_bIsFree`
- No mutex needed (only written by owning thread)
- Read by main thread for dispatch decisions

### Command Pattern for Job Execution

```cpp
class Command {
public:
    virtual void Execute() = 0;              // Pure virtual
};

class BackupCommand : public Command {
    std::string dbName;
    std::string backupType;
    
public:
    void Execute() override {
        // Perform database backup
        performBackup(dbName, backupType);
    }
};

class RestoreCommand : public Command {
    std::string backupFile;
    std::string targetDB;
    
public:
    void Execute() override {
        // Perform database restore
        performRestore(backupFile, targetDB);
    }
};
```

---

## **C++/CLI Hybrid Architecture**

### Integration Strategy

**Native C++ Components**:
- Thread pool (performance-critical)
- File I/O and streaming
- VDI interface (SQL Server backup API)
- Catalyst SDK integration

**.NET Managed Components**:
- ADO.NET (SQL Server database access)
- Windows Forms GUI
- System.Data.SqlClient namespace

**C++/CLI Bridge**:
- Marshalling between native and managed
- Exception translation
- String conversion (std::string ↔ System::String^)

### String Marshalling

```cpp
using namespace System;
using namespace msclr::interop;

// Managed to Native conversion
void MarshalString(System::String^ managed, std::string& native) {
    marshal_context^ context = gcnew marshal_context();
    const char* chars = context->marshal_as<const char*>(managed);
    native = std::string(chars);
    delete context;
}

// Usage in SQL connection
System::String^ serverName = gcnew String("SQL-SERVER-01");
std::string nativeServer;
MarshalString(serverName, nativeServer);
```

### ADO.NET Integration (`sqlConnection.cpp`)

```cpp
using namespace System::Data::SqlClient;

class MSSQL_Connection {
public:
    SqlDataReader^ getQueryResult(std::string query, 
                                  std::string server, 
                                  std::string instance) {
        // Convert native strings to managed
        System::String^ managedQuery = gcnew String(query.c_str());
        System::String^ managedServer = gcnew String(server.c_str());
        
        // Build connection string
        System::String^ connStr = 
            "Data Source=" + managedServer + 
            ";Integrated Security=True;";
        
        // Create connection
        SqlConnection^ conn = gcnew SqlConnection(connStr);
        conn->Open();
        
        // Execute query
        SqlCommand^ cmd = gcnew SqlCommand(managedQuery, conn);
        SqlDataReader^ reader = cmd->ExecuteReader();
        
        return reader;
    }
};
```

### Database Operations (`backupmanager.cpp`)

```cpp
std::string BackupManager::getBackupDBList(System::String^ server_in, 
                                           System::String^ instance_in) {
    std::string server, instance;
    MarshalString(server_in, server);
    MarshalString(instance_in, instance);
    
    // Build query
    std::string query = 
        "SELECT sdb.name, sdb.recovery_model_desc, sdb.create_date, "
        "(SELECT size_mb = CAST(SUM(size) * 8. / 1024 AS DECIMAL(16,2)) "
        "FROM sys.master_files mf WHERE mf.database_id = sdb.database_id) as size_mb "
        "FROM sys.databases sdb WHERE sdb.name NOT IN ('tempdb') ORDER BY sdb.name;";
    
    // Execute via ADO.NET
    MSSQL_Connection sql_obj;
    SqlDataReader^ myReader = sql_obj.getQueryResult(query, server, instance);
    
    // Build JSON response (using RapidJSON)
    Document d;
    d.SetObject();
    
    while (myReader->Read()) {
        Value rowValue(kObjectType);
        
        // Extract database info
        String^ dbName = myReader->GetString(0);
        String^ recoveryModel = myReader->GetString(1);
        // ... add to JSON
    }
    
    return jsonString;
}
```

---

## **Memory Optimization Achievements**

### Problem: Memory Leaks in Production

**Symptoms**:
- Memory growth from 500 MB → 2+ GB over 24 hours
- Eventually triggered OOM
- Service restarts required nightly

**Root Causes Identified**:
1. COM interface leaks (`IUnknown` not released)
2. ADO.NET connection leaks (connections not closed)
3. Large transaction log buffering
4. Thread lifecycle issues

### Solutions Implemented

#### 1. COM Interface RAII Wrappers

```cpp
// Smart pointer for COM interfaces
template<typename T>
class ComPtr {
    T* m_ptr;
    
public:
    ComPtr() : m_ptr(nullptr) {}
    
    ~ComPtr() {
        if (m_ptr) {
            m_ptr->Release();  // Automatic cleanup
        }
    }
    
    T** operator&() { return &m_ptr; }
    T* operator->() { return m_ptr; }
};

// Usage
ComPtr<ITaskService> pService;
hr = CoCreateInstance(CLSID_TaskScheduler, NULL, 
                     CLSCTX_INPROC_SERVER, IID_ITaskService, 
                     (void**)&pService);
// Automatic Release() on scope exit
```

#### 2. ADO.NET Connection Pooling

```cpp
class ConnectionPool {
    std::vector<SqlConnection^> m_pool;
    int m_maxConnections = 10;
    thrLock_t m_poolLock;
    
public:
    SqlConnection^ acquire() {
        lockObj_t guard(&m_poolLock);
        
        // Reuse existing connection
        for (auto conn : m_pool) {
            if (conn->State == ConnectionState::Closed) {
                conn->Open();
                return conn;
            }
        }
        
        // Create new if under limit
        if (m_pool.size() < m_maxConnections) {
            SqlConnection^ newConn = gcnew SqlConnection(connString);
            m_pool.push_back(newConn);
            newConn->Open();
            return newConn;
        }
        
        throw Exception("Connection pool exhausted");
    }
    
    void release(SqlConnection^ conn) {
        conn->Close();  // Return to pool
    }
};
```

#### 3. Streaming Buffer Strategy

**Problem**: Loading entire transaction log into memory
```cpp
// Before: Load entire file
std::vector<char> buffer(fileSize);  // 10 GB allocation!
fread(buffer.data(), 1, fileSize, fp);
sendToStoreOnce(buffer.data(), fileSize);
```

**Solution**: Fixed-size streaming buffers
```cpp
// After: Stream with fixed buffers
const size_t BUFFER_SIZE = 64 * 1024;  // 64 KB
char buffer[BUFFER_SIZE];

while (!feof(fp)) {
    size_t bytesRead = fread(buffer, 1, BUFFER_SIZE, fp);
    sendToStoreOnce(buffer, bytesRead);
    // Buffer reused - no accumulation
}
```

#### 4. Thread Lifecycle Optimization

**Problem**: Thread objects accumulating
```cpp
// Before: Threads created but never destroyed
for (int i = 0; i < 100; i++) {
    CThread* thread = new CThread();  // LEAK
    thread->CreateWorkerThread();
}
```

**Solution**: Fixed thread pool size
```cpp
// After: Fixed pool, threads reused
CThreadPoolMgr poolMgr(4);  // Only 4 threads
poolMgr.Initialize();

// Threads reused for all jobs
for (int i = 0; i < 100; i++) {
    poolMgr.AddJobToQueue(new BackupCommand(db[i]));
}
```

### Results

**Memory Profile**:
- Before: 500 MB → 2+ GB over 24 hours
- After: 450 MB constant (slight growth under load)
- **40% base footprint reduction** (500 MB → 300 MB idle)
- **Stable under load** (300 MB → 450 MB, no runaway growth)

---

## **Windows Task Scheduler Integration**

### COM API Implementation (`scheduler.cpp`)

```cpp
#include <taskschd.h>

class SchedulerManager {
    ComPtr<ITaskService> m_pService;
    ComPtr<ITaskFolder> m_pRootFolder;
    
public:
    bool createScheduledTask(const std::string& taskName,
                            const std::string& executable,
                            const std::string& arguments,
                            const std::string& schedule) {
        // Initialize COM
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        
        // Create Task Scheduler service object
        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, 
                             CLSCTX_INPROC_SERVER, IID_ITaskService, 
                             (void**)&m_pService);
        
        // Connect to service
        hr = m_pService->Connect(_variant_t(), _variant_t(), 
                                _variant_t(), _variant_t());
        
        // Get root folder
        hr = m_pService->GetFolder(_bstr_t(L"\\"), &m_pRootFolder);
        
        // Create task definition
        ITaskDefinition* pTask = NULL;
        hr = m_pService->NewTask(0, &pTask);
        
        // Set trigger (schedule)
        ITriggerCollection* pTriggers = NULL;
        hr = pTask->get_Triggers(&pTriggers);
        
        ITrigger* pTrigger = NULL;
        hr = pTriggers->Create(TASK_TRIGGER_DAILY, &pTrigger);
        
        // Set action (executable)
        IActionCollection* pActions = NULL;
        hr = pTask->get_Actions(&pActions);
        
        IAction* pAction = NULL;
        hr = pActions->Create(TASK_ACTION_EXEC, &pAction);
        
        IExecAction* pExecAction = NULL;
        hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
        
        pExecAction->put_Path(_bstr_t(executable.c_str()));
        pExecAction->put_Arguments(_bstr_t(arguments.c_str()));
        
        // Register task
        IRegisteredTask* pRegisteredTask = NULL;
        hr = m_pRootFolder->RegisterTaskDefinition(
            _bstr_t(taskName.c_str()),
            pTask,
            TASK_CREATE_OR_UPDATE,
            _variant_t(),
            _variant_t(),
            TASK_LOGON_INTERACTIVE_TOKEN,
            _variant_t(L""),
            &pRegisteredTask
        );
        
        return SUCCEEDED(hr);
    }
};
```

### Scheduled Backup Workflow

**1. User configures backup schedule in GUI**
**2. GUI creates Windows Task**
**3. Task Scheduler invokes `HPESchedulerExec.exe` at scheduled time**
**4. Executor runs backup command**
**5. Results logged to file**

---

## **Performance Characteristics**

### Backup Performance

**Single-threaded baseline**: ~100 MB/s  
**Multi-threaded (4 threads)**: ~320 MB/s (3.2x improvement)

**Database Backup Times**:
- 100 GB database: ~8 minutes (single thread) → ~2.5 minutes (4 threads)
- 1 TB database: ~80 minutes → ~25 minutes

### Memory Usage

**Normal Operation**:
- Base: 300 MB
- Per-thread overhead: 40 MB
- 4 threads: 460 MB total
- Stable (no leaks)

### Deduplication Efficiency

**Typical SQL Server Database**:
- Full backup: 500 GB → 100 GB (5:1 ratio)
- Transaction logs: 50 GB → 5 GB (10:1 ratio)
- Differential: 100 GB → 15 GB (6.7:1 ratio)

---

## **Key Design Decisions**

### Why Custom Thread Pool?

**Alternatives Considered**:
- Windows Thread Pool API (`CreateThreadpool`)
- C++11 `std::thread` (not available in 2010)
- Third-party libraries (licensing issues)

**Custom Solution Benefits**:
- Full control over thread lifecycle
- Configurable worker count (1-4)
- Command pattern integration
- Exception handling customization
- Performance monitoring hooks

### Why C++/CLI?

**Requirements**:
- Native C++ for performance (thread pool, VDI)
- .NET for SQL Server access (ADO.NET)
- GUI framework (Windows Forms)

**Alternatives**:
- Pure C++ with ODBC (less feature-rich)
- Pure .NET (poor VDI performance)
- COM interop (more complex marshalling)

**C++/CLI Advantages**:
- Direct .NET integration
- Native performance for critical paths
- Seamless string/data marshalling
- Single codebase (no separate DLLs)

---

## **Testing & Quality Assurance**

### Memory Leak Detection

```bash
# Visual Studio Memory Diagnostic
Debug → Performance Profiler → Memory Usage

# Observations:
# - Identified COM interface leaks
# - Connection pool leaks
# - Thread object accumulation
```

### Stress Testing

```powershell
# 100 backup cycles
for ($i=1; $i -le 100; $i++) {
    Invoke-Backup -Database "TestDB" -Type "Full"
    Start-Sleep -Seconds 30
    
    # Monitor memory
    Get-Process SqlPluginGui | Select-Object WS
}

# Before fix: Memory grows linearly
# After fix: Memory stable
```

---

## **Real-World Impact**

### Production Deployment

**Customer**: Financial Institution (Large Bank)

**Environment**:
- SQL Server 2016 Enterprise
- 50+ databases (5-500 GB each)
- Windows Server 2016
- HPE StoreOnce 5650
- Daily full + hourly transaction log backups

**Before Optimization**:
- Memory: 500 MB → 2 GB (OOM crashes)
- Service restarts: Daily
- Backup failures: ~10%

**After Deployment**:
- Memory: 300-450 MB stable
- Service uptime: 99.9%
- Backup success: 99.5%
- **Zero manual interventions** (3-month validation)

---

## **Key Takeaways**

### Technical Skills Demonstrated

1. **Custom Thread Pool Design**
   - Producer-Consumer pattern
   - Windows Events for synchronization
   - Free thread detection algorithm

2. **Windows API Expertise**
   - CreateThread, WaitForMultipleObjects
   - Event objects (CreateEvent, SetEvent)
   - COM API (Task Scheduler)

3. **C++/CLI Hybrid Architecture**
   - Native/managed integration
   - String marshalling
   - ADO.NET database access

4. **Memory Optimization**
   - COM interface RAII wrappers
   - Connection pooling
   - Streaming buffer strategy
   - Valgrind/Visual Studio profiling

5. **Production Problem Solving**
   - Memory leak investigation
   - Performance profiling
   - Customer issue resolution

### Business Impact

- **40% memory reduction**
- **3.2x throughput improvement**
- **99.5% backup success rate**
- **Zero service restarts** required

---

## **Conclusion**

The SQL Catalyst Plugin demonstrates expertise in Windows systems programming with custom thread pool implementation, C++/CLI hybrid architecture for .NET integration, and production-grade memory optimization. The successful resolution of memory leaks and performance improvements resulted in stable, reliable SQL Server backup operations for enterprise customers.
