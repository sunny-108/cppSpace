# SAP HANA Plugin - Multi-threaded Backup Orchestration with Parallel Stream Processing

## Overview

The SAP HANA plugin implements a sophisticated multi-threaded backup orchestration system that enables parallel processing of multiple backup streams. This architecture significantly improves backup performance by processing multiple files concurrently while maintaining thread safety and resource control.

## Architecture Components

### 1. Thread Management Classes

#### SAPOperationThread
- **Base Class**: `PluginThread` (platform-independent thread wrapper)
- **Purpose**: Wrapper class that executes individual backup/restore/delete operations
- **Key Features**:
  - Encapsulates the execution of `operationStream()` for a specific index
  - Manages thread execution status via mutex-protected state transitions
  - Automatically updates thread state to `THREAD_EXECUTION_FINISHED` upon completion

```cpp
class SAPOperationThread : public PluginThread
{
    void run()
    {
        // Run the operation stream
        m_pOperation->operationStream(m_index);

        // Lock mutex and set thread execution status to FINISHED
        if ( m_pOperation->operationEntriesMutex.lock() == false )
        {
            PLUGIN_TRACE_IF_ERROR_LOG( m_pOperation->m_pLogger, "Failed to lock operationEntriesMutex" )
        }

        m_pOperation->m_operationEntries[m_index].ThreadExecutionStatus = THREAD_EXECUTION_FINISHED;

        if ( m_pOperation->operationEntriesMutex.unlock() == false )
        {
            PLUGIN_TRACE_IF_ERROR_LOG( m_pOperation->m_pLogger, "Failed to unlock operationEntriesMutex" )
        }
    }
};
```

### 2. Operation Entry Structure

Each backup/restore/delete operation is represented by a `sOperationEntry` structure that contains:
- File metadata (fileName, objectName, externalBackupId)
- Operation result status
- Thread execution status (NOT_STARTED, RUNNING, FINISHED, COMPLETED)
- File type and size information

### 3. Thread Execution States

The system tracks four distinct thread states:
- **THREAD_EXECUTION_NOT_STARTED**: Initial state before thread creation
- **THREAD_EXECUTION_RUNNING**: Thread is actively processing
- **THREAD_EXECUTION_FINISHED**: Thread has completed execution (before join)
- **THREAD_EXECUTION_COMPLETED**: Thread has been joined and cleaned up

## Parallel Orchestration Flow

### Main Orchestration Method: `runParallelOperation()`

Located in `ISAPOperation.cpp`, this method coordinates all parallel thread execution:

```cpp
void ISAPOperation::runParallelOperation(bool fileOnlineOption, bool& softErrorOccured, bool waitForThreadStatus)
```

#### Phase 1: Initialization

1. **Validate Operation Entries**
   - Ensures there are backup/restore operations to process
   - Verifies `m_operationEntries` vector is not empty

2. **Determine Thread Pool Size**
   ```cpp
   numThreadsToRunInParallel = getNumberOfThreadsToRunInParallel();
   ```
   - Queries the Catalyst server for available data sessions
   - Applies 60% limit: `SAP_PARALLEL_STREAMS_DATA_SESSION_LIMIT(nStreams)`
   - Caps at the number of operations if fewer operations than available threads

3. **Allocate Thread Array**
   ```cpp
   ppOperationThreads = new SAPOperationThread *[m_operationEntries.size()];
   ```

#### Phase 2: Initial Thread Startup

Starts initial batch of threads up to the parallel limit:

```cpp
for (uint64_t i = 0; i < numThreadsToRunInParallel; i++)
{
    if(true == startThreads((void **) ppOperationThreads, i, waitForThreadStatus))
    {
        if(true == waitForThreadStatus)
        {
            // Wait for thread to transition from NOT_STARTED
            int retryCount = THEAD_EXECUTION_RETRY_COUNT; // 3600 retries
            for ( ;  retryCount > 0; retryCount--)
            {
                if(THREAD_EXECUTION_NOT_STARTED == m_operationEntries[i].ThreadExecutionStatus)
                {
                    Platform::sleepMillisec(THREAD_EXCECUTION_SLEEP_TIME_MSEC); // 1 second
                }
                else
                {
                    break;
                }
            }
        }
        numThreadsThatHaveBeenStarted++;
    }
}
```

**Key Features**:
- Creates and starts threads in a controlled manner
- Optional wait mechanism ensures thread actually starts before proceeding
- Implements 1-hour timeout for thread startup (3600 * 1 second)

#### Phase 3: Dynamic Thread Pool Management

Manages thread lifecycle as operations complete:

```cpp
while(numThreadsThatHaveBeenStarted < m_operationEntries.size())
{
    // Check if any thread has finished
    if( true == isThreadFinished((void **)ppOperationThreads, fileOnlineOption, softErrorOccured))
    {
        // Start a new thread to replace the finished one
        if( true == startThreads((void**)ppOperationThreads, numThreadsThatHaveBeenStarted, waitForThreadStatus))
        {   
            numThreadsThatHaveBeenStarted++;
        }
    }
    else
    {
        // No threads finished yet, sleep and retry
        Platform::sleepMillisec(THREAD_EXCECUTION_SLEEP_TIME_MSEC);
    }
}
```

**Dynamic Scheduling Logic**:
1. Monitor for finished threads using `isThreadFinished()`
2. When a thread finishes:
   - Join the thread (cleanup resources)
   - Start a new thread for the next operation
3. If no threads finished, sleep 1 second and retry
4. Continues until all operations have been started

#### Phase 4: Final Cleanup

Join any remaining running threads:

```cpp
for (uint64_t i=0; i < m_operationEntries.size(); i++)
{
    bool completionStatus = getCompletionStatus(i);
    if( completionStatus == false )
    {
        // Thread still running, join it
        if( false == joinThreads((void **)ppOperationThreads, i, fileOnlineOption))
        {
            softErrorOccured = true;
        }
    }
}
```

### Thread State Management Functions

#### 1. `startThreads()`
- Allocates `SAPOperationThread` instance
- Sets operation entry result to ERROR by default
- Updates thread state to RUNNING (with mutex protection)
- Calls `thread->start()` to begin execution

#### 2. `isThreadFinished()`
- Scans all operation entries (with mutex lock)
- Identifies threads in FINISHED state
- Calls `joinThreads()` for finished threads
- Returns true if any thread was found and joined

#### 3. `joinThreads()`
- Calls `thread->join()` to wait for thread completion
- Performs cleanup (deletes thread object)
- Updates state to COMPLETED
- Handles Oracle file_online mode special logic

#### 4. `getCompletionStatus()`
- Checks if thread is in COMPLETED or NOT_STARTED state
- Used to identify threads that still need joining

## Thread Safety Mechanisms

### 1. Mutex Protection: `operationEntriesMutex`

A thread-safe lock (`thrLock_t`) protects the `m_operationEntries` vector:

```cpp
thrLock_t operationEntriesMutex;
```

**Protected Operations**:
- Reading/writing thread execution status
- Updating operation results
- Scanning for finished threads

**Usage Pattern**:
```cpp
if ( operationEntriesMutex.lock() == false )
{
    PLUGIN_TRACE_IF_ERROR_LOG( m_pLogger, "Failed to lock mutex" )
}

// Critical section - access m_operationEntries

if( operationEntriesMutex.unlock() == false )
{
    PLUGIN_TRACE_IF_ERROR_LOG( m_pLogger, "Failed to unlock mutex" )
}
```

### 2. Object Name Generation Mutex: `generatedObjectNamesMutex`

Protects unique object name generation across parallel threads:

```cpp
static thrLock_t generatedObjectNamesMutex;
std::vector<std::string> generatedObjectNames;
```

**Purpose**: Ensures no two threads generate the same object name
**Algorithm**:
1. Generate timestamp-based object name
2. Lock mutex and check if name already exists
3. If exists, sleep 1ms and regenerate
4. If unique, add to vector and release lock

### 3. Context Isolation per Thread

Each thread operates with its own `PluginCommandContext`:

```cpp
PluginCommandContext* pCommandContext = NULL;
HANA_CONTEXT_MGR->createContext( &pCommandContext );

// ... perform operations ...

HANA_CONTEXT_MGR->destroyContext( &pCommandContext );
```

**Isolated Resources per Context**:
- Command/Data session connections to Catalyst server
- Request/Response objects
- Timing instrumentation (ISV, Catalyst, Plugin timers)
- Error state tracking

## Stream Processing Workflow

Each thread executes `operationStream(index)` which follows this pattern:

### Backup Stream Processing

```cpp
void SAPHanaBackup::operationStream(uint32_t index)
{
    // 1. Create isolated context
    HANA_CONTEXT_MGR->createContext( &pCommandContext );
    
    // 2. Initialize instrumentation timers
    initStreamInstrumentation( pCommandContext );
    
    // 3. Generate unique object name
    generateObjectName(m_operationEntries[index].sequencerString, 
                      m_operationEntries[index].objectFileMap.objectName);
    
    // 4. Setup storage parameters
    setStorageParameters(m_operationEntries[index], storageLocation, 
                        backupOptions, backupParams);
    
    // 5. Validate configuration
    validateConfig(pCommandContext, storageLocation, backupOptions, 
                  m_operationEntries[index]);
    
    // 6. Validate backup operation
    validateStorageOperation(pCommandContext, m_operationEntries[index], 
                            storageLocation, backupOptions, backupParams);
    
    // 7. Transfer data (read from pipe/file, write to Catalyst)
    doStorageOperation(pCommandContext, m_operationEntries[index], 
                      storageLocation, backupOptions, backupParams);
    
    // 8. Finalize backup (update metadata)
    finishStorageOperation(pCommandContext, m_operationEntries[index], 
                          storageLocation, backupOptions, backupParams);
    
    // 9. Rollup timing data
    rollupInstrumentation(pCommandContext, totalCatalystTimer, 
                         totalISVTimer, true /*parallel*/);
    
    // 10. Cleanup context
    HANA_CONTEXT_MGR->destroyContext( &pCommandContext );
}
```

### Data Transfer in Backup

```cpp
// Open IPC pipe or file for reading
SAPIPCData *pReader = new SAPIPCData(pLogger, fileName, timeout);
pReader->open(SAPIPCData::READ_PIPE);
readerBufferSize = pReader->getDataExchangerMaxSize();
pBuffer = new char[readerBufferSize];

// Transfer loop
pCommandContext->getIsvTimer()->start();
while((partialReadSize = read(*pReader, pBuffer, readerBufferSize)) > 0)
{
    pCommandContext->getIsvTimer()->stop();
    
    // Setup transfer request
    pRequest->setInputBufferAddr( pBuffer );
    pRequest->setInputBufferSize( partialReadSize );
    
    // Execute transfer command
    m_commandManager->executeCommand( pCommandContext, TRANSFER_BACKUP );
    
    totalBackupSize += partialReadSize;
    pCommandContext->getIsvTimer()->start();
}
pCommandContext->getIsvTimer()->stop();
```

## Performance Characteristics

### Resource Management

1. **Thread Pool Sizing**
   - Limited to 60% of available Catalyst server data sessions
   - Formula: `(nStreams * 6) / 10`
   - Prevents overwhelming the Catalyst server

2. **Data Session Management**
   - HANA: 1 data session per stream
   - Oracle: 2 data sessions per stream (for demuxing)
   - Configurable via `getNumberOfDataSessionsPerStream()`

3. **Memory Efficiency**
   - Buffer allocated per thread: `dataExchangerMaxSize` (typically 1-4 MB)
   - Buffers deallocated immediately after each stream completes
   - Thread objects deleted after join

### Timing Instrumentation

Each operation tracks three timing categories:

1. **Plugin Time**: Overhead of plugin logic
2. **ISV Time**: SAP HANA read/write operations
3. **Catalyst Time**: Network transfer and storage operations

**Per-stream Timing**:
```cpp
initStreamInstrumentation(pCommandContext);   // Start timers
// ... perform operations, toggling ISV timer ...
finishStreamInstrumentation(pCommandContext); // Stop timers
rollupInstrumentation(...);                   // Aggregate to global
```

**Global Aggregation**:
- Parallel streams: Sum all timers
- Sequential operations: Add sequentially

## Error Handling

### Soft vs Hard Errors

1. **Hard Errors**: Exceptions in operations
   - Caught in `operationStream()`
   - Sets result to `SAP_OPERATION_ERROR`
   - Thread completes normally
   - Other threads continue execution

2. **Soft Errors**: Thread management failures
   - Thread allocation failures
   - Thread join failures
   - Tracked via `softErrorOccured` flag
   - Logged but operation continues

### Exception Safety

```cpp
try
{
    // ... operation logic ...
}
catch(PluginException &ex)
{
    PLUGIN_TRACE_IF_ERROR_LOG( pLogger, ex.what())
    m_operationResult = false;
    m_operationEntries[index].objectFileMap.result = SAP_OPERATION_ERROR;
}
catch(...)
{
    PLUGIN_TRACE_IF_ERROR_LOG( pLogger, "Generic exception catch")
    m_operationResult = false;
    m_operationEntries[index].objectFileMap.result = SAP_OPERATION_ERROR;
}
```

## Key Design Decisions

### 1. Dynamic Thread Pool

**Rationale**: Maintains constant parallelism without over-allocating resources
- Starts only as many threads as server can handle
- Replaces finished threads with new work
- Adapts to varying operation counts

### 2. Mutex-Protected State

**Rationale**: Ensures thread-safe access to shared data structures
- Minimal locking (only state transitions and scanning)
- Lock/unlock always paired
- Soft error on lock failure (logged but continues)

### 3. Isolated Contexts

**Rationale**: Eliminates contention between threads
- Each thread has dedicated server connections
- No sharing of network resources
- Independent error tracking

### 4. Unique Object Name Generation

**Rationale**: Prevents naming collisions in parallel execution
- Timestamp-based with microsecond precision
- Collision detection and retry mechanism
- Thread-safe vector of generated names

## Example Execution Flow

### Scenario: 15 Backup Operations, 5 Parallel Threads

```
Time  | Action
------|--------------------------------------------------------
T0    | Parse input file → 15 operation entries
T1    | Query server → 10 available sessions → limit to 5 threads
T2    | Start threads 0-4 (operations 0-4)
T3    | Operations 0-4 running in parallel
T4    | Operation 2 finishes → join thread 2 → start thread 5 (operation 5)
T5    | Operations 0,1,3,4,5 running
T6    | Operation 0 finishes → join thread 0 → start thread 6 (operation 6)
T7    | Operations 1,3,4,5,6 running
...   | ... continue pattern ...
T20   | Operations 10-14 running
T21   | Operations 10-14 finish
T22   | Join all remaining threads
T23   | Write output file with results
```

## Conclusion

The SAP HANA plugin's multi-threaded orchestration provides:

1. **Scalability**: Efficiently processes dozens of concurrent backup streams
2. **Resource Control**: Respects server capacity limits
3. **Reliability**: Robust error handling with isolated thread contexts
4. **Performance**: Maximizes throughput through dynamic thread management
5. **Thread Safety**: Comprehensive mutex protection for shared state

This architecture enables high-performance backup operations while maintaining data integrity and system stability.
