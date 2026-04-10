# SAP HANA Plugin - Command Sessions and Data Sessions

## Overview

The SAP HANA plugin communicates with the HPE StoreOnce Catalyst storage system through two distinct types of network sessions: **Command Sessions** and **Data Sessions**. Understanding the purpose, lifecycle, and interaction of these sessions is crucial to comprehending how the plugin achieves high-performance, parallel backup and restore operations.

---

## Session Types: Command vs. Data

### Command Session

**Purpose**: Control plane for managing backup/restore operations

A command session is a lightweight, persistent network connection to the Catalyst server used for:
- Sending control commands (create object, delete object, list objects)
- Querying server properties and capabilities
- Managing metadata operations
- Authenticating with the server
- Coordinating data session creation

**Characteristics**:
- **One per context**: Each `PluginCommandContext` has one command session
- **Long-lived**: Remains open for the duration of the operation
- **Low bandwidth**: Handles small control messages
- **Protocol**: HTTP/HTTPS-based RESTful API
- **Port**: Default 9387 (configurable via `commandPort`)

### Data Session

**Purpose**: Data plane for transferring backup/restore data

A data session is a high-throughput network connection to the Catalyst server used for:
- Streaming backup data from SAP HANA to Catalyst storage
- Streaming restore data from Catalyst storage to SAP HANA
- Performing data deduplication and compression
- Transferring large data blocks efficiently

**Characteristics**:
- **Multiple per operation**: Each parallel stream requires one or more data sessions
- **Short-lived**: Created for data transfer, closed after completion
- **High bandwidth**: Optimized for bulk data transfer
- **Protocol**: Proprietary binary protocol optimized for throughput
- **Port**: Default 9388 (configurable via `dataPort`)
- **Pooled resource**: Server limits total concurrent data sessions

---

## Architecture Overview

```
┌────────────────────────────────────────────────────────────────┐
│                    SAP HANA Plugin                              │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │           PluginCommandContext (Thread 1)                │  │
│  │                                                           │  │
│  │   ┌─────────────────┐         ┌────────────────────┐    │  │
│  │   │ Command Session │◄────────┤ Control Commands   │    │  │
│  │   │  (Port 9387)    │         │ - Create object    │    │  │
│  │   └─────────────────┘         │ - Get properties   │    │  │
│  │           │                   │ - Delete object    │    │  │
│  │           │                   └────────────────────┘    │  │
│  │           │                                              │  │
│  │           ├──── Creates ────►┌────────────────────┐     │  │
│  │           │                   │  Data Session 1    │     │  │
│  │           │                   │  (Port 9388)       │     │  │
│  │           │                   │  Writing backup    │     │  │
│  │           │                   └────────────────────┘     │  │
│  │           │                                              │  │
│  │           └──── Creates ────►┌────────────────────┐     │  │
│  │                               │  Data Session 2    │     │  │
│  │                               │  (Port 9388)       │     │  │
│  │                               │  Writing backup    │     │  │
│  │                               └────────────────────┘     │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │           PluginCommandContext (Thread 2)                │  │
│  │                                                           │  │
│  │   ┌─────────────────┐         ┌────────────────────┐    │  │
│  │   │ Command Session │◄────────┤ Control Commands   │    │  │
│  │   │  (Port 9387)    │         │ - Validate backup  │    │  │
│  │   └─────────────────┘         │ - Get properties   │    │  │
│  │           │                   └────────────────────┘    │  │
│  │           │                                              │  │
│  │           └──── Creates ────►┌────────────────────┐     │  │
│  │                               │  Data Session 3    │     │  │
│  │                               │  (Port 9388)       │     │  │
│  │                               │  Writing backup    │     │  │
│  │                               └────────────────────┘     │  │
│  └──────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
                            │
                            │ Network
                            ↓
┌────────────────────────────────────────────────────────────────┐
│              HPE StoreOnce Catalyst Server                      │
│                                                                 │
│  Command Session Listener (Port 9387)                          │
│  ├── Handles API requests                                      │
│  ├── Manages authentication                                    │
│  └── Coordinates data session creation                         │
│                                                                 │
│  Data Session Pool (Port 9388)                                 │
│  ├── Session 1: Receiving backup stream from Thread 1         │
│  ├── Session 2: Receiving backup stream from Thread 1         │
│  ├── Session 3: Receiving backup stream from Thread 2         │
│  └── ... (up to DataSessionsLimit)                            │
│                                                                 │
│  Server Properties:                                            │
│  ├── DataSessionsLimit: 100 (max concurrent data sessions)    │
│  └── FreeDataSessions: 97 (currently available)               │
└────────────────────────────────────────────────────────────────┘
```

---

## Session Lifecycle

### Command Session Lifecycle

#### 1. Creation and Initialization

```cpp
// In CatalystSessionManager::openBackupSession()

SESSION_INFO(backup, options, sessionInfo);
// Creates SessionInfo object with:
// - Server address
// - Command port (9387)
// - Data port (9388)
// - Credentials
// - Retry counts
// - Logging configuration

oscpp::DataStream *dataSession = new oscpp::DataStream(
    oscpp::OpenObject(
        sessionInfo,
        oscpp::Object(sessionInfo, storeKey, objectKey, isTeamed)
    )
);

// Command session is implicitly created inside OpenObject constructor
// It establishes HTTP connection to server:9387
```

**What Happens**:
1. Plugin creates `SessionInfo` with server address and ports
2. `oscpp::OpenObject` constructor initiates TCP connection to `server:9387`
3. HTTP handshake establishes command session
4. Authentication credentials are exchanged
5. Server responds with session token
6. Command session is now ready for API calls

#### 2. Command Execution

Command sessions handle various control operations:

```cpp
// Example: Validate Configuration
CMD::RequestToValidateConfig request;
request.setStorageLocation(storageLocation);
request.setBackupOptions(backupOptions);

m_commandManager->executeCommand(pCommandContext, VALIDATE_CONFIG);

// Internally sends HTTP request over command session:
// POST /api/v1/stores/{storeKey}/validate
// Authorization: Bearer {token}
// Body: { configuration parameters }

CMD::ResultsOfValidateConfig *results = 
    pCommandContext->getResultsOfValidateConfig();

// Server responds with validation results
// Response parsed and stored in results object
```

**Common Command Session Operations**:

| Command | Purpose | HTTP Method | Endpoint |
|---------|---------|-------------|----------|
| **VALIDATE_CONFIG** | Verify server connectivity and store availability | GET | `/api/v1/stores/{store}/properties` |
| **CREATE_BACKUP** | Create new backup object in catalog | POST | `/api/v1/stores/{store}/objects` |
| **VALIDATE_BACKUP** | Check if object name is unique | HEAD | `/api/v1/stores/{store}/objects/{object}` |
| **FINISH_BACKUP** | Commit backup and update metadata | PUT | `/api/v1/stores/{store}/objects/{object}` |
| **VALIDATE_RESTORE** | Verify backup exists for restore | HEAD | `/api/v1/stores/{store}/objects/{object}` |
| **FINISH_RESTORE** | Mark restore as complete | POST | `/api/v1/stores/{store}/objects/{object}/restore` |
| **DELETE_BACKUP** | Delete backup object | DELETE | `/api/v1/stores/{store}/objects/{object}` |
| **GET_SERVER_PROPERTIES** | Query server capabilities | GET | `/api/v1/server/properties` |

#### 3. Keep-Alive and Timeout

```cpp
// Default keep-alive: 50 minutes
#define CATSESSIONMNGR_PRV_DEFAULT_DATA_SESSION_INACTIVITY_TIMEOUT_SECONDS (50 * 60)

// Session manager maintains keep-alive
void CatalystSessionManager::setSessionKeepAliveTime(time_t keepAliveTime)
{
    m_sessionKeepAliveTime = keepAliveTime;
}
```

**Keep-Alive Mechanism**:
- Command sessions send periodic heartbeat messages
- Prevents server from closing idle connections
- 50-minute default avoids 1-hour server timeout
- Critical for long-running operations with intermittent activity

#### 4. Session Closure

```cpp
void CatalystSessionManager::closeBackupSession(const Location_t& backup)
{
    oscpp::DataStream *dataSession = /* retrieve from session map */;
    
    // Flush any pending data
    OSCMN_sObjectDataJobType dataJob;
    dataSession->Close(dataJob);
    
    // Close command session (implicit in destructor)
    delete dataSession;
    
    // Command session TCP connection is closed
}
```

### Data Session Lifecycle

#### 1. Data Session Creation

Data sessions are created on-demand when data transfer is needed:

```cpp
// In backup operation
void SAPHanaBackup::doStorageOperation(...)
{
    // Command session creates data session
    CMD::RequestToTransferBackup *request = 
        pCommandContext->getRequestToTransferBackup();
    
    request->setStorageLocation(storageLocation);
    request->setBackupOptions(backupOptions);
    request->setInputBufferAddr(pBuffer);      // 4 MB chunk
    request->setInputBufferSize(4194304);
    
    // First TRANSFER_BACKUP command creates data session
    m_commandManager->executeCommand(pCommandContext, TRANSFER_BACKUP);
    
    // Data session now open on port 9388
    // Binary protocol initialized
    // Compression/deduplication engines activated
}
```

**Creation Steps**:
1. Plugin sends API request over command session: "Open data session for object X"
2. Server allocates data session from pool
3. Server responds with data session ID and connection details
4. Plugin opens TCP connection to `server:9388`
5. Binary protocol handshake (version negotiation, capabilities)
6. Data session ready for streaming

#### 2. Data Transfer

Data sessions stream data in chunks:

```cpp
// Backup: Read from SAP HANA → Write to Catalyst
while ((bytesRead = read(*pReader, pBuffer, bufferSize)) > 0)
{
    // Setup transfer request
    pRequest->setInputBufferAddr(pBuffer);
    pRequest->setInputBufferSize(bytesRead);
    
    // Send data chunk over data session
    m_commandManager->executeCommand(pCommandContext, TRANSFER_BACKUP);
    
    // Data flows:
    // 1. Plugin → Network (TCP/IP)
    // 2. Catalyst receives chunk
    // 3. Deduplication engine processes
    // 4. Compressed blocks written to storage
    // 5. Acknowledgment sent back to plugin
    
    totalBackupSize += bytesRead;
}
```

**Data Flow (Backup)**:
```
SAP HANA Pipe → Plugin Buffer → Data Session → Network → 
Catalyst Receiver → Deduplication Engine → Compression → 
Storage Blocks → Disk
```

**Data Flow (Restore)**:
```
Disk → Storage Blocks → Decompression → Rehydration Engine → 
Data Session → Network → Plugin Buffer → SAP HANA Pipe
```

#### 3. Session Configuration

Data sessions can be configured for different bandwidth modes:

```cpp
// Configure data session for low/high bandwidth
dataSession->Configure(
    oscpp::OSCPP_IO_MODE_WRITE_BYTES,
    isLowBw ? oscpp::BANDWIDTH_LOW : oscpp::BANDWIDTH_HIGH,
    highBwBufferSize,  // 4 MB typical for high bandwidth
    lowBwBufferSize    // 64 KB typical for low bandwidth
);
```

**Bandwidth Modes**:

| Mode | Buffer Size | Threads | Use Case |
|------|-------------|---------|----------|
| **High Bandwidth** | 4 MB | 4-8 | LAN, high-speed network |
| **Low Bandwidth** | 64 KB | 1-2 | WAN, limited network |

**Configuration Options** (from `Options_t`):
- `threadCount`: Number of worker threads per data session
- `payLoadCompressionEnabled`: Enable compression in transit
- `payLoadChecksumEnabled`: Enable checksums for data integrity
- `implicitFlushFreq`: Auto-flush frequency for data buffers
- `wBufSizeHi`: Write buffer size for high bandwidth
- `wBufSizeLo`: Write buffer size for low bandwidth
- `rBufSizeHi`: Read buffer size for high bandwidth

#### 4. Session Closure

```cpp
void CatalystSessionManager::closeBackupSession(const Location_t& backup)
{
    oscpp::DataStream *dataSession = /* ... */;
    
    // Flush pending data to storage
    OSCMN_sObjectDataJobType dataJob;
    dataSession->Close(dataJob);
    
    // Data session closure sequence:
    // 1. Flush any buffered data
    // 2. Send "end of stream" marker
    // 3. Wait for final acknowledgment
    // 4. Close TCP connection
    // 5. Server returns session to pool
    
    delete dataSession;
}
```

---

## Server Resource Management

### Data Session Limits

The Catalyst server imposes limits on concurrent data sessions to manage resources:

```cpp
uint64_t ISAPOperation::getNumberOfThreadsToRunInParallel()
{
    // Query server for capabilities
    CMD::RequestToGetServerProperties *pRequest = 
        pCommandContext->getRequestToGetServerProperties();
    
    m_commandManager->executeCommand(pCommandContext, GET_SERVER_PROPERTIES);
    
    CMD::ResultsOfGetServerProperties *pResult = 
        pCommandContext->getResultsOfGetServerProperties();
    
    OSCMN_sServerPropertiesType serverProps = pResult->getServerProperties();
    
    // Server reports:
    uint64_t totalLimit = serverProps.DataSessionsLimit;     // e.g., 100
    uint64_t available = serverProps.FreeDataSessions;       // e.g., 97
    
    // Plugin applies 60% rule
    uint64_t sessionLimit = SAP_PARALLEL_STREAMS_DATA_SESSION_LIMIT(totalLimit);
    // sessionLimit = (100 * 6) / 10 = 60 sessions
    
    // Calculate threads based on sessions per stream
    uint64_t numThreads = sessionLimit / getNumberOfDataSessionsPerStream();
    // For HANA: numThreads = 60 / 1 = 60 threads
    // For Oracle: numThreads = 60 / 2 = 30 threads (demuxing)
    
    return numThreads;
}
```

### The 60% Rule

**Why limit to 60% of available sessions?**

```cpp
#define SAP_PARALLEL_STREAMS_DATA_SESSION_LIMIT(nStreams) ((nStreams * 6) / 10)
```

**Rationale**:
1. **Headroom**: Leaves capacity for other concurrent operations
2. **Stability**: Prevents overwhelming the server
3. **Performance**: Avoids resource contention and thrashing
4. **Fairness**: Allows multiple clients to share the server
5. **Recovery**: Provides buffer for retry operations

**Example Calculation**:
```
Server Limit: 100 data sessions
60% Rule:     100 × 0.6 = 60 sessions
Operations:   15 backup files
Sessions/Stream: 1 (SAP HANA)

Threads Allowed: 60 / 1 = 60 threads
Threads Needed:  15 / 1 = 15 threads

Result: Can run all 15 operations in parallel!
```

### Dynamic Allocation

The plugin dynamically allocates sessions as threads start:

```cpp
// Initial allocation
for (uint64_t i = 0; i < numThreadsToRunInParallel; i++)
{
    startThreads(ppOperationThreads, i, waitForThreadStatus);
    // Each thread:
    // 1. Creates command session
    // 2. Requests data session from server
    // 3. Server allocates from pool (if available)
    // 4. FreeDataSessions decrements
}

// As threads complete
while (numThreadsThatHaveBeenStarted < totalOperations)
{
    if (isThreadFinished(ppOperationThreads))
    {
        // Thread finished:
        // 1. Data session closed
        // 2. Server returns session to pool
        // 3. FreeDataSessions increments
        
        // Start new thread:
        startThreads(ppOperationThreads, numThreadsThatHaveBeenStarted);
        // New thread allocates session from pool
    }
}
```

---

## Session Configuration

### Configuration Parameters

Sessions are configured via plugin configuration file and runtime options:

```cpp
SESSION_INFO(backup, options, sessionInfo);
// Macro expands to:

oscpp::SessionInfo sessionInfo(backup.serverAddr);

if (options.vendorId.given)
    sessionInfo.Vendor = options.vendorId.value;

if (options.clientId.given)
    sessionInfo.Identifier = options.clientId.value;

if (options.password.given)
    sessionInfo.Password = options.password.value;

if (options.commandPort.given)
    sessionInfo.ServerCommandPort = options.commandPort.value;  // Default: 9387

if (options.dataPort.given)
    sessionInfo.ServerDataPort = options.dataPort.value;        // Default: 9388

if (options.maxCmdRetryCount.given)
    sessionInfo.MaximumCommandRetryCount = options.maxCmdRetryCount.value;

if (options.maxDataRetryCount.given)
    sessionInfo.MaximumDataRetryCount = options.maxDataRetryCount.value;

if (options.logLevel.given)
    sessionInfo.LogLevel = (OSCLT_eLogLevelType)options.logLevel.value;

if (options.threadCount.given)
    sessionInfo.SessionOptions.ThreadCount = options.threadCount.value;

if (options.payLoadCompressionEnabled.given)
    sessionInfo.SessionOptions.BodyPayloadCompressionDisabled = 
        !options.payLoadCompressionEnabled.value;

if (options.payLoadChecksumEnabled.given)
    sessionInfo.SessionOptions.PayloadChecksumsDisabled = 
        !options.payLoadChecksumEnabled.value;

if (options.implicitFlushFreq.given)
    sessionInfo.SessionOptions.ImplicitFlushFrequency = 
        options.implicitFlushFreq.value;
```

### Configuration File Example

```ini
[SAPHANA]
# Server connection
SERVER_ADDRESS=catalyst.example.com
COMMAND_PORT=9387
DATA_PORT=9388

# Authentication
CLIENT_ID=SAPHANA_PLUGIN
PASSWORD=encrypted_password

# Session behavior
MAX_CMD_RETRY_COUNT=3
MAX_DATA_RETRY_COUNT=5
THREAD_COUNT=4

# Data transfer
PAYLOAD_COMPRESSION=true
PAYLOAD_CHECKSUM=true
IMPLICIT_FLUSH_FREQ=1000

# Bandwidth mode
BANDWIDTH_MODE=HIGHBANDWIDTH
CAT_BUFFER_SIZE_HI_BW_WRITE=4194304    # 4 MB
CAT_BUFFER_SIZE_LO_BW_WRITE=65536      # 64 KB

# Logging
CAT_LOG_LEVEL=INFO
CAT_LOG_FILE=catalyst.log
CAT_LOG_SIZE=10485760                   # 10 MB
```

---

## Session Management: CatalystSessionManager

### Session Map

Sessions are tracked in a thread-safe map:

```cpp
class CatalystSessionManager
{
private:
    // Map: sessionKey → DataStream object
    std::map<std::string, oscpp::DataStream *const> *const m_sessions;
    
    // Mutex for thread-safe access
    mutable thrLock_t s_mutex;
    
    // Session key format: "server store object [teamed]"
    std::string getSessionKey(const Location_t& backup) const
    {
        std::ostringstream oss;
        oss << backup.serverAddr << " " 
            << backup.storeKey << " " 
            << backup.objectKey << " " 
            << (backup.isTeamed.value ? "teamed" : "");
        return oss.str();
    }
};
```

### Session Operations

#### Check if Session Exists

```cpp
bool CatalystSessionManager::hasBackupSession(const Location_t& backup) const
{
    s_mutex.lock();
    
    if (m_sessions->empty())
    {
        s_mutex.unlock();
        return false;
    }
    
    std::string sessionKey = getSessionKey(backup);
    auto it = m_sessions->find(sessionKey);
    
    bool hasSession = false;
    if (it != m_sessions->end())
    {
        oscpp::DataStream *dataSession = it->second;
        hasSession = dataSession->IsOpen();
    }
    
    s_mutex.unlock();
    return hasSession;
}
```

#### Open Session

```cpp
void CatalystSessionManager::openBackupSession(
    const Location_t& backup, 
    const Options_t& options)
{
    s_mutex.lock();
    
    std::string sessionKey = getSessionKey(backup);
    auto it = m_sessions->find(sessionKey);
    
    oscpp::DataStream* dataSession = nullptr;
    
    if (it != m_sessions->end())
    {
        // Session exists, reuse it
        dataSession = it->second;
    }
    else
    {
        // Create new session
        SESSION_INFO(backup, options, sessionInfo);
        dataSession = new oscpp::DataStream(
            oscpp::OpenObject(
                sessionInfo,
                oscpp::Object(sessionInfo, backup.storeKey, 
                             backup.objectKey, backup.isTeamed.value)
            )
        );
        
        // Add to map
        m_sessions->insert(std::pair(sessionKey, dataSession));
    }
    
    // Open if not already open
    if (!dataSession->IsOpen())
    {
        dataSession->Open();
    }
    
    // Configure bandwidth
    if (options.isLowBw.given)
    {
        dataSession->Configure(
            oscpp::OSCPP_IO_MODE_WRITE_BYTES,
            options.isLowBw.value ? oscpp::BANDWIDTH_LOW : oscpp::BANDWIDTH_HIGH,
            options.wBufSizeHi.value,
            options.wBufSizeLo.value
        );
    }
    
    s_mutex.unlock();
}
```

#### Close Session

```cpp
void CatalystSessionManager::closeBackupSession(const Location_t& backup)
{
    s_mutex.lock();
    
    if (m_sessions->empty())
    {
        s_mutex.unlock();
        return;
    }
    
    std::string sessionKey = getSessionKey(backup);
    auto it = m_sessions->find(sessionKey);
    
    if (it != m_sessions->end())
    {
        oscpp::DataStream *dataSession = it->second;
        
        // Close and flush
        OSCMN_sObjectDataJobType dataJob;
        dataSession->Close(dataJob);
        
        // Cleanup
        delete dataSession;
        m_sessions->erase(it->first);
    }
    
    s_mutex.unlock();
}
```

---

## Multi-threaded Session Management

### Thread Isolation

Each thread operates with independent sessions:

```cpp
void SAPHanaBackup::operationStream(uint32_t index)
{
    // 1. CREATE ISOLATED CONTEXT
    PluginCommandContext* pCommandContext = nullptr;
    HANA_CONTEXT_MGR->createContext(&pCommandContext);
    
    // Context contains:
    // - Unique context ID
    // - Command session (created on first use)
    // - Request/Response objects
    // - Timing instrumentation
    
    // 2. OPEN BACKUP SESSION
    // Creates command session + prepares for data session
    Location_t storageLocation;
    storageLocation.serverAddr = "catalyst.example.com";
    storageLocation.storeKey = "store1";
    storageLocation.objectKey = m_operationEntries[index].objectFileMap.objectName;
    
    // This is thread-safe due to mutex in session manager
    CatalystSessionManager* sessionMgr = CatalystSessionManager::getInstance(m_pLogger);
    sessionMgr->openBackupSession(storageLocation, backupOptions);
    
    // 3. TRANSFER DATA
    // Data session created on first TRANSFER_BACKUP command
    doStorageOperation(pCommandContext, m_operationEntries[index], 
                      storageLocation, backupOptions, backupParams);
    
    // 4. CLOSE SESSION
    sessionMgr->closeBackupSession(storageLocation);
    
    // 5. CLEANUP CONTEXT
    HANA_CONTEXT_MGR->destroyContext(&pCommandContext);
}
```

### No Session Sharing Between Threads

**Key Principle**: Each thread has completely independent sessions

```
Thread 1:
  Context ID: "ctx_001"
  Command Session: conn_1 → server:9387
  Data Session: conn_2 → server:9388
  Object: "backup_file_1"

Thread 2:
  Context ID: "ctx_002"
  Command Session: conn_3 → server:9387
  Data Session: conn_4 → server:9388
  Object: "backup_file_2"

No shared state between Thread 1 and Thread 2
```

**Benefits**:
- No lock contention during data transfer
- Independent error handling
- Parallel execution without interference
- Each thread can proceed at its own pace

---

## Performance Considerations

### Session Creation Overhead

**Command Session**: ~50-100 ms
- TCP connection establishment
- TLS/SSL handshake (if enabled)
- Authentication
- Capability negotiation

**Data Session**: ~20-50 ms
- TCP connection establishment
- Binary protocol handshake
- Buffer allocation

**Optimization**: Reuse sessions when possible
- Command sessions stay open for entire operation
- Data sessions closed after each file transfer
- Session pooling on server side reduces overhead

### Bandwidth Utilization

**High Bandwidth Mode**:
```
Buffer Size: 4 MB
Threads per Session: 4-8
Typical Throughput: 100-500 MB/s per data session
Network Usage: Efficient large-block transfers
```

**Low Bandwidth Mode**:
```
Buffer Size: 64 KB
Threads per Session: 1-2
Typical Throughput: 10-50 MB/s per data session
Network Usage: Optimized for high-latency links
```

### Parallel Session Performance

**Example: 6 Parallel Data Sessions**

```
Sequential (1 session):
  Throughput: 200 MB/s
  15 files × 200 MB = 3000 MB
  Time: 3000 MB / 200 MB/s = 15 seconds

Parallel (6 sessions):
  Throughput: 6 × 200 MB/s = 1200 MB/s
  Time: 3000 MB / 1200 MB/s = 2.5 seconds
  Speedup: 6×
```

### Session Lifecycle Optimization

```cpp
// EFFICIENT: Reuse command session
void efficientBackupOperation()
{
    // Open command session once
    openBackupSession(location, options);
    
    // Create, transfer, finish (data session created/destroyed per file)
    for (int i = 0; i < numFiles; i++)
    {
        validateBackup();      // Uses command session
        transferBackup();      // Creates data session, transfers, closes
        finishBackup();        // Uses command session
    }
    
    // Close command session once
    closeBackupSession(location);
}

// INEFFICIENT: Recreate sessions repeatedly
void inefficientBackupOperation()
{
    for (int i = 0; i < numFiles; i++)
    {
        openBackupSession(location, options);   // Overhead!
        validateBackup();
        transferBackup();
        finishBackup();
        closeBackupSession(location);           // Overhead!
    }
}
```

---

## Error Handling and Retry

### Command Session Errors

```cpp
// Configurable retry count
sessionInfo.MaximumCommandRetryCount = 3;

// Automatic retry on transient failures:
// - Network timeout
// - Connection refused (temporary)
// - HTTP 503 (service unavailable)

// Immediate failure on:
// - Authentication failure (401)
// - Authorization failure (403)
// - Resource not found (404)
```

### Data Session Errors

```cpp
// Configurable retry count
sessionInfo.MaximumDataRetryCount = 5;

// Automatic retry on:
// - Partial data transfer
// - Checksum mismatch
// - Network interruption
// - Server busy

// Recovery mechanisms:
// 1. Retry with exponential backoff
// 2. Request new data session from pool
// 3. Resume from last checkpoint
```

### Session Timeout Handling

```cpp
// Data session inactivity timeout: 50 minutes
#define CATSESSIONMNGR_PRV_DEFAULT_DATA_SESSION_INACTIVITY_TIMEOUT_SECONDS (50 * 60)

// If no activity for 50 minutes:
// 1. Server sends keep-alive query
// 2. Plugin responds to keep session alive
// 3. If no response: server closes session
// 4. Plugin detects closure, creates new session
```

---

## Summary

### Key Concepts

1. **Two Session Types**:
   - **Command Sessions**: Control plane (HTTP/REST, port 9387)
   - **Data Sessions**: Data plane (binary protocol, port 9388)

2. **Session Lifecycle**:
   - Command sessions: Long-lived, one per context
   - Data sessions: Short-lived, created per transfer

3. **Resource Management**:
   - Server enforces `DataSessionsLimit`
   - Plugin applies 60% rule for stability
   - Dynamic allocation as threads start/finish

4. **Thread Isolation**:
   - Each thread has independent sessions
   - No session sharing between threads
   - Thread-safe session manager coordinates access

5. **Performance Optimization**:
   - Reuse command sessions
   - Parallel data sessions for throughput
   - Configurable bandwidth modes
   - Session pooling on server

### Best Practices

✅ **DO**:
- Query server properties before parallel operations
- Respect DataSessionsLimit and apply 60% rule
- Close sessions promptly after completion
- Configure appropriate retry counts
- Use high bandwidth mode for LAN

✅ **DON'T**:
- Share sessions between threads
- Exceed server session limits
- Keep sessions idle unnecessarily
- Ignore session timeout warnings
- Use high bandwidth over WAN

---

## Conclusion

Command and data sessions form the foundation of the SAP HANA plugin's communication with Catalyst storage. Command sessions provide the control plane for managing operations, while data sessions provide the high-throughput data plane for transferring backup and restore data. 

The separation of control and data planes, combined with dynamic session allocation and thread isolation, enables the plugin to achieve high-performance parallel backup operations while maintaining stability and resource efficiency. Understanding these session mechanics is essential for troubleshooting, performance tuning, and extending the plugin's capabilities.
