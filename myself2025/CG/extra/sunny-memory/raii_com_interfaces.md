# RAII Wrappers for COM Interfaces

## Brief Explanation

**Problem**: Manual COM lifecycle management in C++ requires explicit `Release()` calls, leading to memory leaks when exceptions occur or cleanup is forgotten. COM objects like `ITaskService` and `ITaskDefinition` were leaking ~180 MB.

**Solution**: Implemented a custom `COMPtr<T>` RAII wrapper template that automatically calls `Release()` in its destructor. This ensures exception-safe cleanup and prevents leaks.

## Problem Demonstration

### The Problem: Manual Cleanup is Error-Prone

COM objects use reference counting. You must call `Release()` for each `AddRef()` (implicitly called by creation functions). Forgetting to call `Release()` causes memory leaks.

### Simplified Problematic Code:

```cpp
void ScheduleBackupTask() {
    ITaskService* pService = NULL;
    
    // Create COM object (implicit AddRef)
    CoCreateInstance(CLSID_TaskScheduler, NULL, 
        CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    
    // Use the object...
    pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    
    // PROBLEM: If exception thrown here, Release() never called!
    if (someErrorCondition()) {
        throw std::runtime_error("Error!");  // MEMORY LEAK!
    }
    
    // Manual cleanup (easy to forget)
    pService->Release();  // Must call this!
}
```

**Issues**:
- **Exception Unsafe**: Early returns/exceptions skip cleanup
- **Easy to Forget**: One missing `Release()` = leak
- **Complex Functions**: Multiple objects require multiple `Release()` calls
- **Maintenance Burden**: Adding code later can introduce leaks

### Key Features:
- **Automatic Cleanup**: Destructor handles `Release()` calls
- **Exception Safe**: No manual cleanup needed in error paths
- **Move Semantics**: Efficient ownership transfer
- **No Copy**: Prevents double-release issues

### Code Example:
```cpp
template<typename T>
class COMPtr {
    T* m_ptr;
public:
    COMPtr() : m_ptr(nullptr) {}
    ~COMPtr() { if (m_ptr) m_ptr->Release(); }
    // Move semantics, operators...
};

// Usage:
COMPtr<ITaskService> pService;
CoCreateInstance(..., (void**)&pService);
// Automatic cleanup when scope exits
```

**Memory Impact**: Eliminated ~180 MB of COM interface leaks, contributing significantly to the overall 40% memory reduction.