# SAP HANA Plugin — Design Patterns Analysis & C++17 Modernization

Based on: `z-Repo/so-d2d-catalyst_plugins/isvsupport/sap/hana/` and `sap/sapcommon/`

---

## 1. Template Method Pattern

**The most prominent pattern — defines the fixed backup/restore algorithm skeleton.**

**Original files:**
- `ISAPOperation.hpp` — abstract base with fixed sequence
- `ISAPHanaOperation.hpp` — HANA-specific intermediate
- `SAPHanaBackup.hpp`, `SAPHanaRestore.hpp`, `SAPHanaDelete.hpp`, `SAPHanaInquire.hpp` — concrete steps

**How it works:**
`ISAPOperation` defines pure virtual "hook" methods that subclasses must implement. The fixed call sequence is enforced in each concrete `operationStream()`:

```cpp
// Fixed sequence in SAPHanaBackup::operationStream(index):
initStreamInstrumentation(ctx);       // base class (fixed)
generateObjectName(...);              // base class (fixed)
setStorageParameters(...);            // virtual (overridden per operation)
validateConfig(...);                  // base class (fixed)
validateStorageOperation(...);        // virtual hook
doStorageOperation(...);              // virtual hook — actual data transfer
finishStorageOperation(...);          // virtual hook
```

Each of `SAPHanaBackup`, `SAPHanaRestore`, `SAPHanaDelete`, `SAPHanaInquire` provides its own implementation of `validateStorageOperation()`, `doStorageOperation()`, and `finishStorageOperation()`.

**C++17 Modernization Advantages:**
- Add `override` and `final` specifiers for compile-time enforcement of virtual method contracts — catches signature mismatches that silently create new virtual methods in the original
- Replace `bool` return + `_GOTO_(out)` error handling with `std::optional<ErrorInfo>` — eliminates goto-based control flow
- Use `if constexpr` for compile-time operation-type branching where runtime polymorphism isn't needed

---

## 2. IPC Command Pattern

**Two-level hierarchy encapsulating inter-process communication with SAP HANA's backint interface.**

**Original files:**
- `ISAPIPCCommand.hpp` — abstract base command interface
- `ISAPHanaIPCCommand.hpp` — HANA-specific intermediate
- `SAPHanaBackupIPCCommand.hpp` — parses backup input, writes backup results
- `SAPHanaRestoreIPCCommand.hpp` — parses restore input, writes restore results
- `SAPHanaDeleteIPCCommand.hpp` — parses delete input, writes delete results
- `SAPHanaInquireIPCCommand.hpp` — parses inquire input, writes inquire results

**How it works:**
SAP HANA communicates with the backint plugin via files — an input file with commands and an output file with results. Each IPC command encapsulates parsing its specific input format and writing its specific result format:

```cpp
// ISAPIPCCommand.hpp — Command interface
class ISAPIPCCommand {
public:
    virtual void processInputFile(void) = 0;                              // parse input
    void generateResultFile(const std::vector<sOperationEntry>&);         // write output
    virtual void parseCommandInput(std::vector<std::string> tokens) = 0;  // parse one line
    virtual void writeCommandResults(std::ostream&, sOperationEntry) = 0; // write one result
};

// Usage in ISAPOperation::init():
m_pIPCCommand->processInputFile();                          // parse all entries
m_operationEntries = m_pIPCCommand->getOperationEntries();  // get parsed entries

// Usage in ISAPOperation::finish():
m_pIPCCommand->generateResultFile(m_operationEntries);      // write all results
```

Each concrete command knows how to parse its input format (backup: `#SAVED <filename>`, restore: `#<ebid> <filename> <dest>`, etc.) and generate proper output.

**C++17 Modernization Advantages:**
- Replace raw `ISAPIPCCommand*` with `std::unique_ptr<ISAPIPCCommand>` — current code uses `new`/`delete` manually, risking leaks on exception paths
- Use `std::variant<BackupIPCCmd, RestoreIPCCmd, DeleteIPCCmd, InquireIPCCmd>` + `std::visit` for closed command sets — eliminates vtable overhead for a fixed set of 4 commands
- Replace `std::vector<std::string> tokens` parsing with `std::string_view` — avoids string copies during input file parsing
- Use structured bindings for multi-value returns from parse methods:
  ```cpp
  // Original: multiple out-parameters
  void parseCommandInput(std::vector<std::string> tokens);
  
  // Modern: structured return
  auto [fileName, objectName, ebid] = parseCommandInput(line);
  ```

---

## 3. Observer Pattern

**Event-driven command dispatch between manager and controller.**

**Original files:**
- `IObservable.hpp` — Subject interface
- `IObserver.hpp` — Observer interface
- `PluginCommandManager.hpp` — concrete Subject (holds observer pointer)
- `PluginController.hpp` — concrete Observer (handles events)

```cpp
// IObservable.hpp
class IObservable {
    virtual void attachObserver(IObserver* observer) = 0;
    virtual void detachObserver(IObserver* observer) = 0;
    virtual void notifyObservers(const std::string& contextId) const = 0;
};

// IObserver.hpp
class IObserver {
    virtual void handleEvent(const std::string& contextId) const = 0;
};
```

`PluginCommandManager` notifies `PluginController` when a command context is ready to execute.

**C++17 Modernization Advantages:**
- Replace two interface files + virtual dispatch with `std::function<void(const std::string&)>` — eliminates `IObservable.hpp`, `IObserver.hpp`, and the boilerplate `attachObserver`/`detachObserver`:
  ```cpp
  // Original: 2 interfaces, 4 virtual methods
  class PluginCommandManager : public IObservable { IObserver* m_observer; };
  
  // Modern: 1 line
  class CommandManager { std::function<void(const std::string&)> m_onCommand; };
  ```
- Type-safe — no raw `IObserver*` pointer that could dangle
- Supports lambdas directly — controller doesn't need to inherit from anything

---

## 4. Singleton Pattern (×5 instances)

**Global access points for shared infrastructure.**

| Singleton | Purpose |
|---|---|
| `PluginCommandManager` | Command routing and execution |
| `PluginController` | Facade over backup storage subsystem |
| `CatalystBackupStorage` | Storage session management |
| `PluginCommandContextManager` | Context creation/lookup registry |
| `PluginConfigManager` | Configuration file reader |

**Original implementation:**
```cpp
// All 5 follow this pattern:
static PluginCommandContextManager* s_instance;
static thrLock_t s_mutex;

static PluginCommandContextManager* getInstance(LoggerBase* pLogger) {
    s_mutex.lock();
    if (!s_instance) s_instance = new PluginCommandContextManager(pLogger);
    s_mutex.unlock();
    return s_instance;
}
```

**C++17 Modernization Advantages:**
- **Meyer's Singleton** — C++11 guarantees thread-safe initialization of function-local statics (§6.7). Eliminates manual `thrLock_t` mutex, raw pointer, and leaked memory:
  ```cpp
  // Original: 8 lines, manual lock, raw new, never deleted
  // Modern: 3 lines, thread-safe by standard, deterministic destruction
  static PluginCommandContextManager& getInstance(LoggerBase* pLogger) {
      static PluginCommandContextManager instance(pLogger);
      return instance;
  }
  ```
- No mutex overhead after first call — the compiler generates internal guard variables
- Deterministic destruction at program exit (reverse order of construction)
- Eliminates 5 static mutex objects and 5 raw pointer leaks across the codebase

---

## 5. Abstract Factory Pattern

**Decouples metadata manager creation from operation logic.**

**Original files:**
- `ICatalystMetadataManagerFactory.hpp` — abstract factory interface
- `SAPMetadataManagerFactory.hpp` — concrete factory

```cpp
// Abstract factory
class ICatalystMetadataManagerFactory {
    virtual CatalystMetadataManager* getNewMetadataManager(LoggerBase* pLogger) const = 0;
};

// Concrete factory
SAPMetadataManager* SAPMetadataManagerFactory::getNewMetadataManager(LoggerBase* pLogger) const {
    return new SAPMetadataManager(pLogger, m_isvName, m_isvVersion, m_backintVersion, m_platform);
}
```

Factory is injected into `ISAPOperation` constructor — operations don't know which metadata manager they get.

**C++17 Modernization Advantages:**
- Return `std::unique_ptr<CatalystMetadataManager>` — makes ownership transfer explicit, prevents caller from forgetting `delete`:
  ```cpp
  std::unique_ptr<SAPMetadataManager> createMetadataManager(LoggerBase* pLogger) const {
      return std::make_unique<SAPMetadataManager>(pLogger, m_isvName, m_isvVersion, ...);
  }
  ```
- Class Template Argument Deduction (CTAD) with `std::make_unique` — cleaner syntax
- Factory itself can be passed as `std::function` instead of requiring interface inheritance

---

## 6. Strategy Pattern

**Interchangeable storage and I/O implementations.**

**Original files:**
- `IBackupStorage.hpp` — storage strategy interface (validate, create, transfer, delete, etc.)
- `CatalystBackupStorage.hpp` — concrete Catalyst storage strategy
- `IPluginIO.hpp` — I/O strategy interface (open, read, write, seek, close)

```cpp
class IBackupStorage {
    virtual void validateConfig(...) const = 0;
    virtual void createBackup(...) const = 0;
    virtual void transferBackupData(...) const = 0;
    virtual void deleteBackup(...) const = 0;
    // ... full lifecycle
};
```

`PluginController` delegates all storage operations to whichever `IBackupStorage` it holds — swappable between Catalyst, cloud, or test implementations.

**C++17 Modernization Advantages:**
- Store strategies as `std::unique_ptr<IBackupStorage>` for clear ownership
- For simple strategies (few methods), replace interface with `std::function` members — eliminates need for separate class hierarchies
- Use `std::variant` for closed strategy sets where all implementations are known at compile time — eliminates vtable overhead

---

## 7. State Machine Pattern

**Thread execution lifecycle in the sliding-window pool.**

**Original file:** `SAPCommonTypes.hpp`

```cpp
typedef enum {
    THREAD_EXECUTION_NOT_STARTED = 0,
    THREAD_EXECUTION_RUNNING     = 1,
    THREAD_EXECUTION_FINISHED    = 2,
    THREAD_EXECUTION_COMPLETED   = 3
} eThreadExecutionStatus;
```

State transitions:
```
NOT_STARTED ──startThreads()──→ RUNNING ──operationStream()──→ FINISHED ──joinThreads()──→ COMPLETED
  (main thread)                 (main thread sets)              (worker thread sets)        (main thread sets)
```

The sliding-window dispatcher (`runParallelOperation`) uses these states to decide when to recycle thread slots.

**C++17 Modernization Advantages:**
- `enum class ThreadStatus` — scoped, type-safe, can't accidentally compare with `int`
- `std::atomic<ThreadStatus>` for lock-free reads — the dispatcher scans all entries' status; with atomic, no mutex needed for the status field alone
- Compile-time state transition validation using `constexpr` functions or `static_assert`

---

## 8. Facade Pattern

**`PluginController` as simplified interface to complex backup subsystem.**

**Original file:** `PluginController.hpp`

The header explicitly documents an MVC architecture:
- **Controller:** `PluginController` (this class)
- **View:** `ICommandManager` interface
- **Model:** `IBackupStorage` + `IBackupAgent`

```cpp
class PluginController : public IObserver {
public:
    void executeCommand(const std::string& contextId, PluginCommand_t commandType) const;
protected:
    // 13 hidden internal operations:
    void doValidateConfig(...)  const;
    void doValidateBackup(...)  const;
    void doCreateBackup(...)    const;
    void doTransferBackup(...)  const;
    void doFinishBackup(...)    const;
    void doValidateRestore(...) const;
    void doTransferRestore(...) const;
    void doFinishRestore(...)   const;
    void doGetBackupInfo(...)   const;
    void doDeleteBackup(...)    const;
    // etc.
};
```

Operations call `m_commandManager->executeCommand(ctx, VALIDATE_CONFIG)` — the entire complex subsystem is hidden.

**C++17 Modernization Advantages:**
- Replace `PluginCommand_t` enum dispatch (likely a switch) with `std::unordered_map<PluginCommand_t, std::function<void(Context&)>>` — open/closed principle for adding new commands
- Use `std::optional<std::string>` for error text instead of `bool isError` + `string getErrorText()` pattern
- Use `[[nodiscard]]` attribute on methods whose results must be checked

---

## 9. RAII Pattern (Custom Lock Wrappers)

**Mutex and process lock lifecycle tied to object lifetime.**

**Original files:** `Lock.h`, `LockUnix.h`

```cpp
// ThrLock wraps pthread_mutex lifecycle:
template<> class ThrLock<_unx, thread> {
    ThrLock() { pthread_mutex_init(&m_mutex, 0); }
    ~ThrLock() { pthread_mutex_destroy(&m_mutex); }
    bool lock()   { return pthread_mutex_lock(&m_mutex) == 0; }
    bool unlock() { return pthread_mutex_unlock(&m_mutex) == 0; }
};

// ProcLock wraps file-descriptor-based process-level lock:
// Constructor opens lock file, destructor closes it
```

Used everywhere: `thrLock_t operationEntriesMutex` in `ISAPOperation`, `static thrLock_t s_mutex` in singletons.

**C++17 Modernization Advantages:**
- **Eliminate entirely** — replace `thrLock_t` with `std::mutex` (portable, standard, battle-tested)
- Replace manual `lock()`/`unlock()` calls with `std::lock_guard` (no forgotten unlocks, exception-safe)
- Replace `ProcLock` with `std::scoped_lock` for multi-mutex acquisition (deadlock-free ordering)
- Original `bool lock()` return value is often unchecked — `std::mutex::lock()` throws `std::system_error` on failure, making errors impossible to ignore

---

## 10. Policy-Based Design (Compile-Time Strategy)

**Platform and threading behavior selected at compile time via template specialization.**

**Original file:** `Lock.h`

```cpp
enum platform_t { _unx, win };
enum thrd_t { thread, no_thread };

template <platform_t P, thrd_t T> class ProcLock { ... };
template <platform_t P, thrd_t T> class ThrLock  { ... };

#ifdef _WIN32
    typedef ThrLock<win, thread> thrLock_t;     // Windows: CRITICAL_SECTION
#else
    typedef ThrLock<_unx, thread> thrLock_t;    // Linux: pthread_mutex_t
#endif
```

No-thread specializations are no-ops — safe for single-threaded builds.

**C++17 Modernization Advantages:**
- **Eliminated** — `std::mutex` is already portable across Linux/Windows/macOS
- Replace `#ifdef _WIN32` blocks with `if constexpr` for remaining platform differences:
  ```cpp
  if constexpr (Platform == PlatformType::Windows) { ... }
  else { ... }
  ```
- Use `constexpr` functions for compile-time configuration instead of `#define`

---

## 11. Layered Inheritance (Metadata Managers)

**Three-level class hierarchy, each layer adding plugin-specific metadata fields.**

```
CatalystMetadataManager              (formatVersion, clientVersion, isvName)
    └── SAPMetadataManager           (databaseName, EBID, backupType, backupTime)
        └── SAPHanaMetadataManager   (HANA-specific metadata rendering)
```

Each level overrides `setPluginMetadata()`, `renderPluginMetadata()`, `parsePluginMetadata()` — a Template Method inside the metadata hierarchy.

**C++17 Modernization Advantages:**
- Use `std::variant<CatalystMetadata, SAPMetadata, SAPHanaMetadata>` if the hierarchy is closed — eliminates vtable and heap allocation
- Use `std::any` for extensible metadata fields instead of fixed member variables per level
- Structured bindings for metadata extraction: `auto [name, version, ebid] = metadata.getFields();`

---

## 12. Context/Registry Pattern

**`PluginCommandContextManager` manages context objects in a map.**

**Original file:** `PluginCommandContextManager.hpp`

```cpp
class PluginCommandContextManager {
    std::map<std::string, PluginCommandContext *const> m_commandContexts;
    static thrLock_t s_mutex;
public:
    void createContext(void *const ctx);
    void destroyContext(void *const ctx);
    PluginCommandContext* getContextFromMap(const std::string& contextId) const;
};
```

Each backup stream creates a context, uses it throughout, then destroys it.

**C++17 Modernization Advantages:**
- Replace `std::map<string, PluginCommandContext*>` with `std::unordered_map<std::string, std::unique_ptr<PluginCommandContext>>` — eliminates manual delete, O(1) lookup instead of O(log n)
- Replace `void*` parameter with proper type — `void *const ctx` is a type-safety hole
- Use `std::shared_mutex` for concurrent read access (multiple streams reading, single writer)
- `try_emplace()` (C++17) for atomic insert-if-not-exists

---

## 13. Simple Factory

**Operation creation based on user command-line input.**

**Original file:** `main.cpp`

```cpp
switch(userRequest.getOperation()) {
    case SAP_FUNCTION_BACKUP:  pOperation = new SAPHanaBackup(userRequest);  break;
    case SAP_FUNCTION_RESTORE: pOperation = new SAPHanaRestore(userRequest); break;
    case SAP_FUNCTION_INQUIRE: pOperation = new SAPHanaInquire(userRequest); break;
    case SAP_FUNCTION_DELETE:  pOperation = new SAPHanaDelete(userRequest);  break;
}
// Used polymorphically: pOperation->init(); pOperation->runOperation(); pOperation->finish();
```

**C++17 Modernization Advantages:**
- Return `std::unique_ptr<ISAPOperation>` instead of raw pointer — caller can't forget `delete`
- Replace switch with `std::unordered_map<SAP_eFunctionType, std::function<std::unique_ptr<ISAPOperation>(UserRequest&)>>` — open for extension without modifying the factory
- Use `std::make_unique<SAPHanaBackup>(userRequest)` for exception-safe construction

---

## Summary: C++17 Modernization Impact

| Area | Before (C++03/11 + Platform API) | After (C++17) | Benefit |
|---|---|---|---|
| **Thread safety** | Manual `thrLock_t` mutex + `lock()`/`unlock()` | `std::mutex` + `std::lock_guard` | Exception-safe, no forgotten unlocks |
| **Singleton** | Raw pointer + manual mutex | Meyer's Singleton (static local) | Thread-safe by standard, no leak |
| **Ownership** | Raw `new`/`delete`, `void*` | `std::unique_ptr`, `std::make_unique` | No leaks, clear ownership |
| **Observer** | 2 interface files + 4 virtual methods | `std::function` callback | Less code, supports lambdas |
| **Platform code** | `#ifdef _WIN32` + template specialization | `std::mutex`/`std::thread` (portable) | Eliminate platform wrappers |
| **Error handling** | `bool` returns + `_GOTO_(out)` | `std::optional`, `[[nodiscard]]` | Compiler-enforced error checking |
| **Enum safety** | C-style `typedef enum { ... }` | `enum class` | Scoped, no implicit int conversion |
| **String handling** | `std::string` copies in parsing | `std::string_view` | Zero-copy input parsing |
| **Container lookup** | `std::map` (O(log n)) | `std::unordered_map` + `try_emplace` | O(1) lookup, atomic insert |
