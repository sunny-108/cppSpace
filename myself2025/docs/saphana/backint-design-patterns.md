# SAP HANA Plugin - Design Patterns for Backint Interface

## Overview

The SAP HANA plugin implements a sophisticated architecture using multiple design patterns to handle backup, restore, and delete operations through the SAP Backint interface. This document analyzes the design patterns employed and their implementation details.

## Architecture Overview

The plugin follows a layered architecture:

```
┌─────────────────────────────────────────────────────────────┐
│                    Main Entry Point                          │
│                      (main.cpp)                              │
└─────────────────────────────────────────────────────────────┘
                            │
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              Factory + Strategy Selection                    │
│         (Creates operation based on function type)           │
└─────────────────────────────────────────────────────────────┘
                            │
                ┌───────────┴───────────┬─────────────┐
                ↓                       ↓             ↓
    ┌──────────────────┐  ┌──────────────────┐  ┌─────────────┐
    │  SAPHanaBackup   │  │ SAPHanaRestore   │  │SAPHanaDelete│
    └──────────────────┘  └──────────────────┘  └─────────────┘
                │                   │                   │
                └───────────────────┴───────────────────┘
                            │
                            ↓
            ┌───────────────────────────────┐
            │     Template Method           │
            │   (ISAPOperation base class)  │
            └───────────────────────────────┘
                            │
                            ↓
            ┌───────────────────────────────┐
            │   Command Pattern             │
            │ (PluginCommandManager)        │
            └───────────────────────────────┘
                            │
                            ↓
            ┌───────────────────────────────┐
            │   Observer Pattern            │
            │   (PluginController)          │
            └───────────────────────────────┘
                            │
                            ↓
            ┌───────────────────────────────┐
            │   Adapter/Facade Pattern      │
            │  (CatalystBackupStorage)      │
            └───────────────────────────────┘
```

## Design Patterns Implementation

### 1. Strategy Pattern

**Purpose**: Encapsulates different backup operation algorithms (backup, restore, delete, inquire)

**Implementation**:

```cpp
// Base Strategy Interface
class ISAPHanaOperation : public ISAPOperation
{
public:
    virtual void runOperation(void) = 0;
    virtual void operationStream(uint32_t index) = 0;
    virtual bool validateStorageOperation(...) = 0;
    virtual bool doStorageOperation(...) = 0;
    virtual bool finishStorageOperation(...) = 0;
};
```

**Concrete Strategies**:

1. **Backup Strategy** (`SAPHanaBackup`)
   - Reads data from pipes/files
   - Transfers to Catalyst storage
   - Generates unique object names

2. **Restore Strategy** (`SAPHanaRestore`)
   - Queries for backup objects
   - Transfers data from Catalyst storage
   - Writes to pipes/files

3. **Delete Strategy** (`SAPHanaDelete`)
   - Validates object existence
   - Deletes from Catalyst storage
   - Updates metadata

4. **Inquire Strategy** (`SAPHanaInquire`)
   - Lists available backups
   - Filters by criteria
   - Returns metadata

**Context Selection**:

```cpp
// In main.cpp - Strategy selection based on user input
switch(userRequest.getOperation())
{
    case SAP_FUNCTION_BACKUP:
        pOperation = new SAPHanaBackup(userRequest);
        break;
    
    case SAP_FUNCTION_RESTORE:
        pOperation = new SAPHanaRestore(userRequest);
        break;
    
    case SAP_FUNCTION_DELETE:
        pOperation = new SAPHanaDelete(userRequest);
        break;
    
    case SAP_FUNCTION_INQUIRE:
        pOperation = new SAPHanaInquire(userRequest);
        break;
}

// Execute selected strategy
if(pOperation)
{
    pOperation->init();
    pOperation->runOperation();
    pOperation->finish();
}
```

### 2. Template Method Pattern

**Purpose**: Defines the skeleton of an operation algorithm in the base class, with specific steps implemented by subclasses

**Implementation in ISAPOperation**:

```cpp
class ISAPOperation
{
public:
    // Template method - defines the overall algorithm
    void runParallelOperation(bool fileOnline, bool& softErrorOccured, bool waitForThreadStatus = false)
    {
        // 1. Initialize parallel operation
        initRunParallelOperation(numThreadsToRunInParallel);
        
        // 2. Start initial threads
        for (uint64_t i = 0; i < numThreadsToRunInParallel; i++)
        {
            startThreads(...);
        }
        
        // 3. Dynamic thread management
        while(numThreadsThatHaveBeenStarted < m_operationEntries.size())
        {
            if(isThreadFinished(...))
            {
                startThreads(...);
            }
        }
        
        // 4. Join remaining threads
        for (uint64_t i=0; i < m_operationEntries.size(); i++)
        {
            joinThreads(...);
        }
    }
    
    // Primitive operations - implemented by subclasses
    virtual void operationStream(uint32_t index) = 0;
    virtual bool validateStorageOperation(...) = 0;
    virtual bool doStorageOperation(...) = 0;
    virtual bool finishStorageOperation(...) = 0;
};
```

**Concrete Implementation in SAPHanaBackup**:

```cpp
void SAPHanaBackup::operationStream(uint32_t index)
{
    // Step 1: Setup (common pattern)
    HANA_CONTEXT_MGR->createContext(&pCommandContext);
    initStreamInstrumentation(pCommandContext);
    
    // Step 2: Generate object name (backup-specific)
    generateObjectName(m_operationEntries[index].sequencerString, 
                      m_operationEntries[index].objectFileMap.objectName);
    
    // Step 3: Setup parameters (operation-specific)
    setStorageParameters(m_operationEntries[index], storageLocation, 
                        backupOptions, backupParams);
    
    // Step 4: Validate (hook method)
    if(!validateConfig(pCommandContext, storageLocation, backupOptions, 
                      m_operationEntries[index]))
        goto out;
    
    // Step 5: Validate operation (hook method)
    if(!validateStorageOperation(pCommandContext, m_operationEntries[index], 
                                 storageLocation, backupOptions, backupParams))
        goto out;
    
    // Step 6: Execute operation (hook method)
    if(!doStorageOperation(pCommandContext, m_operationEntries[index], 
                          storageLocation, backupOptions, backupParams))
        goto out;
    
    // Step 7: Finalize (hook method)
    if(!finishStorageOperation(pCommandContext, m_operationEntries[index], 
                               storageLocation, backupOptions, backupParams))
        goto out;
    
out:
    // Step 8: Cleanup (common pattern)
    rollupInstrumentation(pCommandContext, totalCatalystTimer, totalISVTimer);
    HANA_CONTEXT_MGR->destroyContext(&pCommandContext);
}
```

**Template Method Benefits**:
- Enforces consistent operation flow across all operations
- Allows customization of specific steps
- Reduces code duplication
- Ensures proper resource management (init/cleanup)

### 3. Command Pattern

**Purpose**: Encapsulates backup operations as command objects that can be executed, queued, and tracked

**Implementation**:

```cpp
// Command Enumeration
enum PluginCommand_t
{
    VALIDATE_CONFIG,
    CREATE_BACKUP,
    VALIDATE_BACKUP,
    TRANSFER_BACKUP,
    CANCEL_BACKUP,
    FINISH_BACKUP,
    VALIDATE_RESTORE,
    TRANSFER_RESTORE,
    FINISH_RESTORE,
    GET_BACKUP_INFO,
    DELETE_BACKUP,
    GET_SERVER_PROPERTIES
};
```

**Command Manager (Invoker)**:

```cpp
class PluginCommandManager : public ICommandManager
{
public:
    // Execute a command
    void executeCommand(PluginCommandContext*& context, PluginCommand_t commandType)
    {
        // Set command type on context
        context->setCommandType(commandType);
        context->clearError();
        
        // Notify observer to execute
        notifyObservers(context->getContextId());
    }
    
    // Request objects (Command parameters)
    RequestToValidateBackup* getRequestToValidateBackup(const std::string& contextId);
    RequestToTransferBackup* getRequestToTransferBackup(const std::string& contextId);
    RequestToFinishBackup* getRequestToFinishBackup(const std::string& contextId);
    RequestToValidateRestore* getRequestToValidateRestore(const std::string& contextId);
    RequestToTransferRestore* getRequestToTransferRestore(const std::string& contextId);
    RequestToDeleteBackup* getRequestToDeleteBackup(const std::string& contextId);
    
    // Result objects (Command results)
    ResultsOfValidateBackup* getResultsOfValidateBackup(const std::string& contextId);
    ResultsOfTransferBackup* getResultsOfTransferBackup(const std::string& contextId);
    ResultsOfFinishBackup* getResultsOfFinishBackup(const std::string& contextId);
    ResultsOfValidateRestore* getResultsOfValidateRestore(const std::string& contextId);
    ResultsOfTransferRestore* getResultsOfTransferRestore(const std::string& contextId);
    ResultsOfDeleteBackup* getResultsOfDeleteBackup(const std::string& contextId);
};
```

**Command Execution Example**:

```cpp
// Backup operation - Transfer command
bool SAPHanaBackup::doStorageOperation(...)
{
    // Create command parameters
    CMD::RequestToTransferBackup *const pRequest = 
        pCommandContext->getRequestToTransferBackup();
    pRequest->setBackupOptions(backupOptions);
    pRequest->setStorageLocation(storageLocation);
    pRequest->setInputBufferAddr(pBuffer);
    pRequest->setInputBufferSize(partialReadSize);
    
    // Execute command
    m_commandManager->executeCommand(pCommandContext, TRANSFER_BACKUP);
    
    // Get command results
    CMD::ResultsOfTransferBackup *const pResult = 
        pCommandContext->getResultsOfTransferBackup();
    
    if(pResult->isError())
    {
        // Handle error
    }
}
```

**Command Pattern Benefits**:
- Decouples request from execution
- Supports queuing and logging of operations
- Enables timing and instrumentation per command
- Facilitates retry and error handling

### 4. Observer Pattern

**Purpose**: Allows the Controller to be notified when commands are ready for execution

**Implementation**:

```cpp
// Observer Interface
class IObserver
{
public:
    virtual void handleEvent(const std::string& contextId) const = 0;
};

// Observable (Subject)
class PluginCommandManager : public ICommandManager
{
protected:
    IObserver* m_observer;
    
    void attachObserver(IObserver* observer)
    {
        if(m_observer != NULL)
            throw PluginException("Observer already attached");
        m_observer = observer;
    }
    
    void notifyObservers(const std::string& contextId) const
    {
        if(m_observer != NULL)
        {
            m_observer->handleEvent(contextId);
        }
    }
};

// Concrete Observer
class PluginController : public IObserver
{
public:
    void handleEvent(const std::string& contextId) const
    {
        // Get command type from context
        PluginCommand_t commandType = m_commandManager->getCommandType(contextId);
        
        // Execute the command
        executeCommand(contextId, commandType);
    }
    
    void executeCommand(const std::string& contextId, 
                       const PluginCommand_t commandType) const
    {
        switch(commandType)
        {
            case VALIDATE_CONFIG:
                doValidateConfig(contextId);
                break;
            case TRANSFER_BACKUP:
                doTransferBackup(contextId);
                break;
            case TRANSFER_RESTORE:
                doTransferRestore(contextId);
                break;
            case DELETE_BACKUP:
                doDeleteBackup(contextId);
                break;
            // ... other commands
        }
    }
};
```

**Setup in Main**:

```cpp
// Create singletons
PluginCommandManager* const commandManager = 
    PluginCommandManager::getInstance(pLogger, configFile);

CatalystBackupStorage* const backupStorage = 
    CatalystBackupStorage::getInstance(pLogger, pMetadataManagerFactory);

PluginController* const pluginController = 
    PluginController::getInstance(pLogger, commandManager, backupStorage);

// Controller automatically attaches as observer
pluginController->init();
```

**Observer Pattern Benefits**:
- Loose coupling between command manager and controller
- Supports multiple observers (extensible)
- Event-driven architecture
- Enables asynchronous execution

### 5. Singleton Pattern

**Purpose**: Ensures only one instance of key managers exists throughout the application lifecycle

**Implementations**:

```cpp
// PluginCommandManager Singleton
class PluginCommandManager
{
    static PluginCommandManager* s_instance;
    
    PluginCommandManager(const std::string& configFile); // Private constructor
    
public:
    static PluginCommandManager* getInstance(LoggerBase* pLogger, 
                                            const std::string& configFile)
    {
        if(s_instance == NULL)
        {
            s_logger = pLogger;
            s_instance = new PluginCommandManager(configFile);
        }
        return s_instance;
    }
};

// PluginController Singleton
class PluginController
{
    static PluginController* s_instance;
    
    PluginController(ICommandManager* commandManager, 
                    IBackupStorage* backupStorage); // Private constructor
    
public:
    static PluginController* getInstance(LoggerBase* pLogger,
                                        ICommandManager* commandManager,
                                        IBackupStorage* backupStorage)
    {
        if(s_instance == NULL)
        {
            s_logger = pLogger;
            s_instance = new PluginController(commandManager, backupStorage);
        }
        return s_instance;
    }
};

// PluginCommandContextManager Singleton
class PluginCommandContextManager
{
    static PluginCommandContextManager* s_instance;
    
public:
    static PluginCommandContextManager* getInstance(LoggerBase* pLogger)
    {
        if(s_instance == NULL)
        {
            s_instance = new PluginCommandContextManager(pLogger);
        }
        return s_instance;
    }
    
    void createContext(PluginCommandContext** ppContext);
    void destroyContext(PluginCommandContext** ppContext);
    PluginCommandContext* getContextFromMap(const std::string& contextId);
};
```

**Singleton Pattern Benefits**:
- Global access point to managers
- Prevents multiple configurations
- Resource management (single connection pool)
- Thread-safe initialization

### 6. Factory Pattern (Implicit)

**Purpose**: Creates appropriate IPC command handlers based on operation type

**Implementation**:

```cpp
// Base IPC Command Interface
class ISAPHanaIPCCommand : public ISAPIPCCommand
{
public:
    virtual void processInputFile(void);
    virtual void generateResultFile(std::vector<sOperationEntry> operationEntries);
    
protected:
    virtual void parseCommandInput(std::vector<std::string> tokens) = 0;
    virtual void writeCommandResults(std::ostream& outFileStream, 
                                     sOperationEntry operationEntry) = 0;
};

// Concrete Factories in Operation Classes
SAPHanaBackup::SAPHanaBackup(SAPHanaUserRequest userRequest) 
    : ISAPHanaOperation(userRequest)
{
    // Factory: Create backup-specific IPC command
    m_pIPCCommand = (ISAPHanaIPCCommand *)new SAPHanaBackupIPCCommand(
        userRequest.getInputFile(), 
        userRequest.getOutputFile()
    );
}

SAPHanaRestore::SAPHanaRestore(SAPHanaUserRequest userRequest) 
    : ISAPHanaOperation(userRequest)
{
    // Factory: Create restore-specific IPC command
    m_pIPCCommand = (ISAPHanaIPCCommand *)new SAPHanaRestoreIPCCommand(
        userRequest.getInputFile(), 
        userRequest.getOutputFile()
    );
}

SAPHanaDelete::SAPHanaDelete(SAPHanaUserRequest userRequest) 
    : ISAPHanaOperation(userRequest)
{
    // Factory: Create delete-specific IPC command
    m_pIPCCommand = (ISAPHanaIPCCommand *)new SAPHanaDeleteIPCCommand(
        userRequest.getInputFile(), 
        userRequest.getOutputFile()
    );
}
```

### 7. Adapter Pattern

**Purpose**: Adapts the SAP Backint interface to the Catalyst storage API

**Implementation**:

```cpp
// Target Interface (Catalyst API)
class IBackupStorage
{
public:
    virtual void validateConfig(const RequestToValidateConfig& request, 
                               ResultsOfValidateConfig& results) = 0;
    virtual void validateBackup(const RequestToValidateBackup& request, 
                               ResultsOfValidateBackup& results) = 0;
    virtual void transferBackup(const RequestToTransferBackup& request, 
                               ResultsOfTransferBackup& results) = 0;
    // ... more methods
};

// Adapter
class CatalystBackupStorage : public IBackupStorage
{
private:
    // Adaptee (Catalyst C++ wrapper)
    MetadataManager* m_metadataManager;
    
public:
    void transferBackup(const RequestToTransferBackup& request, 
                       ResultsOfTransferBackup& results)
    {
        // Adapt SAP parameters to Catalyst API
        Location_t location = request.getStorageLocation();
        Options_t options = request.getBackupOptions();
        
        // Call Catalyst API
        m_metadataManager->writeData(
            location.storeKey,
            location.objectKey,
            request.getInputBufferAddr(),
            request.getInputBufferSize()
        );
        
        // Convert results back to SAP format
        results.setIsError(false);
        results.setBytesTransferred(bytesWritten);
    }
};
```

**Adapter Pattern Benefits**:
- Isolates SAP-specific code from Catalyst API
- Enables replacement of storage backend
- Simplifies testing (mock adapters)

## Operation-Specific Implementations

### Backup Operation Design

```cpp
class SAPHanaBackup : public ISAPHanaOperation
{
public:
    void runOperation(void)
    {
        bool softErrorStatus = false;
        this->runParallelOperation(false, softErrorStatus);
    }
    
    void operationStream(uint32_t index)
    {
        // 1. Generate unique object name
        generateObjectName(m_operationEntries[index].sequencerString, 
                          m_operationEntries[index].objectFileMap.objectName);
        
        // 2. Validate backup can be created
        validateStorageOperation(...);
        
        // 3. Transfer data from HANA to Catalyst
        doStorageOperation(...);
        
        // 4. Finalize and update metadata
        finishStorageOperation(...);
    }
    
    bool doStorageOperation(...)
    {
        // Open IPC pipe/file for reading
        SAPIPCData *pReader = new SAPIPCData(pLogger, fileName, timeout);
        pReader->open(SAPIPCData::READ_PIPE);
        
        // Transfer loop
        while((partialReadSize = read(*pReader, pBuffer, bufferSize)) > 0)
        {
            // Setup transfer request
            pRequest->setInputBufferAddr(pBuffer);
            pRequest->setInputBufferSize(partialReadSize);
            
            // Execute TRANSFER_BACKUP command
            m_commandManager->executeCommand(pCommandContext, TRANSFER_BACKUP);
            
            totalBackupSize += partialReadSize;
        }
        
        return true;
    }
};
```

**Backup Design Highlights**:
- **Unique Name Generation**: Timestamp-based with collision detection
- **Incremental Transfer**: Reads and writes in chunks
- **ISV Timing**: Tracks time spent reading from HANA
- **Result Tracking**: Updates `SAP_OPERATION_SAVED` on success

### Restore Operation Design

```cpp
class SAPHanaRestore : public ISAPHanaOperation
{
public:
    void runOperation(void)
    {
        bool softErrorStatus = false;
        this->runParallelOperation(false, softErrorStatus, true /*waitForThreadStatus*/);
    }
    
    void operationStream(uint32_t index)
    {
        // 1. Find object name from EBID or filename
        findObjectName(m_operationEntries[index]);
        
        // 2. Validate object exists
        validateStorageOperation(...);
        
        // 3. Transfer data from Catalyst to HANA
        doStorageOperation(...);
        
        // 4. Finalize
        finishStorageOperation(...);
    }
    
    bool findObjectName(sOperationEntry& operationEntry)
    {
        // Use inquire operation to search for backup
        SAPHanaInquire inquire(m_userRequest);
        inquire.operationStream(0);
        inquire.sortListedBackup();
        
        std::vector<sOperationEntry> listedEntries = inquire.getResultEntries();
        
        if(listedEntries.size() == 0)
        {
            operationEntry.objectFileMap.result = SAP_OPERATION_NOTFOUND;
            return false;
        }
        
        // Use first (most recent) match
        operationEntry.objectFileMap.objectName = listedEntries[0].objectFileMap.objectName;
        return true;
    }
    
    bool doStorageOperation(...)
    {
        // Open IPC pipe/file for writing
        SAPIPCData dataWriter(pLogger, restoreFileName, timeout);
        dataWriter.open(getRestoreOpenMode(operationEntry.objectFileMap));
        
        // Transfer loop
        do
        {
            // Setup transfer request
            pRequest->setOutputBufferAddr(pBuffer);
            pRequest->setOutputBufferSize(bufferSize);
            
            // Execute TRANSFER_RESTORE command
            m_commandManager->executeCommand(pCommandContext, TRANSFER_RESTORE);
            
            // Get results
            partialReadSize = pResult->getBytesRestored();
            
            // Write to HANA
            partialWriteSize = write(dataWriter, pBuffer, partialReadSize);
            totalRestoreSize += partialWriteSize;
            
        } while(totalRestoreSize < operationEntry.size);
        
        return true;
    }
};
```

**Restore Design Highlights**:
- **Object Discovery**: Uses inquire to find matching backups
- **Latest Backup Selection**: Sorts and selects most recent
- **Reverse Transfer**: Catalyst → HANA
- **Thread Synchronization**: Waits for thread startup before proceeding

### Delete Operation Design

```cpp
class SAPHanaDelete : public ISAPHanaOperation
{
public:
    void runOperation(void)
    {
        bool softErrorStatus = false;
        this->runParallelOperation(false, softErrorStatus);
    }
    
    void operationStream(uint32_t index)
    {
        // 1. Convert EBID to object name
        getObjectNameFromExternalBackupId(
            m_operationEntries[index].objectFileMap.externalBackupId,
            m_operationEntries[index].objectFileMap.objectName
        );
        
        // 2. Validate object exists (using inquire)
        validateStorageOperation(...);
        
        // 3. Delete object
        doStorageOperation(...);
        
        // 4. Finalize
        finishStorageOperation(...);
    }
    
    bool validateStorageOperation(...)
    {
        // Use inquire to verify object exists
        SAPHanaInquire inquire(m_userRequest);
        inquire.operationStream(0);
        
        std::vector<sOperationEntry> listedEntries = inquire.getResultEntries();
        
        if(listedEntries.size() != 1 || 
           listedEntries[0].objectFileMap.result != SAP_OPERATION_BACKUP)
        {
            operationEntry.objectFileMap.result = SAP_OPERATION_NOTFOUND;
            return false;
        }
        
        // Copy exact object name from list result
        operationEntry.objectFileMap.objectName = listedEntries[0].objectFileMap.objectName;
        return true;
    }
    
    bool doStorageOperation(...)
    {
        // Setup delete request
        CMD::RequestToDeleteBackup *const pRequest = 
            pCommandContext->getRequestToDeleteBackup();
        pRequest->setStorageLocation(storageLocation);
        
        // Execute DELETE_BACKUP command
        m_commandManager->executeCommand(pCommandContext, DELETE_BACKUP);
        
        // Check results
        CMD::ResultsOfDeleteBackup *const pResult = 
            pCommandContext->getResultsOfDeleteBackup();
        
        if(pResult->isPermissionError())
        {
            operationEntry.objectFileMap.result = SAP_OPERATION_NOTDELETED;
            return false;
        }
        
        operationEntry.objectFileMap.result = SAP_OPERATION_DELETED;
        return true;
    }
};
```

**Delete Design Highlights**:
- **Pre-validation**: Ensures object exists before deletion
- **Permission Handling**: Distinguishes permission errors from other failures
- **Composition**: Reuses inquire operation for validation
- **Result Granularity**: Three states (DELETED, NOTFOUND, NOTDELETED)

## IPC Communication Pattern

The plugin uses a specialized IPC pattern for SAP HANA communication:

```cpp
class ISAPHanaIPCCommand : public ISAPIPCCommand
{
public:
    // Read input file generated by SAP HANA
    virtual void processInputFile(void)
    {
        // Open input file
        std::ifstream inputFile(m_inputFilePath);
        
        // Parse line by line
        while(std::getline(inputFile, line))
        {
            // Tokenize line
            std::vector<std::string> tokens = tokenize(line);
            
            // Parse command (backup/restore/delete specific)
            parseCommandInput(tokens);
        }
    }
    
    // Write output file for SAP HANA
    virtual void generateResultFile(std::vector<sOperationEntry> operationEntries)
    {
        std::ofstream outFileStream(m_outputFilePath);
        
        // Write version
        addVersionOutputFile(outFileStream);
        
        // Write results for each operation
        for(auto& entry : operationEntries)
        {
            writeCommandResults(outFileStream, entry);
        }
    }
    
protected:
    // Operation-specific parsing (implemented by subclasses)
    virtual void parseCommandInput(std::vector<std::string> tokens) = 0;
    
    // Operation-specific result formatting (implemented by subclasses)
    virtual void writeCommandResults(std::ostream& outFileStream, 
                                     sOperationEntry operationEntry) = 0;
};
```

**Backup IPC Format**:
```
Input:  #PIPE <pipe_path>
Output: #SAVED <external_backup_id> <pipe_path> <size>
```

**Restore IPC Format**:
```
Input:  <external_backup_id> #PIPE <pipe_path>
Output: #RESTORED <external_backup_id> <pipe_path>
```

**Delete IPC Format**:
```
Input:  <external_backup_id>
Output: #DELETED <external_backup_id>
```

## Summary of Design Pattern Benefits

| Pattern | Primary Benefit | Impact on Backint Interface |
|---------|----------------|---------------------------|
| **Strategy** | Operation encapsulation | Clean separation of backup/restore/delete logic |
| **Template Method** | Consistent workflow | Ensures proper init/cleanup in all operations |
| **Command** | Request/Response decoupling | Enables async execution and error handling |
| **Observer** | Event-driven execution | Controller responds to command readiness |
| **Singleton** | Global coordination | Single point for configuration and state |
| **Factory** | Object creation abstraction | Operation-specific IPC handlers |
| **Adapter** | Interface translation | SAP Backint ↔ Catalyst API bridge |

## Conclusion

The SAP HANA plugin employs a rich set of design patterns to create a robust, maintainable, and extensible architecture:

1. **Strategy Pattern**: Provides flexibility in operation implementation while maintaining common interfaces
2. **Template Method**: Enforces consistent workflow across all operations
3. **Command Pattern**: Enables sophisticated request handling with timing, retries, and error management
4. **Observer Pattern**: Decouples command initiation from execution
5. **Singleton Pattern**: Ensures coordinated resource management
6. **Factory Pattern**: Simplifies creation of operation-specific components
7. **Adapter Pattern**: Bridges SAP Backint and Catalyst storage APIs

These patterns work together to create a system that is:
- **Extensible**: New operations can be added with minimal changes
- **Maintainable**: Clear separation of concerns
- **Testable**: Interfaces enable mocking and unit testing
- **Reliable**: Consistent error handling and resource management
- **Performant**: Supports parallel execution with proper synchronization
