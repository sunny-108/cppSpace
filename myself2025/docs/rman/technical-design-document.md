# RMAN Plugin - High-Level Technical Design Document

**Version:** 1.0  
**Date:** January 15, 2026  
**Product:** HPE StoreOnce Catalyst Plugin for Oracle RMAN  
**Architecture:** Enterprise Backup & Recovery Solution

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [System Architecture Overview](#system-architecture-overview)
3. [Design Patterns](#design-patterns)
4. [Component Architecture](#component-architecture)
5. [Data Flow](#data-flow)
6. [Thread Safety & Concurrency](#thread-safety--concurrency)
7. [Error Handling Strategy](#error-handling-strategy)
8. [Configuration Management](#configuration-management)
9. [Key Design Decisions](#key-design-decisions)
10. [Extension Points](#extension-points)

---

## 1. Executive Summary

The RMAN Plugin is a sophisticated backup solution that bridges Oracle Recovery Manager (RMAN) with HPE StoreOnce Catalyst storage systems. The plugin implements Oracle's SBT (System Backup to Tape) API v2.0, enabling high-performance, parallel backup and restore operations.

### Key Capabilities

- **Multi-channel parallel backups** - Simultaneous backup of multiple tablespaces
- **Context-isolated operations** - Thread-safe concurrent execution
- **Metadata-rich storage** - Comprehensive backup tracking and management
- **Content-aware deduplication** - Optimized storage utilization
- **Server-managed copies** - Automated backup replication
- **Demultiplexed streams** - Separate metadata and data handling

### Technology Stack

- **Language:** C++ (C++11 compatible)
- **API:** Oracle SBT 2.0
- **Platform Support:** Linux, AIX, HP-UX, Solaris, Windows
- **Build System:** Gradle with native compilation
- **Dependencies:** Catalyst SDK, Boost libraries

---

## 2. System Architecture Overview

### 2.1 High-Level Architecture

```
┌────────────────────────────────────────────────────────────────┐
│                        Oracle RMAN                              │
│                 (Recovery Manager Client)                       │
└───────────────┬────────────────────────────────────────────────┘
                │ SBT API 2.0
                │ (sbtinit2, sbtbackup, sbtwrite2, etc.)
                ▼
┌────────────────────────────────────────────────────────────────┐
│                      RMAN Plugin Layer                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                   API.c / glue.cpp                        │  │
│  │            (Entry Points & Function Routing)             │  │
│  └───────────────────────┬──────────────────────────────────┘  │
│                          ▼                                      │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              RmanCommandManager                           │  │
│  │         (Singleton - Command Orchestration)               │  │
│  └────┬────────────────────────────────────┬────────────────┘  │
│       │                                    │                    │
│       ▼                                    ▼                    │
│  ┌─────────────────┐            ┌──────────────────────┐       │
│  │ RmanContext     │            │  RmanApiDataManager  │       │
│  │ Manager         │            │  (Data Conversion)   │       │
│  │ (Singleton)     │            └──────────────────────┘       │
│  └────┬────────────┘                                            │
│       │ Manages                                                 │
│       ▼                                                         │
│  ┌─────────────────────────────────────────────┐               │
│  │        RmanCommandContext (Per Channel)     │               │
│  │  ┌────────────┬────────────┬────────────┐  │               │
│  │  │ CTX_MAIN   │ CTX_META   │ CTX_DATA   │  │               │
│  │  └────────────┴────────────┴────────────┘  │               │
│  └─────────────────┬───────────────────────────┘               │
│                    │                                            │
└────────────────────┼────────────────────────────────────────────┘
                     │
                     ▼
┌────────────────────────────────────────────────────────────────┐
│                   Common Plugin Framework                       │
│  ┌──────────────┐  ┌─────────────────┐  ┌─────────────────┐   │
│  │ Plugin       │  │ CatalystBackup  │  │ RmanMetadata    │   │
│  │ Controller   │  │ Storage         │  │ Manager         │   │
│  └──────────────┘  └─────────────────┘  └─────────────────┘   │
└───────────────┬────────────────────────────────────────────────┘
                │ Catalyst SDK
                ▼
┌────────────────────────────────────────────────────────────────┐
│              HPE StoreOnce Catalyst Storage                     │
│            (Deduplicated Backup Repository)                     │
└────────────────────────────────────────────────────────────────┘
```

### 2.2 Layered Architecture

The plugin follows a **3-tier layered architecture**:

#### **Tier 1: API/Interface Layer**
- **Purpose:** Oracle SBT API compliance
- **Components:** `API.c`, `glue.cpp`, `glue.h`
- **Responsibilities:**
  - SBT function entry points
  - Parameter validation
  - API contract enforcement
  - Error code translation

#### **Tier 2: Business Logic Layer**
- **Purpose:** Core backup/restore orchestration
- **Components:** Managers, Contexts, Data Handlers
- **Responsibilities:**
  - Command execution workflow
  - Context lifecycle management
  - Data transformation
  - Metadata generation
  - State management

#### **Tier 3: Integration Layer**
- **Purpose:** Storage backend communication
- **Components:** Catalyst SDK wrappers
- **Responsibilities:**
  - Network communication
  - Storage operations
  - Credential management
  - Connection pooling

---

## 3. Design Patterns

The RMAN Plugin employs several well-established design patterns to ensure maintainability, scalability, and robustness.

### 3.1 Singleton Pattern

**Purpose:** Ensure single instance of critical managers throughout the plugin lifecycle.

#### Implementation Examples:

**RmanCommandManager** (Thread-Safe Singleton)
```cpp
class RmanCommandManager : public COMMON::ICommandManager {
protected:
    static Logger* s_logger;
    static RmanCommandManager* s_instance;
    
    // Private constructor
    RmanCommandManager(const std::string& mmlConfigFile);

public:
    static RmanCommandManager* getInstance(Logger* pLogger, 
                                          const std::string& mmlConfigFile);
};
```

**RmanContextManager** (Thread-Safe Singleton)
```cpp
class RmanContextManager {
protected:
    static Logger* s_logger;
    static RmanContextManager* s_instance;
    static thrLock_t s_mutex;  // Thread synchronization
    
    RmanContextManager();  // Private constructor

public:
    static RmanContextManager* getInstance(Logger* logger);
};
```

**RmanApiDataManager** (Stateless Singleton)
```cpp
class RmanApiDataManager {
protected:
    static Logger* s_logger;
    static RmanApiDataManager* s_instance;
    RmanApiDataManager();

public:
    static RmanApiDataManager* getInstance(Logger* pLogger);
};
```

**Benefits:**
- **Global access point** to critical services
- **Controlled instantiation** - prevents multiple instances
- **Thread-safe initialization** with mutex protection
- **Lazy initialization** - created only when needed

**Usage Pattern:**
```cpp
// In glue.cpp
static RmanCommandManager* const commandManager = 
    RmanCommandManager::getInstance(&myLogger, mmlConfigFile);
```

---

### 3.2 Factory Pattern

**Purpose:** Abstract object creation and enable runtime polymorphism.

#### RmanMetadataManagerFactory

```cpp
class RmanMetadataManagerFactory : 
    public COMMON::ICatalystMetadataManagerFactory {
public:
    RmanMetadataManager* getNewMetadataManager(LoggerBase* pLogger) const;
};
```

**Implementation:**
```cpp
RmanMetadataManager* RmanMetadataManagerFactory::getNewMetadataManager(
    LoggerBase* pLogger) const {
    return new RmanMetadataManager(pLogger, MML_VENDOR_ID, MML_VERSION);
}
```

**Benefits:**
- **Decouples creation** from usage
- **Polymorphic instantiation** - returns base class pointer
- **Testability** - easily mock factories for unit tests
- **Extension** - new metadata formats without changing client code

**Usage:**
```cpp
static RmanMetadataManagerFactory metadataManagerFactory;
static CatalystBackupStorage* const backupStorage = 
    CatalystBackupStorage::getInstance(&myLogger, &metadataManagerFactory);
```

#### ConfigOutputterFactory & JSONParserFactory

Used in the config upgrade utility:

```cpp
class ConfigOutputterFactory {
public:
    static IConfigOutputter* getOutputter(int version, 
                                          const std::string& filePath);
};

class JSONParserFactory {
public:
    static IJSONParser* getParser(int version, 
                                  const std::string& filePath);
};
```

---

### 3.3 Observer Pattern

**Purpose:** Loose coupling between command execution and storage operations.

#### Architecture

```
┌─────────────────────────┐         ┌──────────────────────┐
│  RmanCommandManager     │         │  PluginController    │
│  (Observable/Subject)   │────────▶│  (Observer)          │
└─────────────────────────┘ notify  └──────────────────────┘
         │                                      │
         │ attachObserver()                    │
         │ notifyObservers()                   │
         ▼                                      ▼
   Command Execution               Storage Operations
   (Context Changes)               (Backup/Restore)
```

#### Implementation

**Observable (RmanCommandManager):**
```cpp
class RmanCommandManager : public COMMON::ICommandManager {
protected:
    COMMON::IObserver* m_observer;

    void attachObserver(COMMON::IObserver* observer);
    void detachObserver(COMMON::IObserver* observer);
    void notifyObservers(const std::string& contextId) const;
};
```

**Observer (PluginController):**
```cpp
class PluginController : public COMMON::IObserver {
public:
    void update(const std::string& contextId) override;
    // Receives notifications and executes storage operations
};
```

**Notification Flow:**
```cpp
// In RmanCommandManager
#define EXECUTE_COMMAND_IN_CONTEXT(pCommandContext, cmdType) \
    pCommandContext->setCommandType(cmdType); \
    notifyObservers(pCommandContext->getContextId());

// Usage
EXECUTE_COMMAND_IN_CONTEXT(pCommandContext, PLUGIN_CMD_VALIDATE_BACKUP);
```

**Benefits:**
- **Decoupling** - Command manager doesn't know about storage details
- **Single Responsibility** - Clear separation of concerns
- **Extensibility** - Easy to add new observers
- **Event-driven** - Asynchronous notification model

---

### 3.4 Strategy Pattern

**Purpose:** Encapsulate algorithms and make them interchangeable.

#### Metadata Management Strategy

Different backup types require different metadata handling strategies:

```cpp
class CatalystMetadataManager {  // Base strategy
public:
    virtual void setPluginMetadata(...) = 0;
    virtual void renderPluginMetadata(...) const = 0;
    virtual void parsePluginMetadata(...) = 0;
};

class RmanMetadataManager : public CatalystMetadataManager {
    // RMAN-specific metadata strategy
    void setPluginMetadata(const Location_t& backup, 
                          const Options_t& options, 
                          const BackupParams_t& backupParams) override;
};
```

**Metadata Fields (RMAN Strategy):**
- Database ID, Name, Block Size
- File type, name, size
- Backup timestamps and durations
- Copy numbers and media pools
- Platform and version details

#### Credential Management Strategy

```cpp
class CredentialsManager {  // Base strategy
public:
    virtual std::string getPassword(...) = 0;
};

class RmanCredentialmanager : public CredentialsManager {
    std::string getPassword(std::string& configFile, 
                           char* pSerialNumberString, 
                           uint32_t SerialNumberStringSize, 
                           std::string credentialsFile = "", 
                           std::string userId = "", 
                           std::string serverAddr = "") override;
};
```

**Benefits:**
- **Algorithm encapsulation** - Easy to swap implementations
- **Open/Closed Principle** - Extend without modifying
- **Runtime selection** - Choose strategy dynamically

---

### 3.5 Command Pattern

**Purpose:** Encapsulate requests as objects with all necessary parameters.

#### Command Hierarchy

```
ICommandContext (Interface)
       │
       ├── Validate Config
       ├── Validate Backup
       ├── Transfer Backup
       ├── Finish Backup
       ├── Cancel Backup
       ├── Validate Restore
       ├── Transfer Restore
       ├── Finish Restore
       ├── Get Backup Info
       ├── Get Backup Exists
       └── Delete Backup
```

#### Request/Result Pattern

Each command has a paired request-result structure:

```cpp
class RmanCommandContext {
private:
    // Backup commands
    CMD::RequestToValidateBackup*  m_requestToValidateBackup;
    CMD::ResultsOfValidateBackup*  m_resultsOfValidateBackup;
    
    CMD::RequestToTransferBackup*  m_requestToTransferBackup;
    CMD::ResultsOfTransferBackup*  m_resultsOfTransferBackup;
    
    CMD::RequestToFinishBackup*    m_requestToFinishBackup;
    CMD::ResultsOfFinishBackup*    m_resultsOfFinishBackup;
    
    // Restore commands
    CMD::RequestToValidateRestore* m_requestToValidateRestore;
    CMD::ResultsOfValidateRestore* m_resultsOfValidateRestore;
    
    // ... more commands
};
```

**Command Execution Pattern:**
```cpp
// 1. Prepare request
RequestToValidateBackup* request = context->getRequestToValidateBackup();
request->setStorageLocation(location);
request->setBackupParams(params);

// 2. Execute command
EXECUTE_COMMAND_IN_CONTEXT(context, PLUGIN_CMD_VALIDATE_BACKUP);
notifyObservers(contextId);  // Triggers actual execution

// 3. Check results
ResultsOfValidateBackup* results = context->getResultsOfValidateBackup();
if (results->isError()) {
    // Handle error
}
```

**Benefits:**
- **Parameterization** - Commands carry all needed data
- **Queueing** - Commands can be queued/logged
- **Undo/Redo** - Reversible operations (cancel backup)
- **Macro commands** - Composite operations

---

### 3.6 Adapter Pattern

**Purpose:** Bridge between Oracle's SBT API and internal plugin architecture.

#### RmanApiDataManager (Adapter)

Converts between Oracle's SBT data structures and plugin's internal formats:

```cpp
class RmanApiDataManager {
public:
    // Oracle SBT → Plugin internal
    static void getBackupParams(
        char* backup_file_name,         // SBT format
        sbtobject* file_info,           // SBT format
        size_t block_size,
        size_t max_size,
        unsigned int copy_number,
        unsigned int media_pool,
        COMMON::BackupParams_t& backupParams  // Plugin format
    );
    
    // Oracle SBT → Plugin internal
    static void getRestoreParams(
        char* backup_file_name,         // SBT format
        size_t block_size,
        COMMON::RestoreParams_t& restoreParams  // Plugin format
    );
    
    // Plugin internal → Oracle SBT
    static sbtbfinfo* setBackupInfoArray(
        const size_t arrayLength,
        COMMON::BackupMap_t& objectsMap  // Plugin format → SBT format
    );
};
```

**Data Structure Translation:**
```cpp
// SBT structures
typedef struct sbtobject { ... } sbtobject;
typedef struct sbtbfinfo { ... } sbtbfinfo;

// Plugin structures
struct BackupParams_t {
    std::string filename;
    uint64_t blockSize;
    uint64_t maxSize;
    uint8_t copyNumber;
    uint32_t mediaPool;
};
```

**Benefits:**
- **Interface compatibility** - Oracle API compliance
- **Impedance matching** - Different data representations
- **Insulation** - Changes to one side don't affect the other

---

### 3.7 State Management Pattern

**Purpose:** Track operation lifecycle and ensure valid state transitions.

#### Context State

```cpp
class RmanCommandContext {
public:
    enum eOperationType { 
        OPERATION_UNKNOWN, 
        OPERATION_BACKUP, 
        OPERATION_RESTORE 
    };
    
    enum eContextType { 
        CTX_MAIN,   // Main control context
        CTX_META,   // Metadata stream
        CTX_DATA,   // Data stream
        NUM_CONTEXT_TYPES 
    };

private:
    PluginCommand_t m_commandType;  // Current command
    FlowDirection_t m_flowDirection; // BACKUP/RESTORE
    bool m_isBackupRestoreStarting;
    bool m_isObjectCreated;
    size_t m_bytesTransfered;
};
```

#### State Transitions

```
INIT → VALIDATE → TRANSFER → FINISH → END
                     ↓
                  CANCEL
```

**Validation Logic:**
```cpp
bool RmanCommandContext::getIsBackupRestoreStarting() const {
    return m_isBackupRestoreStarting;
}

bool RmanCommandContext::getIsObjectCreated() const {
    return m_isObjectCreated;
}
```

---

### 3.8 Resource Acquisition Is Initialization (RAII)

**Purpose:** Automatic resource management through object lifecycle.

#### Shared Memory Management

```cpp
class RmanCommandContext {
private:
    COMMON::PlatformSharedMemory* m_pSharedResource;

public:
    // Acquire resource
    void initSharedResourceAndSetOriginLocation() {
        m_pSharedResource = PlatformSharedMemory::getOrigInstance(...);
        // Resource automatically managed
    }
    
    // Release resource
    void freeSharedResource() {
        delete m_pSharedResource;
        m_pSharedResource = NULL;
    }
    
    // Automatic cleanup in destructor
    ~RmanCommandContext() {
        freeSharedResource();
        // Other cleanup...
    }
};
```

#### Timer Management

```cpp
class RmanCommandContext {
private:
    COMMON::PluginTimer* m_isvTimer;  // RMAN time
    COMMON::PluginTimer* m_mmlTimer;  // Plugin time
    COMMON::PluginTimer* m_catTimer;  // Catalyst time

public:
    RmanCommandContext(...) {
        m_isvTimer = new PluginTimer(false);
        m_mmlTimer = new PluginTimer(false);
        m_catTimer = new PluginTimer(true);  // Has sub-timer
    }
    
    ~RmanCommandContext() {
        delete m_isvTimer;
        delete m_mmlTimer;
        delete m_catTimer;
    }
};
```

**Benefits:**
- **Exception safety** - Resources cleaned up automatically
- **No memory leaks** - Deterministic cleanup
- **Scope-bound** - Resource lifetime tied to object

---

## 4. Component Architecture

### 4.1 Core Components

#### 4.1.1 RmanCommandManager

**Role:** Central orchestrator for all SBT API commands

**Responsibilities:**
- Command routing and execution
- Context lifecycle coordination
- Observer notification
- Error handling and reporting
- Configuration management

**Key Methods:**
```cpp
class RmanCommandManager {
public:
    // SBT API implementations
    int api_sbtinit(sbt_mms_fptr* pCallbacks, bserc *se, ...);
    int api_sbtinit2(void *ctx, unsigned long flags, ...);
    int api_sbtbackup(void *ctx, unsigned long flags, ...);
    int api_sbtrestore(void *ctx, unsigned long flags, ...);
    int api_sbtwrite2(void *ctx, unsigned long flags, void *buf);
    int api_sbtread2(void *ctx, unsigned long flags, void *buf);
    int api_sbtclose2(void *ctx, unsigned long flags);
    int api_sbtend(void *ctx, unsigned long flags);
    int api_sbtinfo2(void *ctx, unsigned long flags, ...);
    int api_sbtremove2(void *ctx, unsigned long flags, ...);
    int api_sbterror(void *ctx, unsigned long flags, ...);
    
    // Context management
    PluginCommand_t getCommandType(const std::string& contextId) const;
    
protected:
    RmanApiDataManager* m_apiDataManager;
    COMMON::IObserver* m_observer;  // PluginController
};
```

**Singleton Initialization:**
```cpp
static RmanCommandManager* const commandManager = 
    RmanCommandManager::getInstance(&myLogger, mmlConfigFile);
```

---

#### 4.1.2 RmanContextManager

**Role:** Thread-safe management of execution contexts

**Responsibilities:**
- Context creation and destruction
- Context-to-pointer mapping
- Thread synchronization
- Context isolation enforcement

**Key Methods:**
```cpp
class RmanContextManager {
public:
    static RmanContextManager* getInstance(Logger* logger);
    
    void createContexts(void *const ctx);
    void destroyContexts(void *const ctx);
    
    void getContextsFromCtx(void *const ctx, 
                           RmanCommandContext::ContextContainer *pContextContainer);
    
    RmanCommandContext* getContextFromCtx(void *const ctx);
    RmanCommandContext* getContextFromMap(const std::string& contextId) const;

protected:
    static thrLock_t s_mutex;  // Thread-safe access
    std::map<std::string, RmanCommandContext*> m_commandContexts;
    
    void createContext(const std::string contextId, 
                      const RmanCommandContext::eContextType contextType, 
                      void *const ctx);
    void destroyContext(void *const ctx);
};
```

**Context Hierarchy:**
```
void *ctx (Oracle API context pointer)
    │
    ├── CTX_MAIN  (RmanCommandContext) - Main control
    ├── CTX_META  (RmanCommandContext) - Metadata stream
    └── CTX_DATA  (RmanCommandContext) - Data stream
```

---

#### 4.1.3 RmanCommandContext

**Role:** Per-channel execution state container

**Responsibilities:**
- Store channel-specific state
- Maintain command request/result pairs
- Track timing metrics
- Manage shared memory resources
- Hold backup/restore parameters

**Key Attributes:**
```cpp
class RmanCommandContext : public COMMON::ICommandContext {
public:
    enum eContextType { CTX_MAIN, CTX_META, CTX_DATA, NUM_CONTEXT_TYPES };
    typedef RmanCommandContext* ContextContainer[NUM_CONTEXT_TYPES];
    
    enum eOperationType { OPERATION_UNKNOWN, OPERATION_BACKUP, OPERATION_RESTORE };

private:
    // Identity
    Logger* m_logger;
    std::string m_contextId;        // UUID
    eContextType m_contextType;
    
    // Operation state
    PluginCommand_t m_commandType;
    FlowDirection_t m_flowDirection;
    bool m_isBackupRestoreStarting;
    bool m_isObjectCreated;
    
    // Configuration
    bool m_isBackupContentAware;
    bool m_isServerManagedCopy;
    uint8_t m_maxCopies;
    uint8_t m_copyIndex;
    
    // Block dimensions
    uint64_t m_databaseBlockSize;
    uint64_t m_numBlocksInBuffer;
    uint64_t m_demuxMetaPartSize;
    uint64_t m_demuxDataPartSize;
    uint64_t m_implicitFlushFreq;
    
    // Transfer tracking
    size_t m_bytesTransfered;
    
    // Storage locations (original + copies)
    Location_t m_storageLocation[IBackupStorage::MAX_NUM_COPIES];
    
    // Shared memory (for server-managed copies)
    PlatformSharedMemory* m_pSharedResource;
    
    // Timing
    PluginTimer* m_isvTimer;  // RMAN time
    PluginTimer* m_mmlTimer;  // Plugin time
    PluginTimer* m_catTimer;  // Catalyst time
    
    // Command objects (Request/Result pairs)
    RequestToValidateBackup*  m_requestToValidateBackup;
    ResultsOfValidateBackup*  m_resultsOfValidateBackup;
    // ... (all other commands)
    
    // Error handling
    bool m_hasError;
    int m_rmanErrorCode;
    char* m_errorText;
};
```

---

#### 4.1.4 RmanApiDataManager

**Role:** Data transformation between SBT and plugin formats

**Responsibilities:**
- SBT API property generation
- Parameter conversion (SBT ↔ Plugin)
- Backup info array construction
- Filename validation and parsing

**Key Methods:**
```cpp
class RmanApiDataManager {
public:
    // API properties
    static sbtinit_output* getApiProperties(Logger* pLogger, 
                                           sbt_mms_fptr* pCallbacks);
    
    // Parameter conversion
    static void getBackupParams(char* backup_file_name, 
                               sbtobject* file_info, 
                               size_t block_size, 
                               size_t max_size, 
                               unsigned int copy_number, 
                               unsigned int media_pool, 
                               BackupParams_t& backupParams);
    
    static void getRestoreParams(char* backup_file_name, 
                                size_t block_size, 
                                RestoreParams_t& restoreParams);
    
    // Backup info array construction
    static sbtbfinfo* setBackupInfoArray(const size_t arrayLength, 
                                        BackupMap_t& objectsMap);
    
    // Filename handling
    static bool validateFilename(const std::string& filename, 
                                const uint64_t databaseId, 
                                const uint64_t copyNumber);
    
    static bool getSharedFilename(const std::string& suffix, 
                                 const std::string& filename, 
                                 std::string& sharedFilename, 
                                 std::string& errorMessage);
    
protected:
    static sbtinit_output* s_apiProperties;
};
```

**Supported Filename Formats:**
```
Format #1: *_c     (e.g., backup_1)
Format #5: c-IIIIIIIIII-DDDDDDDD-QQ  (e.g., c-1234567890-00000008-00)
Format #6: *_c-IIIIIIIIII-DDDDDDDD-QQ (e.g., prefix_c-1234567890-00000008-00)
```

---

#### 4.1.5 RmanMetadataManager

**Role:** RMAN-specific metadata generation and parsing

**Responsibilities:**
- Plugin metadata serialization
- Backup metadata generation
- JSON rendering and parsing
- Timer report generation

**Key Metadata Fields:**
```cpp
class RmanMetadataManager : public COMMON::CatalystMetadataManager {
protected:
    // Plugin-level metadata
    std::string m_sdkVersion;
    std::string m_mmlVersion;
    std::string m_mmlPlatform;
    
    // Backup-level metadata
    std::string m_databaseName;
    long long m_databaseId;
    std::string m_fileType;
    std::string m_fileName;
    long long m_fileSize;
    
    // Location metadata
    std::string m_backupClient;
    std::string m_backupServer;
    std::string m_backupStore;
    
    // Operation metadata
    int m_backupStatus;
    int m_copyNumber;
    int m_mediaPool;
    
    // Block metadata
    int m_databaseBlockSize;
    int m_demuxMetaPartSize;
    
    // Timing metadata
    std::string m_backupStarted;
    std::string m_isvTimeElapsed;   // RMAN time
    std::string m_mmlTimeElapsed;   // Plugin time
    std::string m_catTimeElapsed;   // Catalyst time
    std::string m_catCopyTimeElapsed;  // Copy operation time
};
```

---

#### 4.1.6 PluginController

**Role:** Storage operations orchestrator (Observer)

**Responsibilities:**
- Receive command notifications
- Execute storage operations
- Manage backup storage
- Coordinate with Catalyst SDK

**Key Methods:**
```cpp
class PluginController : public COMMON::IObserver {
public:
    static PluginController* getInstance(Logger* pLogger, 
                                        ICommandManager* commandManager, 
                                        IBackupStorage* backupStorage);
    
    void init();
    
    // IObserver interface
    void update(const std::string& contextId) override;
    
protected:
    ICommandManager* m_commandManager;
    IBackupStorage* m_backupStorage;
};
```

**Notification Handling:**
```
1. RmanCommandManager calls notifyObservers(contextId)
2. PluginController::update(contextId) is invoked
3. Controller retrieves context and command type
4. Executes appropriate storage operation
5. Updates context with results
```

---

### 4.2 Supporting Components

#### 4.2.1 CatalystBackupStorage

**Role:** Catalyst SDK abstraction layer

**Responsibilities:**
- Storage operations (validate, transfer, finish, delete)
- Session management
- Network communication
- Deduplication handling

#### 4.2.2 RmanMetadataManagerFactory

**Role:** Metadata manager instantiation

**Responsibilities:**
- Create RmanMetadataManager instances
- Provide to CatalystBackupStorage
- Enable polymorphic metadata handling

#### 4.2.3 RmanCredentialManager

**Role:** Credential retrieval and management

**Responsibilities:**
- Password retrieval from credential file
- StoreOnce authentication
- Serial number handling

#### 4.2.4 PluginConfigManager

**Role:** Configuration file parsing

**Responsibilities:**
- Read plugin configuration
- Parse storage locations
- Extract backup options
- Validate configuration

---

## 5. Data Flow

### 5.1 Backup Operation Flow

```
┌─────────────┐
│ Oracle RMAN │
└──────┬──────┘
       │ sbtinit()
       ▼
┌────────────────────────────────────────────┐
│ 1. INITIALIZATION                          │
│ ─────────────────────────────────────────  │
│ • glue_sbtinit()                           │
│ • Initialize Logger, Config, Controllers   │
│ • Return API properties                    │
└────────────┬───────────────────────────────┘
             │ sbtinit2(ctx)
             ▼
┌────────────────────────────────────────────┐
│ 2. CONTEXT CREATION                        │
│ ─────────────────────────────────────────  │
│ • glue_sbtinit2()                          │
│ • RmanContextManager::createContexts()     │
│ • Create CTX_MAIN, CTX_META, CTX_DATA      │
│ • Parse SBT_PARMS                          │
│ • Load configuration                       │
└────────────┬───────────────────────────────┘
             │ sbtbackup(ctx, filename, ...)
             ▼
┌────────────────────────────────────────────┐
│ 3. BACKUP VALIDATION                       │
│ ─────────────────────────────────────────  │
│ • glue_sbtbackup()                         │
│ • RmanCommandManager::api_sbtbackup()      │
│ • Validate filename format                 │
│ • Check backup doesn't exist               │
│ • Prepare RequestToValidateBackup          │
│ • EXECUTE_COMMAND_IN_CONTEXT()             │
│ • notifyObservers()                        │
│ • PluginController::update()               │
│ • backupStorage->validate()                │
└────────────┬───────────────────────────────┘
             │ sbtwrite2(ctx, buf) [repeated]
             ▼
┌────────────────────────────────────────────┐
│ 4. DATA TRANSFER                           │
│ ─────────────────────────────────────────  │
│ • glue_sbtwrite2()                         │
│ • RmanCommandManager::api_sbtwrite2()      │
│ • First call: Start transfer session       │
│ •   - RequestToTransferBackup              │
│ •   - EXECUTE_COMMAND_IN_CONTEXT()         │
│ •   - backupStorage->beginTransfer()       │
│ • Subsequent calls:                        │
│ •   - Write buffer to storage              │
│ •   - Track bytes transferred              │
│ •   - Update progress                      │
│ • Demultiplexing (if enabled):             │
│ •   - Separate meta/data streams           │
│ •   - Route to appropriate context         │
└────────────┬───────────────────────────────┘
             │ sbtclose2(ctx)
             ▼
┌────────────────────────────────────────────┐
│ 5. BACKUP FINALIZATION                     │
│ ─────────────────────────────────────────  │
│ • glue_sbtclose2()                         │
│ • RmanCommandManager::api_sbtclose2()      │
│ • RequestToFinishBackup                    │
│ • Set backup metadata                      │
│ • Server-managed copies (if configured):   │
│ •   - Original: Store location in shmem    │
│ •   - Copies: Read location from shmem     │
│ •   - Schedule copy operation              │
│ • EXECUTE_COMMAND_IN_CONTEXT()             │
│ • backupStorage->endTransfer()             │
│ • Update metadata with timing info         │
└────────────┬───────────────────────────────┘
             │ sbtend(ctx)
             ▼
┌────────────────────────────────────────────┐
│ 6. CLEANUP                                 │
│ ─────────────────────────────────────────  │
│ • glue_sbtend()                            │
│ • RmanCommandManager::api_sbtend()         │
│ • RmanContextManager::destroyContexts()    │
│ • Free shared memory resources             │
│ • Delete command contexts                  │
│ • Log timing reports                       │
└────────────────────────────────────────────┘
```

### 5.2 Restore Operation Flow

```
┌─────────────┐
│ Oracle RMAN │
└──────┬──────┘
       │ sbtinit() / sbtinit2(ctx)
       ▼
┌────────────────────────────────────────────┐
│ 1-2. INITIALIZATION (Same as backup)       │
└────────────┬───────────────────────────────┘
             │ sbtrestore(ctx, filename, ...)
             ▼
┌────────────────────────────────────────────┐
│ 3. RESTORE VALIDATION                      │
│ ─────────────────────────────────────────  │
│ • glue_sbtrestore()                        │
│ • RmanCommandManager::api_sbtrestore()     │
│ • Validate filename format                 │
│ • Check backup EXISTS                      │
│ • Retrieve backup metadata                 │
│ • Prepare RequestToValidateRestore         │
│ • notifyObservers()                        │
│ • backupStorage->validate()                │
└────────────┬───────────────────────────────┘
             │ sbtread2(ctx, buf) [repeated]
             ▼
┌────────────────────────────────────────────┐
│ 4. DATA TRANSFER                           │
│ ─────────────────────────────────────────  │
│ • glue_sbtread2()                          │
│ • RmanCommandManager::api_sbtread2()       │
│ • First call: Start restore session        │
│ •   - RequestToTransferRestore             │
│ •   - backupStorage->beginRestore()        │
│ • Subsequent calls:                        │
│ •   - Read buffer from storage             │
│ •   - Track bytes transferred              │
│ • Multiplexing (if demuxed):               │
│ •   - Recombine meta/data streams          │
└────────────┬───────────────────────────────┘
             │ sbtclose2(ctx)
             ▼
┌────────────────────────────────────────────┐
│ 5. RESTORE FINALIZATION                    │
│ ─────────────────────────────────────────  │
│ • glue_sbtclose2()                         │
│ • RmanCommandManager::api_sbtclose2()      │
│ • RequestToFinishRestore                   │
│ • backupStorage->endRestore()              │
│ • Log timing reports                       │
└────────────┬───────────────────────────────┘
             │ sbtend(ctx)
             ▼
┌────────────────────────────────────────────┐
│ 6. CLEANUP (Same as backup)                │
└────────────────────────────────────────────┘
```

### 5.3 Parallel Multi-Channel Flow

```
Oracle RMAN
     │
     ├─── Channel 1 (ctx1) ───┐
     │    Tablespace USERS     │
     │                         │
     ├─── Channel 2 (ctx2) ───┼─── Parallel
     │    Tablespace DATA      │    Execution
     │                         │
     └─── Channel 3 (ctx3) ───┘
          Tablespace INDEX

Each channel maintains:
• Independent RmanCommandContext
• Separate CTX_MAIN/META/DATA contexts
• Isolated buffer management
• Independent timing metrics
• Thread-safe access via RmanContextManager
```

---

## 6. Thread Safety & Concurrency

### 6.1 Concurrency Model

The RMAN Plugin supports **true parallelism** through multiple independent channels, each with its own execution context.

#### Thread Safety Mechanisms

**1. Context Isolation**
```cpp
// Each channel gets unique contexts
void RmanContextManager::createContexts(void *const ctx) {
    const std::string contextId = get_random_uuid();  // Unique UUID
    
    for (size_t i = 0; i < NUM_CONTEXT_TYPES; i++) {
        RmanCommandContext* pCommandContext = 
            new RmanCommandContext(s_logger, contextId + suffix[i], contextType);
        (*pContextContainer)[i] = pCommandContext;
    }
}
```

**2. Mutex Protection**
```cpp
class RmanContextManager {
    static thrLock_t s_mutex;  // Protects context map
    
    void createContext(...) {
        s_mutex.lock();
        try {
            m_commandContexts.insert(std::pair<...>(contextId, pCommandContext));
        } catch(...) { }
        s_mutex.unlock();
    }
};
```

**3. Per-Context State**
```cpp
class RmanCommandContext {
    // Each context maintains its own state
    size_t m_bytesTransfered;           // No sharing
    Location_t m_storageLocation[...];  // Independent
    PluginTimer* m_isvTimer;            // Per-context timing
    // ... all other state is isolated
};
```

**4. Shared Memory with Unique Keys**
```cpp
std::string RmanCommandContext::getSharedMemoryKey() const {
    std::string sharedFilename = getFilename();  // Unique per backup
    return "shm-" + MML_VENDOR_ID + "-" + sharedFilename;
}
```

### 6.2 Intentionally Shared Data

**Server-Managed Copies Map** (Protected by Context Manager mutex):
```cpp
// In RmanCommandContext.hpp
static std::map<std::string, 
                std::map<uint8_t, CMD::RequestToFinishBackup>> s_serverCopies;

// Access always through RmanContextManager (mutex-protected)
```

**Singleton Instances** (Lazy initialization with mutex):
```cpp
RmanCommandManager* RmanCommandManager::getInstance(...) {
    // Protected initialization
    return s_instance;  // Then thread-safe reads
}
```

### 6.3 Concurrency Guarantees

✅ **Safe:**
- Multiple channels executing simultaneously
- Concurrent backup of different tablespaces
- Parallel read/write operations
- Independent context creation/destruction

❌ **Not Safe (by design):**
- Accessing same RmanCommandContext from multiple threads
  - *Not needed - each channel has its own context*
- Modifying global config during operations
  - *Config is read-only after initialization*

---

## 7. Error Handling Strategy

### 7.1 Error Handling Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   Error Handling Layers                  │
├─────────────────────────────────────────────────────────┤
│ Layer 1: Oracle SBT Error Codes                         │
│ • SBT_API_SUCCESS (0)                                   │
│ • SBT_API_ERROR (-1)                                    │
│ • Specific error codes (SBT_ERROR_MM, SBT_ERROR_EXISTS) │
├─────────────────────────────────────────────────────────┤
│ Layer 2: Plugin Exceptions                              │
│ • PluginException (base)                                │
│ • Detailed error messages                               │
│ • Error context preservation                            │
├─────────────────────────────────────────────────────────┤
│ Layer 3: Context Error State                            │
│ • bool m_hasError                                       │
│ • int m_rmanErrorCode                                   │
│ • char* m_errorText                                     │
├─────────────────────────────────────────────────────────┤
│ Layer 4: Logging                                        │
│ • Logger with trace levels                              │
│ • Error, warning, info, verbose, debug                  │
│ • File-based logging with rotation                      │
└─────────────────────────────────────────────────────────┘
```

### 7.2 Error Handling Pattern

#### Centralized Exception Handling

```cpp
#define CATCH_ALL_TO_CONTEXT \
    catch(PluginException &e) { \
        if(pCommandContext != NULL) { \
            pCommandContext->setError(e); \
        } \
        status = SBT_API_ERROR; \
        if(pCommandContext != NULL) { \
            pCommandContext->setRmanError(SBT_ERROR_MM); \
        } \
        PLUGIN_TRACE_IF_ERROR_LOG(s_logger, e.getErrorText()); \
    } \
    catch(...) { \
        status = SBT_API_ERROR; \
        if(pCommandContext != NULL) { \
            pCommandContext->setRmanError(SBT_ERROR_MM); \
        } \
        PLUGIN_TRACE_IF_ERROR_LOG(s_logger, "unknown error"); \
    }
```

#### Usage Pattern

```cpp
int RmanCommandManager::api_sbtbackup(...) {
    int status = SBT_API_SUCCESS;
    RmanCommandContext* pCommandContext = NULL;
    
    try {
        // Get context
        pCommandContext = CONTEXT_MGR_RMAN->getContextFromCtx(ctx);
        
        // Validate parameters
        if (!validateFilename(backup_file_name, ...)) {
            THROW_PLUGIN_ERROR("Invalid filename", details);
        }
        
        // Execute operation
        EXECUTE_COMMAND_IN_CONTEXT(pCommandContext, PLUGIN_CMD_VALIDATE_BACKUP);
        
        // Check results
        ResultsOfValidateBackup* results = 
            pCommandContext->getResultsOfValidateBackup();
        
        if (results->isError()) {
            SET_ERROR_TEXT(pCommandContext, ...);
            status = SBT_API_ERROR;
        }
    }
    CATCH_ALL_TO_CONTEXT
    
    return status;
}
```

### 7.3 Error Code Mapping

#### SBT Error Codes
```cpp
// Oracle-defined error codes
#define SBT_API_SUCCESS      0
#define SBT_API_ERROR       -1

// Specific error types
#define SBT_ERROR_MM         /* Media manager error */
#define SBT_ERROR_EXISTS     /* Backup already exists */
#define SBT_ERROR_NOT_FOUND  /* Backup not found */
#define SBTINIT_ERROR_SYS    /* System error */
#define SBTINIT_ERROR_ARG    /* Invalid argument */
```

#### Context Error Management
```cpp
class RmanCommandContext {
public:
    bool hasError() const { return m_hasError; }
    
    void setError(PluginException& e) {
        m_hasError = true;
        setErrorText(e.getErrorText());
    }
    
    void setRmanError(const int rmanErrorCode) {
        m_rmanErrorCode = rmanErrorCode;
    }
    
    int getRmanErrorCode() const {
        return m_rmanErrorCode;
    }
    
    char* getErrorText() const {
        return m_errorText;
    }
};
```

### 7.4 Logging Strategy

#### Log Levels
```cpp
enum LogLevel {
    LOG_ERROR,    // Critical errors
    LOG_WARNING,  // Warnings and potential issues
    LOG_INFO,     // Informational messages
    LOG_VERBOSE,  // Detailed operation flow
    LOG_DEBUG     // Debug/development information
};
```

#### Logging Macros
```cpp
PLUGIN_TRACE_IF_ERROR_LOG(logger, message)
PLUGIN_TRACE_IF_INFO_LOG(logger, message)
PLUGIN_TRACE_IF_VERBOSE_LOG(logger, message)
PLUGIN_TRACE_DEBUG_LOG_VAR(logger, variable)
PLUGIN_TRACE_IF_VERBOSE_LOG_ENTER(logger)
PLUGIN_TRACE_IF_VERBOSE_LOG_EXIT(logger)
```

---

## 8. Configuration Management

### 8.1 Configuration Files

#### Primary Configuration: `plugin.conf` (JSON format)

**Location:** `$PLUGIN_HOME/config/plugin.conf`

**Structure:**
```json
{
    "version": "2.0",
    "plugin": {
        "vendor": "HPE",
        "name": "RMAN",
        "version": "4.1.0"
    },
    "storage": {
        "locations": [
            {
                "serverAddress": "storeonce.example.com",
                "storeName": "Catalyst1",
                "username": "admin",
                "passwordFile": "/path/to/.password"
            }
        ],
        "copies": [
            {
                "serverAddress": "storeonce-dr.example.com",
                "storeName": "Catalyst2",
                "username": "admin",
                "passwordFile": "/path/to/.password"
            }
        ]
    },
    "options": {
        "contentAwareBackup": "ENABLE",
        "applicationManagedCopies": "DISABLE",
        "storageMode": "FIXED",
        "compression": "ENABLE",
        "encryption": "DISABLE",
        "deduplication": "ENABLE"
    },
    "logging": {
        "logLevel": "INFO",
        "logFile": "/var/log/rman/plugin.log",
        "maxLogSize": "10MB",
        "maxLogFiles": 5
    }
}
```

#### Configuration Override via Environment

```bash
export CONFIG_FILE=/custom/path/to/plugin.conf
```

#### Configuration Parsing

```cpp
class PluginConfigManager {
public:
    void loadConfig(const std::string& configFile);
    
    // Getters
    std::vector<Location_t> getStorageLocations() const;
    Options_t getBackupOptions() const;
    LogLevel getLogLevel() const;
    
protected:
    void parseJSON(const std::string& jsonContent);
    void validateConfig();
};
```

### 8.2 Runtime Configuration (SBT_PARMS)

**Passed via RMAN command:**
```sql
ALLOCATE CHANNEL C1 DEVICE TYPE SBT 
PARMS 'SBT_LIBRARY=/path/to/libisvsupport_rman.so,
       ENV=(CONFIG_FILE=/path/to/plugin.conf,
            CATALYST_CONTENT_AWARE=ENABLE,
            CATALYST_STORAGE_MODE=FIXED)';
```

**Parsing in sbtinit2:**
```cpp
int RmanCommandManager::api_sbtinit2(..., sbtinit2_input *initin, ...) {
    // Parse SBTINIT2_SBT_PARMS entry
    for (size_t i = 0; initin[i].sbtinit2_input_type != SBTINIT2_INEND; i++) {
        if (initin[i].sbtinit2_input_type == SBTINIT2_SBT_PARMS) {
            char* parms = (char*)initin[i].sbtinit2_input_value;
            parseRmanParms(parms);  // Extract key=value pairs
        }
    }
}
```

---

## 9. Key Design Decisions

### 9.1 Why SBT 2.0 Only?

**Decision:** Implement only SBT API 2.0, make v1.0 functions stubs

**Rationale:**
- **Context-based architecture** - Required for thread safety
- **Modern RMAN versions** - All supported Oracle versions use 2.0
- **Parallel operations** - v1.0 cannot support multi-channel
- **Reduced complexity** - Single code path to maintain

**Implementation:**
```cpp
// SBT 1.0 functions return error
DLLEXPORT
int sbtwrite(bserc *se, int th, char *buf) {
    return SBT_API_ERROR;  // not implemented
}
```

---

### 9.2 Why Three Contexts Per Channel?

**Decision:** CTX_MAIN, CTX_META, CTX_DATA contexts for each channel

**Rationale:**
- **Demultiplexing support** - RMAN can multiplex meta/data
- **Independent streams** - Separate metadata from data
- **Performance optimization** - Parallel processing of streams
- **Flexibility** - Can handle both multiplexed and non-multiplexed

**Implementation:**
```cpp
enum eContextType { 
    CTX_MAIN,  // Main control and non-demuxed operations
    CTX_META,  // Metadata stream (when demuxed)
    CTX_DATA,  // Data stream (when demuxed)
    NUM_CONTEXT_TYPES 
};
```

---

### 9.3 Why Singleton Pattern for Managers?

**Decision:** Use singletons for CommandManager, ContextManager, etc.

**Rationale:**
- **Global state** - Plugin configuration applies globally
- **Resource sharing** - Logger, config shared across operations
- **Lifecycle management** - Single initialization/cleanup point
- **Oracle constraint** - SBT API has global scope

**Trade-offs:**
- ✅ Simplified access pattern
- ✅ Guaranteed single instance
- ❌ Harder to unit test (mitigated with dependency injection)
- ❌ Global state (acceptable for plugin architecture)

---

### 9.4 Why Observer Pattern for Storage Operations?

**Decision:** CommandManager notifies PluginController via observer

**Rationale:**
- **Separation of concerns** - Command parsing vs storage execution
- **Decoupling** - CommandManager doesn't know storage details
- **Testability** - Can mock observer for unit tests
- **Extensibility** - Easy to add new observers

**Benefits:**
```cpp
// CommandManager focus: Command orchestration
RmanCommandManager::api_sbtbackup(...) {
    // Parse parameters
    // Validate inputs
    // Prepare request
    EXECUTE_COMMAND_IN_CONTEXT(ctx, PLUGIN_CMD_VALIDATE_BACKUP);
    // Results handling
}

// PluginController focus: Storage operations
PluginController::update(contextId) {
    // Execute storage operation
    // Interact with Catalyst SDK
    // Update results
}
```

---

### 9.5 Why Shared Memory for Server-Managed Copies?

**Decision:** Use platform shared memory for copy coordination

**Rationale:**
- **Process isolation** - RMAN creates separate processes per copy
- **Location sharing** - Copies need original backup location
- **Persistence** - Survives process boundaries
- **Efficiency** - Minimal overhead

**Implementation:**
```cpp
// Original backup stores location
void initSharedResourceAndSetOriginLocation() {
    m_pSharedResource = PlatformSharedMemory::getOrigInstance(
        m_logger, sharedMemoryKey, sizeof(pod)
    );
    m_pSharedResource->copyIntoSharedMemory(&pod);
}

// Copy backup retrieves location
void getOriginLocationAndFreeSharedResource(RequestToFinishBackup& request) {
    m_pSharedResource = PlatformSharedMemory::getCopyInstance(
        m_logger, sharedMemoryKey, sizeof(pod)
    );
    m_pSharedResource->copyFromSharedMemory(&pod);
    request.setOriginLocation(backup);
}
```

---

### 9.6 Why Request/Result Command Pattern?

**Decision:** Pair each command with Request and Result objects

**Rationale:**
- **Data encapsulation** - All command data in one place
- **Asynchronous execution** - Prepare request, execute later
- **Error handling** - Results contain success/error info
- **Traceability** - Request ID links request to result

**Pattern:**
```cpp
// Prepare
RequestToValidateBackup* request = ctx->getRequestToValidateBackup();
request->setStorageLocation(location);
request->setBackupParams(params);

// Execute (via observer notification)
EXECUTE_COMMAND_IN_CONTEXT(ctx, PLUGIN_CMD_VALIDATE_BACKUP);

// Check results
ResultsOfValidateBackup* results = ctx->getResultsOfValidateBackup();
if (results->isError()) {
    // Handle error
}
```

---

## 10. Extension Points

### 10.1 Adding New Metadata Fields

**Extension Point:** `RmanMetadataManager`

**Steps:**
1. Add field to `RmanMetadataManager` class
2. Update `setPluginMetadata()` to populate field
3. Update `renderPluginMetadata()` to serialize to JSON
4. Update `parsePluginMetadata()` to deserialize from JSON

**Example:**
```cpp
class RmanMetadataManager {
protected:
    std::string m_newField;  // 1. Add field

public:
    void setPluginMetadata(...) {
        // 2. Populate
        m_newField = extractNewField(backupParams);
    }
    
    void renderPluginMetadata(JSONObject& pluginObj) const {
        // 3. Serialize
        pluginObj["NewField"] = m_newField;
    }
    
    void parsePluginMetadata(JSONObject& pluginObj) {
        // 4. Deserialize
        m_newField = pluginObj["NewField"].asString();
    }
};
```

---

### 10.2 Supporting New Configuration Options

**Extension Point:** `PluginConfigManager`

**Steps:**
1. Add option to `Options_t` structure
2. Update configuration JSON schema
3. Update `parseJSON()` to read new option
4. Update `validateConfig()` to validate option
5. Use option in command execution logic

---

### 10.3 Adding New SBT Commands

**Extension Point:** `RmanCommandManager` and `RmanCommandContext`

**Steps:**
1. Add Request/Result classes for new command
2. Add command type to `PluginCommand_t` enum
3. Add API function to `RmanCommandManager`
4. Add request/result members to `RmanCommandContext`
5. Implement command execution in `PluginController`
6. Add glue function to `glue.cpp`

---

### 10.4 Platform-Specific Customizations

**Extension Point:** Platform abstraction layer

**Current Abstractions:**
- `Platform.hpp` - Platform-specific functions
- `PlatformSharedMemory` - Shared memory abstraction
- `Lock.h` - Mutex/lock abstraction

**Adding New Platform:**
1. Implement platform-specific shared memory
2. Implement platform-specific locking
3. Add build configuration for new platform
4. Test on target platform

---

## Summary

The RMAN Plugin demonstrates a **well-architected enterprise backup solution** that:

✅ **Follows industry-standard design patterns** - Singleton, Factory, Observer, Command, Strategy, Adapter

✅ **Ensures thread safety** - Per-context isolation, mutex protection, RAII

✅ **Maintains separation of concerns** - Layered architecture, clear responsibilities

✅ **Supports extensibility** - Well-defined extension points, polymorphic design

✅ **Handles errors gracefully** - Centralized error handling, comprehensive logging

✅ **Manages complexity** - Modular components, clear data flows

✅ **Scales horizontally** - True multi-channel parallelism

✅ **Integrates cleanly** - Adapter pattern for Oracle SBT API compliance

This architecture has proven robust across multiple platforms and Oracle versions, supporting high-performance enterprise backup requirements while maintaining code quality and maintainability.

---

**Document Revision History:**

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| 1.0 | 2026-01-15 | Architecture Team | Initial high-level technical design document |

---

**Related Documents:**
- SBT 1.0 to 2.0 Architecture Refactoring
- Multi-threaded Backup Orchestration (SAP HANA reference)
- Oracle SBT API Specification
- Catalyst SDK Integration Guide
