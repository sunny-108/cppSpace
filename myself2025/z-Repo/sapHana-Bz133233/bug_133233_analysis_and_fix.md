# Bug 133233 - Object Expiration Not Always Successful

## Problem Summary

### Symptom
During SAP HANA backup expiration operations, some backup objects failed to delete while others succeeded. The failed deletes resulted in `#ERROR` status instead of `#DELETED`.

### Root Cause Analysis

Based on the logs in `problem.txt`, the issue manifested as:

1. **Failed Deletes** (2 backups):
   - `1588681255074_Stm8LHvc` - ERROR
   - `1588682155076_hV63KIvc` - ERROR
   
2. **Error Pattern**:
   ```
   OSCLT_ERR_SERVER_OFFLINE [-1404]
   OSCLT_ERR_PERMISSION_DENIED [-1410]
   ```

3. **Key Observations from Logs**:
   - Failed deletes: `Failed to read credentials file!` followed by `OSCLT_ERR_SERVER_OFFLINE [-1404]`
   - Successful deletes: `User qm1adm-sapvqm1db01-hana using information stored in credentials file`
   - Protocol status 101 mapped to local status -1410 (PERMISSION_DENIED)
   - Failed during `osCltCmd_ListObjectStores` operation

### Technical Root Cause

The problem appears to be a **race condition** in concurrent delete operations. When multiple delete threads run simultaneously:
- Some threads successfully read credentials and connect to the object store
- Other threads fail to read credentials, likely due to concurrent access issues
- The failed threads then cannot establish connections (SERVER_OFFLINE) and fail with PERMISSION_DENIED errors

## Solution Implemented (PR #35)

### Changes Overview
- **Files Modified**: 2
- **Additions**: 58 lines
- **Deletions**: 7 lines

### 1. New Utility Function (`CommonUtils.hpp`)

Added `isDirExist()` function to check directory existence:

```cpp
inline bool isDirExist(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}
```

### 2. Process-Level Mutex for Delete Operations (`main.cpp`)

**Key Changes**:

#### Before:
- Delete operations were treated like other operations
- No synchronization between concurrent delete threads
- All operations followed the same code path:
  ```cpp
  case SAP_FUNCTION_DELETE:
      pOperation = new SAPHanaDelete(userRequest);
  break;
  ```

#### After:
- **Separated delete operation handling** with process-level locking
- **Process mutex** (`procLock_t`) ensures only one delete operation runs at a time
- **Lock file location**: `./hana_plugin/hana_del_lock`

```cpp
if(operation == SAP_FUNCTION_DELETE)
{
    try{
        std::string dir_path = "./hana_plugin/";
        if (!isDirExist(dir_path))
            mkdir(dir_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);  

        procLock_t proc_mutex(dir_path,"hana_del_lock");

        if(proc_mutex.lock() == false){
            PLUGIN_TRACE_IF_ERROR_LOG( pLogger, "Failed to lock proc mutex")
        } 

        pOperation = new SAPHanaDelete(userRequest);
        if(pOperation){
            pOperation->init();
            pOperation->runOperation();
            pOperation->finish();                    
        }
        
        if(proc_mutex.unlock() == false){
            PLUGIN_TRACE_IF_ERROR_LOG( pLogger, "Failed to unlock proc mutex") 
        }
    }
    catch(std::exception e){
        PLUGIN_TRACE_IF_ERROR_LOG( pLogger, e.what());
    }
}
```

## How the Fix Addresses the Problem

### 1. **Serializes Delete Operations**
- Process-level lock (`procLock_t`) prevents multiple delete operations from running concurrently
- Each delete operation must acquire the lock before proceeding
- This eliminates race conditions in credential file access and object store connections

### 2. **Maintains Safe State**
- Creates dedicated directory (`./hana_plugin/`) for lock files if it doesn't exist
- Proper error handling with try-catch blocks
- Logging for lock acquisition/release failures

### 3. **Non-Intrusive for Other Operations**
- Only delete operations are affected by the mutex
- Backup, restore, inquire operations continue to run normally
- Delete operation handling is isolated in a separate code block

## Expected Outcome

After this fix:
- ✅ All delete operations will be serialized
- ✅ No concurrent access to credentials files during delete
- ✅ No OSCLT_ERR_SERVER_OFFLINE errors during delete
- ✅ No OSCLT_ERR_PERMISSION_DENIED errors during delete
- ✅ All backups in expiration list should delete successfully
- ✅ Other operations (backup/restore/inquire) remain unaffected

## Technical Details

**Lock Mechanism**: Process-level lock (likely using file-based locking)
- **Lock Directory**: `./hana_plugin/`
- **Lock File**: `hana_del_lock`
- **Lock Type**: `procLock_t` class (appears to be a file-based process mutex)

**Directory Permissions**:
- User: Read, Write, Execute
- Group: Read, Write, Execute  
- Others: Read, Execute

## PR Status
- **State**: OPEN
- **Author**: sshivam@hpe.com
- **Repository**: github.hpe.com/hpe/so-d2d-catalyst_plugins
